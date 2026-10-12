#ifndef GFX_H
#define GFX_H

#include <stdbool.h>
#include <stdint.h>

#include "../fonts/icons.h"

/*
 * Фреймбуфер в раскладке SSD1309/SSD1306: 8 страниц по 128 байт,
 * байт = вертикальный столбик из 8 пикселей, бит 0 — верхний.
 * Буфер целиком уходит в дисплей без преобразований.
 */
#define GFX_WIDTH   128
#define GFX_HEIGHT  64
#define GFX_PAGES   (GFX_HEIGHT / 8)
#define GFX_FB_SIZE (GFX_WIDTH * GFX_PAGES)

typedef enum {
    GFX_OFF = 0,
    GFX_ON = 1,
    GFX_INVERT = 2,
} gfx_color_t;

typedef struct {
    uint8_t fb[GFX_FB_SIZE];
} gfx_t;

void gfx_clear(gfx_t *g);
void gfx_pixel(gfx_t *g, int16_t x, int16_t y, gfx_color_t c);
bool gfx_get_pixel(const gfx_t *g, int16_t x, int16_t y);

void gfx_hline(gfx_t *g, int16_t x, int16_t y, int16_t w, gfx_color_t c);
void gfx_vline(gfx_t *g, int16_t x, int16_t y, int16_t h, gfx_color_t c);
void gfx_line(gfx_t *g, int16_t x0, int16_t y0, int16_t x1, int16_t y1, gfx_color_t c);
void gfx_rect(gfx_t *g, int16_t x, int16_t y, int16_t w, int16_t h, gfx_color_t c);
void gfx_fill_rect(gfx_t *g, int16_t x, int16_t y, int16_t w, int16_t h, gfx_color_t c);

/* Битмап из байт-столбцов высотой до 8 px; рисуются только горящие пиксели. */
void gfx_bitmap8(gfx_t *g, int16_t x, int16_t y, const uint8_t *cols, uint8_t w, uint8_t scale,
                 gfx_color_t c);
void gfx_icon(gfx_t *g, int16_t x, int16_t y, const icon_t *icon, gfx_color_t c);

/* Текст UTF-8 шрифтом 5x7. Возвращает x после последнего символа. */
int16_t gfx_text(gfx_t *g, int16_t x, int16_t y, const char *utf8, uint8_t scale, gfx_color_t c);
int16_t gfx_text_width(const char *utf8, uint8_t scale);

/* Целое со знаком в десятичную строку; buf >= 12 байт. Возвращает buf. */
char *gfx_itoa(int32_t v, char *buf);

#endif /* GFX_H */
