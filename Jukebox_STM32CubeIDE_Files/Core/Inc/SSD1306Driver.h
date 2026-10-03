#ifndef SSD1306DRIVER_H
#define SSD1306DRIVER_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stddef.h>

// Handle for the I2C peripheral the OLED is on.
// Defined in main.c by MX_I2C1_Init(); declared here so the driver
// file can reach it without passing it into every call.
extern I2C_HandleTypeDef hi2c1;
#define SSD1306_I2C_HANDLE   (&hi2c1)

#define SSD1306_WIDTH          128
#define SSD1306_HEIGHT         64
#define SSD1306_BUFFER_SIZE    (SSD1306_WIDTH * SSD1306_HEIGHT / 8)

// Shared with the TIM2 update-event callback in main.c
extern volatile uint8_t staggeredCharIndex;

void ssd1306_cmd(uint8_t cmd);
void ssd1306_init(void);
void ssd1306_set_full_window(void);
void ssd1306_update(void);
void ssd1306_clear_buffer(void);
void ssd1306_clear(void);
void ssd1306_draw_pixel(int x, int y);
void ssd1306_draw_line(int x0, int y0, int x1, int y1);
void ssd1306_draw_rect(int x, int y, int w, int h);
void ssd1306_filled_rect(int x, int y, int w, int h);
void ssd1306_draw_circle(int cx, int cy, int r);
void ssd1306_draw_char(int x, int y, char c);
void ssd1306_print(int x, int y, const char *str);
void ssd1306_print_staggered(int x, int y, const char *str);

#endif
