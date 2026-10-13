#include "gfx.h"

#include "../fonts/font5x7.h"
#include "utf8.h"

void gfx_clear(gfx_t *g)
{
    for (uint16_t i = 0; i < GFX_FB_SIZE; i++) {
        g->fb[i] = 0u;
    }
}

void gfx_pixel(gfx_t *g, int16_t x, int16_t y, gfx_color_t c)
{
    if (x < 0 || y < 0 || x >= GFX_WIDTH || y >= GFX_HEIGHT) {
        return;
    }
    // В SSD1309 байт хранит 8 вертикальных пикселей: выбираем страницу и бит строки.
    uint16_t idx = (uint16_t)((y / 8) * GFX_WIDTH + x);
    uint8_t mask = (uint8_t)(1u << (y & 7));
    switch (c) {
    case GFX_ON:
        g->fb[idx] |= mask;
        break;
    case GFX_OFF:
        g->fb[idx] &= (uint8_t)~mask;
        break;
    case GFX_INVERT:
        g->fb[idx] ^= mask;
        break;
    }
}

bool gfx_get_pixel(const gfx_t *g, int16_t x, int16_t y)
{
    if (x < 0 || y < 0 || x >= GFX_WIDTH || y >= GFX_HEIGHT) {
        return false;
    }
    return (g->fb[(y / 8) * GFX_WIDTH + x] >> (y & 7)) & 1u;
}

void gfx_hline(gfx_t *g, int16_t x, int16_t y, int16_t w, gfx_color_t c)
{
    for (int16_t i = 0; i < w; i++) {
        gfx_pixel(g, (int16_t)(x + i), y, c);
    }
}

void gfx_vline(gfx_t *g, int16_t x, int16_t y, int16_t h, gfx_color_t c)
{
    for (int16_t i = 0; i < h; i++) {
        gfx_pixel(g, x, (int16_t)(y + i), c);
    }
}

static int16_t iabs16(int16_t v)
{
    return v < 0 ? (int16_t)-v : v;
}

void gfx_line(gfx_t *g, int16_t x0, int16_t y0, int16_t x1, int16_t y1, gfx_color_t c)
{
    /* Брезенхэм без float. */
    int16_t dx = iabs16((int16_t)(x1 - x0));
    int16_t dy = (int16_t)-iabs16((int16_t)(y1 - y0));
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = (int16_t)(dx + dy);

    for (;;) {
        gfx_pixel(g, x0, y0, c);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        int16_t e2 = (int16_t)(2 * err);
        if (e2 >= dy) {
            err = (int16_t)(err + dy);
            x0 = (int16_t)(x0 + sx);
        }
        if (e2 <= dx) {
            err = (int16_t)(err + dx);
            y0 = (int16_t)(y0 + sy);
        }
    }
}

void gfx_rect(gfx_t *g, int16_t x, int16_t y, int16_t w, int16_t h, gfx_color_t c)
{
    if (w <= 0 || h <= 0) {
        return;
    }
    gfx_hline(g, x, y, w, c);
    if (h > 1) {
        gfx_hline(g, x, (int16_t)(y + h - 1), w, c);
    }
    if (h > 2) {
        gfx_vline(g, x, (int16_t)(y + 1), (int16_t)(h - 2), c);
        if (w > 1) {
            gfx_vline(g, (int16_t)(x + w - 1), (int16_t)(y + 1), (int16_t)(h - 2), c);
        }
    }
}

void gfx_fill_rect(gfx_t *g, int16_t x, int16_t y, int16_t w, int16_t h, gfx_color_t c)
{
    for (int16_t j = 0; j < h; j++) {
        gfx_hline(g, x, (int16_t)(y + j), w, c);
    }
}

void gfx_bitmap8(gfx_t *g, int16_t x, int16_t y, const uint8_t *cols, uint8_t w, uint8_t scale,
                 gfx_color_t c)
{
    if (scale == 0u) {
        scale = 1u;
    }
    for (uint8_t col = 0; col < w; col++) {
        uint8_t bits = cols[col];
        for (uint8_t row = 0; row < 8u; row++) {
            if (((bits >> row) & 1u) == 0u) {
                continue;
            }
            int16_t px = (int16_t)(x + col * scale);
            int16_t py = (int16_t)(y + row * scale);
            if (scale == 1u) {
                gfx_pixel(g, px, py, c);
            } else {
                gfx_fill_rect(g, px, py, scale, scale, c);
            }
        }
    }
}

void gfx_icon(gfx_t *g, int16_t x, int16_t y, const icon_t *icon, gfx_color_t c)
{
    gfx_bitmap8(g, x, y, icon->cols, icon->width, 1u, c);
}

int16_t gfx_text(gfx_t *g, int16_t x, int16_t y, const char *utf8, uint8_t scale, gfx_color_t c)
{
    if (scale == 0u) {
        scale = 1u;
    }
    uint32_t cp;
    while ((cp = utf8_next(&utf8)) != 0u) {
        gfx_bitmap8(g, x, y, font5x7_glyph(cp), FONT5X7_WIDTH, scale, c);
        x = (int16_t)(x + FONT5X7_ADVANCE * scale);
    }
    return x;
}

int16_t gfx_text_width(const char *utf8, uint8_t scale)
{
    if (scale == 0u) {
        scale = 1u;
    }
    uint16_t n = utf8_length(utf8);
    if (n == 0u) {
        return 0;
    }
    /* Последний столбец-интервал не считаем. */
    return (int16_t)(n * FONT5X7_ADVANCE * scale - scale);
}

char *gfx_itoa(int32_t v, char *buf)
{
    char tmp[12];
    uint8_t n = 0;
    uint32_t u = v < 0 ? (uint32_t)(-(v + 1)) + 1u : (uint32_t)v;

    do {
        tmp[n++] = (char)('0' + (u % 10u));
        u /= 10u;
    } while (u != 0u);

    uint8_t i = 0;
    if (v < 0) {
        buf[i++] = '-';
    }
    while (n > 0u) {
        buf[i++] = tmp[--n];
    }
    buf[i] = '\0';
    return buf;
}
