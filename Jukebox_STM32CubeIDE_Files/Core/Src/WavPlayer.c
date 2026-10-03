#include "WavPlayer.h"
#include "main.h"
#include <string.h>
#include <stdio.h>

extern I2S_HandleTypeDef hi2s1;

// Frames (L+R pairs) per DMA half-buffer. At 44.1kHz this is
// 4096 / 44100 =~ 93ms of playback per half. Made large enough so
// that RFID polling (worst case ~50ms internal blocking), SD card read
// time, OLED updates, and rotary encoder reads all fit inside one
// half-buffer's playback duration without starving the DMA.
//
// Halving it to 2048 (~46ms at 44.1kHz) would already be tighter than
// the RFID library's worst-case block on its own.
#define I2S_HALF_BUFFER_FRAMES   4096
#define I2S_DMA_BUFFER_SIZE      (2 * 2 * I2S_HALF_BUFFER_FRAMES) // 2 halves x L+R

static int16_t i2s_dma_buffer[I2S_DMA_BUFFER_SIZE];
static volatile int16_t *dma_half_to_fill = NULL;

static uint8_t g_volumePercent = 50;

void WavPlayer_SetVolume(uint8_t percent)
{
    if (percent > 100) percent = 100;
    g_volumePercent = percent;
}

// ---- I2S DMA callbacks --------------------------------------------------
// These only ever set a pointer, no major work in these functions.
// WavPlayer_Poll() does the actual (potentially slow)
// refill work from the main loop, not from these interrupts.

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI1)
        dma_half_to_fill = &i2s_dma_buffer[0];
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI1)
        dma_half_to_fill = &i2s_dma_buffer[2 * I2S_HALF_BUFFER_FRAMES];
}

// ---- WAV header parsing --------------------------------------------------

#pragma pack(push, 1)
typedef struct {
    char     chunkId[4];
    uint32_t chunkSize;
} ChunkHeader_t;
#pragma pack(pop)

static uint8_t parse_wav_header(WavPlayer_t *wp)
{
    uint8_t riffHdr[12];
    UINT br;

    if (f_read(&wp->file, riffHdr, 12, &br) != FR_OK || br != 12) return 0;
    if (memcmp(riffHdr, "RIFF", 4) != 0 || memcmp(&riffHdr[8], "WAVE", 4) != 0) return 0;

    uint8_t haveFmt = 0, haveData = 0;

    while (!haveData)
    {
        ChunkHeader_t ch;
        if (f_read(&wp->file, &ch, 8, &br) != FR_OK || br != 8) return 0;

        if (memcmp(ch.chunkId, "fmt ", 4) == 0)
        {
            if (ch.chunkSize < 16) return 0;
            uint8_t fmt[16];
            if (f_read(&wp->file, fmt, 16, &br) != FR_OK || br != 16) return 0;

            uint16_t audioFormat = (uint16_t)(fmt[0] | (fmt[1] << 8));
            wp->numChannels   = (uint16_t)(fmt[2] | (fmt[3] << 8));
            wp->sampleRate    = (uint32_t)(fmt[4] | (fmt[5] << 8) | (fmt[6] << 16) | ((uint32_t)fmt[7] << 24));
            wp->bitsPerSample = (uint16_t)(fmt[14] | (fmt[15] << 8));

            // Only plain 16-bit PCM is supported
            if (audioFormat != 1 || wp->bitsPerSample != 16) return 0;

            if (ch.chunkSize > 16)
                f_lseek(&wp->file, f_tell(&wp->file) + (ch.chunkSize - 16));

            haveFmt = 1;
        }
        else if (memcmp(ch.chunkId, "data", 4) == 0)
        {
            if (!haveFmt) return 0;
            wp->dataChunkSize = ch.chunkSize;
            haveData = 1;
        }
        else
        {
            // Unknown chunk (LIST, fact, cue, etc.) -- skip it.
            // Chunks are word-aligned: odd sizes have a pad byte.
            f_lseek(&wp->file, f_tell(&wp->file) + ch.chunkSize + (ch.chunkSize & 1));
        }
    }

    wp->totalSamples = wp->dataChunkSize / 2 / wp->numChannels;
    return 1;
}

// ---- Refill ---------------------------------------------------------------

// Reads up to framesRequested stereo frames into dst (interleaved L/R,
// int16). Mono files are duplicated to both channels. Applies software
// volume. Any shortfall at end-of-file is zero-filled (silence).
static void fill_stereo_block(WavPlayer_t *wp, int16_t *dst, uint32_t framesRequested)
{
    uint32_t bytesRemaining = wp->dataChunkSize - wp->bytesRead;
    uint32_t bytesPerFrame = wp->numChannels * 2;
    uint32_t framesAvailable = bytesRemaining / bytesPerFrame;
    uint32_t framesToRead = (framesRequested < framesAvailable) ? framesRequested : framesAvailable;
    uint32_t framesGot = 0;

    if (wp->numChannels == 2)
    {
        UINT br;
        f_read(&wp->file, dst, framesToRead * 4, &br);
        wp->bytesRead += br;
        framesGot = br / 4;
    }
    else // mono -> duplicate into both channels
    {
        static int16_t monoScratch[512];
        while (framesGot < framesToRead)
        {
            uint32_t chunk = framesToRead - framesGot;
            if (chunk > 512) chunk = 512;

            UINT br;
            f_read(&wp->file, monoScratch, chunk * 2, &br);
            uint32_t gotFrames = br / 2;

            for (uint32_t i = 0; i < gotFrames; i++)
            {
                dst[2 * (framesGot + i)]     = monoScratch[i];
                dst[2 * (framesGot + i) + 1] = monoScratch[i];
            }
            wp->bytesRead += br;
            framesGot += gotFrames;
            if (gotFrames < chunk) break; // short read -> EOF
        }
    }

    // Software volume, applied to whatever was actually read
    if (g_volumePercent != 100)
    {
        for (uint32_t i = 0; i < framesGot * 2; i++)
        {
            dst[i] = (int16_t)(((int32_t)dst[i] * g_volumePercent) / 100);
        }
    }

    // Zero-fill any remainder (true EOF, or last partial block) -> silence
    for (uint32_t i = framesGot; i < framesRequested; i++)
    {
        dst[2 * i] = 0;
        dst[2 * i + 1] = 0;
    }
}

// ---- Public API -------------------------------------------------------

void WavPlayer_Init(WavPlayer_t *wp)
{
    memset(wp, 0, sizeof(*wp));
    wp->state = WAV_IDLE;
}

void WavPlayer_Stop(WavPlayer_t *wp)
{
    if (wp->state != WAV_IDLE)
    {
        HAL_I2S_DMAStop(&hi2s1);
        f_close(&wp->file);
    }
    wp->state = WAV_IDLE;
    dma_half_to_fill = NULL;
}

uint8_t WavPlayer_Play(WavPlayer_t *wp, const char *filePath)
{
    WavPlayer_Stop(wp);

    if (f_open(&wp->file, filePath, FA_READ) != FR_OK)
    {
    	printf("Read fail");
        wp->state = WAV_ERROR;
        return 1;
    }

    if (!parse_wav_header(wp))
    {
    	printf("Parse fail");
        f_close(&wp->file);
        wp->state = WAV_ERROR;
        return 1;
    }

    wp->bytesRead = 0;

    // Derive a display title from the filename: strip path + extension,
    // uppercase it (the OLED font only has A-Z / 0-9 glyphs).
    const char *base = strrchr(filePath, '/');
    base = base ? base + 1 : filePath;
    strncpy(wp->title, base, sizeof(wp->title) - 1);
    wp->title[sizeof(wp->title) - 1] = '\0';
    char *dot = strrchr(wp->title, '.');
    if (dot) *dot = '\0';
    for (char *c = wp->title; *c; c++)
        if (*c >= 'a' && *c <= 'z') *c -= 32;

    // Match I2S clock to the file's actual sample rate
    HAL_I2S_DeInit(&hi2s1);
    hi2s1.Init.AudioFreq = wp->sampleRate;
    if (HAL_I2S_Init(&hi2s1) != HAL_OK)
    {
    	printf("Clock I2S fail");
        f_close(&wp->file);
        wp->state = WAV_ERROR;
        return 1;
    }

    // Pre-fill both halves so playback doesn't start with silence
    fill_stereo_block(wp, &i2s_dma_buffer[0], I2S_HALF_BUFFER_FRAMES);
    fill_stereo_block(wp, &i2s_dma_buffer[2 * I2S_HALF_BUFFER_FRAMES], I2S_HALF_BUFFER_FRAMES);
    dma_half_to_fill = NULL;

    HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t *)i2s_dma_buffer, I2S_DMA_BUFFER_SIZE);

    wp->state = WAV_PLAYING;
    return 0;
}

void WavPlayer_Pause(WavPlayer_t *wp)
{
    if (wp->state == WAV_PLAYING)
    {
        HAL_I2S_DMAPause(&hi2s1);
        wp->state = WAV_PAUSED;
    }
}

void WavPlayer_Resume(WavPlayer_t *wp)
{
    if (wp->state == WAV_PAUSED)
    {
        HAL_I2S_DMAResume(&hi2s1);
        wp->state = WAV_PLAYING;
    }
}

void WavPlayer_Poll(WavPlayer_t *wp)
{
    if (wp->state != WAV_PLAYING) return;
    if (dma_half_to_fill == NULL) return;

    int16_t *target = (int16_t *)dma_half_to_fill;
    dma_half_to_fill = NULL; // clear before the (potentially slow) f_read

    fill_stereo_block(wp, target, I2S_HALF_BUFFER_FRAMES);
}

uint8_t WavPlayer_IsFinished(WavPlayer_t *wp)
{
    return (wp->state == WAV_PLAYING) && (wp->bytesRead >= wp->dataChunkSize);
}

uint32_t WavPlayer_GetElapsedSeconds(WavPlayer_t *wp)
{
    if (wp->sampleRate == 0) return 0;
    uint32_t samplesElapsed = wp->bytesRead / (wp->numChannels * 2);
    return samplesElapsed / wp->sampleRate;
}

uint32_t WavPlayer_GetTotalSeconds(WavPlayer_t *wp)
{
    if (wp->sampleRate == 0) return 0;
    return wp->totalSamples / wp->sampleRate;
}

uint8_t WavPlayer_GetProgressFraction(WavPlayer_t *wp)
{
    if (wp->totalSamples == 0) return 0;
    uint32_t samplesElapsed = wp->bytesRead / (wp->numChannels * 2);
    return (uint8_t)(((uint64_t)samplesElapsed * 255) / wp->totalSamples);
}
