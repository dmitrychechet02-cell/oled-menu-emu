#include "utf8.h"

static int is_cont(uint8_t b)
{
    return (b & 0xC0u) == 0x80u;
}

uint32_t utf8_next(const char **s)
{
    const uint8_t *p = (const uint8_t *)*s;
    uint8_t b0 = p[0];

    if (b0 == 0u) {
        return 0u;
    }
    if (b0 < 0x80u) {
        *s += 1;
        return b0;
    }
    if ((b0 & 0xE0u) == 0xC0u && is_cont(p[1])) {
        *s += 2;
        return ((uint32_t)(b0 & 0x1Fu) << 6) | (uint32_t)(p[1] & 0x3Fu);
    }
    if ((b0 & 0xF0u) == 0xE0u && is_cont(p[1]) && is_cont(p[2])) {
        *s += 3;
        return ((uint32_t)(b0 & 0x0Fu) << 12) | ((uint32_t)(p[1] & 0x3Fu) << 6) |
               (uint32_t)(p[2] & 0x3Fu);
    }
    if ((b0 & 0xF8u) == 0xF0u && is_cont(p[1]) && is_cont(p[2]) && is_cont(p[3])) {
        *s += 4;
        return ((uint32_t)(b0 & 0x07u) << 18) | ((uint32_t)(p[1] & 0x3Fu) << 12) |
               ((uint32_t)(p[2] & 0x3Fu) << 6) | (uint32_t)(p[3] & 0x3Fu);
    }
    *s += 1;
    return UTF8_REPLACEMENT;
}

uint16_t utf8_length(const char *s)
{
    uint16_t n = 0;
    while (utf8_next(&s) != 0u) {
        n++;
    }
    return n;
}
