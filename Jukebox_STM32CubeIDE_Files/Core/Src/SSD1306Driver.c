#include "SSD1306Driver.h"
#include <string.h>

#define SSD1306_ADDR     0x3C   // 7-bit address
#define MIRROR_VERT      0
#define MIRROR_HORIZ     1
#define INVERSE_COLOR    1

#define I2C_TIMEOUT_MS   100

uint8_t ssd1306_buffer[SSD1306_BUFFER_SIZE];
volatile uint8_t staggeredCharIndex = 0;

static const uint8_t font5x7_letters[29][5] = {

    // A
    {0x7C, 0x12, 0x11, 0x12, 0x7C},
    // B
    {0x7F, 0x49, 0x49, 0x49, 0x36},
    // C
    {0x3E, 0x41, 0x41, 0x41, 0x22},
    // D
    {0x7F, 0x41, 0x41, 0x22, 0x1C},
    // E
    {0x7F, 0x49, 0x49, 0x49, 0x41},
    // F
    {0x7F, 0x09, 0x09, 0x09, 0x01},
    // G
    {0x3E, 0x41, 0x49, 0x49, 0x7A},
    // H
    {0x7F, 0x08, 0x08, 0x08, 0x7F},
    // I
    {0x00, 0x41, 0x7F, 0x41, 0x00},
    // J
    {0x20, 0x40, 0x41, 0x3F, 0x01},
    // K
    {0x7F, 0x08, 0x14, 0x22, 0x41},
    // L
    {0x7F, 0x40, 0x40, 0x40, 0x40},
    // M
    {0x7F, 0x02, 0x04, 0x02, 0x7F},
    // N
    {0x7F, 0x04, 0x08, 0x10, 0x7F},
    // O
    {0x3E, 0x41, 0x41, 0x41, 0x3E},
    // P
    {0x7F, 0x09, 0x09, 0x09, 0x06},
    // Q
    {0x3E, 0x41, 0x51, 0x21, 0x5E},
    // R
    {0x7F, 0x09, 0x19, 0x29, 0x46},
    // S
    {0x46, 0x49, 0x49, 0x49, 0x31},
    // T
    {0x01, 0x01, 0x7F, 0x01, 0x01},
    // U
    {0x3F, 0x40, 0x40, 0x40, 0x3F},
    // V
    {0x1F, 0x20, 0x40, 0x20, 0x1F},
    // W
    {0x7F, 0x20, 0x18, 0x20, 0x7F},
    // X
    {0x63, 0x14, 0x08, 0x14, 0x63},
    // Y
    {0x03, 0x04, 0x78, 0x04, 0x03},
    // Z
    {0x61, 0x51, 0x49, 0x45, 0x43},
    // [
    {0x60, 0x60, 0x3E, 0x04, 0x00},
    // "\"
    {0x0C, 0x1E, 0x3C, 0x1E, 0x0C},
    // ]
    {0x00, 0x30, 0x30, 0x1F, 0x02}
};

static const uint8_t font5x7_numbers[10][5] = {
    // 0
    {0x3E, 0x51, 0x49, 0x45, 0x3E},
    // 1
    {0x00, 0x42, 0x7F, 0x40, 0x00},
    // 2
    {0x62, 0x51, 0x49, 0x49, 0x46},
    // 3
    {0x22, 0x41, 0x49, 0x49, 0x36},
    // 4
    {0x18, 0x14, 0x12, 0x7F, 0x10},
    // 5
    {0x2F, 0x49, 0x49, 0x49, 0x31},
    // 6
    {0x3E, 0x49, 0x49, 0x49, 0x32},
    // 7
    {0x01, 0x71, 0x09, 0x05, 0x03},
    // 8
    {0x36, 0x49, 0x49, 0x49, 0x36},
    // 9
    {0x26, 0x49, 0x49, 0x49, 0x3E}
};

static const uint8_t font5x7_special[2][5] = {
    // .
    {0x00, 0x00, 0x60, 0x60, 0x00},
    // :
    {0x00, 0x00, 0x36, 0x36, 0x00}
};

// Sends a single command byte. HAL_I2C_Master_Transmit
// builds the START/ADDR/STOP sequence itself.
void ssd1306_cmd(uint8_t cmd)
{
    uint8_t packet[2] = {0x00, cmd};  // control byte 0x00 = command stream
    HAL_I2C_Master_Transmit(SSD1306_I2C_HANDLE, SSD1306_ADDR << 1,
                             packet, sizeof(packet), I2C_TIMEOUT_MS);
}


// Sends a data block. HAL has no fixed internal TX buffer like Arduino's
// Wire library, so the whole 1024-byte framebuffer can go out as one
// transaction instead of being chunked. (Originally wrote this lib for Arduino)
static void ssd1306_write_data_block(const uint8_t *data, size_t len)
{
    static uint8_t txbuf[SSD1306_BUFFER_SIZE + 1];
    txbuf[0] = 0x40; // control byte: Co=0, D/C=1 (data stream)
    memcpy(&txbuf[1], data, len);
    HAL_I2C_Master_Transmit(SSD1306_I2C_HANDLE, SSD1306_ADDR << 1,
                             txbuf, (uint16_t)(len + 1), I2C_TIMEOUT_MS);
}


void ssd1306_init(void)
{
    /* Turn display off during configuration */
    ssd1306_cmd(0xAE);

    /* Set memory addressing mode */
    ssd1306_cmd(0x20);
    /* Use horizontal addressing mode */
    ssd1306_cmd(0x00);

    /* Set Page Start Address for Page Addressing Mode,0-7 */
    ssd1306_cmd(0xB0);

    /* Mirror vertically - Remap display column scan order  */
    if (MIRROR_VERT == 1) ssd1306_cmd(0xC0);
    else                  ssd1306_cmd(0xC8);

    /* Mirror horizontally - Remap display segs scan order */
    if (MIRROR_HORIZ == 1) ssd1306_cmd(0xA1);
    else                   ssd1306_cmd(0xA0);

    /* Set start line address */
    ssd1306_cmd(0x40);

    /* Set display contrast */
    ssd1306_cmd(0x81);
    ssd1306_cmd(0x7F);

    /* Set normal (non-inverted) display mode */
    if (INVERSE_COLOR != 0) ssd1306_cmd(0xA6);
    else                    ssd1306_cmd(0xA7);

    /* Set multiplex ratio */
    ssd1306_cmd(0xA8);
    ssd1306_cmd(0x3F);

    /* Set display offset */
    ssd1306_cmd(0xD3);
    ssd1306_cmd(0x00);

    /* Set display clock divide ratio */
    ssd1306_cmd(0xD5);
    ssd1306_cmd(0x80);

    /* Enable charge pump regulator */
    ssd1306_cmd(0x8D);
    ssd1306_cmd(0x14);

    /* Turn display on */
    ssd1306_cmd(0xAF);
}


void ssd1306_set_full_window(void)
{
    // Set full column address range
    ssd1306_cmd(0x21);
    ssd1306_cmd(0);
    ssd1306_cmd(127);

    // Set full page address range
    ssd1306_cmd(0x22);
    ssd1306_cmd(0);
    ssd1306_cmd(7);
}


// Sends the contents of the display buffer to the SSD1306.
void ssd1306_update(void)
{
    ssd1306_set_full_window();
    ssd1306_write_data_block(ssd1306_buffer, SSD1306_BUFFER_SIZE);
}


// Clears the entire display buffer by setting all pixels to OFF.
void ssd1306_clear_buffer(void)
{
    memset(ssd1306_buffer, 0x00, SSD1306_BUFFER_SIZE);
}


void ssd1306_clear(void)
{
    ssd1306_clear_buffer();
    ssd1306_update();
}


// Draws a single pixel at the specified (x, y) coordinate.
void ssd1306_draw_pixel(int x, int y)
{
    if (x < 0 || x > 127 || y < 0 || y > 63)
        return;

    uint16_t index = (uint16_t)(x + ((y / 8) * 128));
    uint8_t bit = (uint8_t)(1U << (y % 8));

    ssd1306_buffer[index] |= bit;
}


// Draws a line between two points using Bresenham's line algorithm.
void ssd1306_draw_line(int x0, int y0, int x1, int y1)
{
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int dy = -((y1 > y0) ? (y1 - y0) : (y0 - y1));
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (1)
    {
        ssd1306_draw_pixel(x0, y0);

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}


// Draws the outline of a rectangle.
void ssd1306_draw_rect(int x, int y, int w, int h)
{
    for (int i = 0; i < w; i++) {
        ssd1306_draw_pixel(x + i, y);
        ssd1306_draw_pixel(x + i, y + h - 1);
    }
    for (int i = 0; i < h; i++) {
        ssd1306_draw_pixel(x, y + i);
        ssd1306_draw_pixel(x + w - 1, y + i);
    }
}


// Draws a filled rectangle.
void ssd1306_filled_rect(int x, int y, int w, int h)
{
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            ssd1306_draw_pixel(x + i, y + j);
        }
    }
}


// Draws the outline of a circle centered at (cx, cy).
void ssd1306_draw_circle(int cx, int cy, int r)
{
    for (int x = -r; x <= r; x++)
    {
        for (int y = -r; y <= r; y++)
        {
            int d = x * x + y * y;
            if (d >= (r * r - r) && d <= (r * r + r))
            {
                ssd1306_draw_pixel(cx + x, cy + y);
            }
        }
    }
}


// Draws a single 5x7 character.
void ssd1306_draw_char(int x, int y, char c)
{
    const uint8_t *font = 0;

    if (c >= 'A' && c <= ']')
    {
        font = font5x7_letters[c - 'A'];
    }
    else if (c >= '0' && c <= '9')
    {
        font = font5x7_numbers[c - '0'];
    }
    else if (c == '.')
	{
		font = font5x7_special[0];
	}
	else if (c == ':')
	{
		font = font5x7_special[1];
	}
    else
    {
        return;
    }

    for (int i = 0; i < 5; i++)
    {
        uint8_t col = font[i];
        for (int j = 0; j < 7; j++)
        {
            if (col & (1 << j))
            {
                ssd1306_draw_pixel(x + i, y + j);
            }
        }
    }
}


// Prints a null-terminated string to the display.
void ssd1306_print(int x, int y, const char *str)
{
    int xpos = x;

    while (*str)
    {
        if ((*str >= 'A' && *str <= ']') || (*str >= '0' && *str <= '9')
        || (*str == '.') || (*str == ':'))

        {
            ssd1306_draw_char(xpos, y, *str);
        }
        xpos += 6; // 5 pixels + spacing
        str++;
    }
}


// Prints characters of a string one at a time, revealing a new char
// each time staggeredCharIndex is incremented (from the TIM2 callback).
void ssd1306_print_staggered(int x, int y, const char *str)
{
    int xpos = x;
    uint8_t len = (uint8_t)strlen(str);

    // staggeredCharIndex is a single-byte volatile, so on Cortex-M
    // (32-bit bus) this read is atomic.
    uint8_t visibleChars = staggeredCharIndex;
    if (visibleChars > len) visibleChars = len;

    for (uint8_t i = 0; i < visibleChars; i++)
    {
        char c = str[i];
        if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
          || c == '[' || c == '\\' || c == ']' || c == '.' || c == ':')
        {
            ssd1306_draw_char(xpos, y, c);
        }
        xpos += 6;
    }
}
