#ifndef WAVPLAYER_H
#define WAVPLAYER_H

#include "fatfs.h"
#include <stdint.h>

typedef enum {
    WAV_IDLE,
    WAV_PLAYING,
    WAV_PAUSED,
    WAV_ERROR
} WavPlayerState_t;

typedef struct {
    FIL file;
    uint32_t dataChunkSize;     // bytes of PCM data in the "data" chunk
    uint32_t bytesRead;         // bytes consumed so far within the data chunk
    uint32_t sampleRate;
    uint16_t numChannels;
    uint16_t bitsPerSample;
    uint32_t totalSamples;      // per-channel frame count, for duration
    WavPlayerState_t state;
    char title[32];             // from filename, uppercased (lowercase ASCII chars are shifted)
} WavPlayer_t;

// Called once at startup, before anything else.
void WavPlayer_Init(WavPlayer_t *wp);

// Opens filePath, parses the WAV header (16-bit PCM only), reconfigures
// I2S to the file's actual sample rate, pre-fills both DMA half-buffers,
// and starts playback. Stops/closes whatever was previously playing
// first. Returns 0 on success, 1 on error (bad file, unsupported format).
uint8_t WavPlayer_Play(WavPlayer_t *wp, const char *filePath);

// Stops playback and closes the file. Safe to call when already idle.
void WavPlayer_Stop(WavPlayer_t *wp);

void WavPlayer_Pause(WavPlayer_t *wp);
void WavPlayer_Resume(WavPlayer_t *wp);

// Called every main-loop pass, unconditionally, before anything else that
// could block. Does nothing unless a DMA half-buffer is actually due
// for a refill, in which case it performs one f_read() sized to match
// one half-buffer. Bounded by the buffer-size/sample-rate math, see
// the top of WavPlayer.c.
void WavPlayer_Poll(WavPlayer_t *wp);

// True once playback has consumed the entire data chunk. The tail end
// is zero-filled (silence) rather than glitching, so there's no rush
// to react to this immediately.
uint8_t WavPlayer_IsFinished(WavPlayer_t *wp);

// 0-100. Applied as digital gain to samples during refill, so it works
// regardless of whether DACs have internal volume control.
void WavPlayer_SetVolume(uint8_t percent);

uint32_t WavPlayer_GetElapsedSeconds(WavPlayer_t *wp);
uint32_t WavPlayer_GetTotalSeconds(WavPlayer_t *wp);
uint8_t  WavPlayer_GetProgressFraction(WavPlayer_t *wp); // 0-255

#endif
