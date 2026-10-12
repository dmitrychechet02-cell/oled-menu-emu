#ifndef FONT5X7_H
#define FONT5X7_H

#include <stdint.h>

#define FONT5X7_WIDTH       5
#define FONT5X7_HEIGHT      7
#define FONT5X7_ADVANCE     6 /* ширина глифа + 1 столбец интервала */
#define FONT5X7_ASCII_COUNT 95
#define FONT5X7_CYR_COUNT   64
#define FONT5X7_EXTRA_COUNT 3

typedef struct {
    uint16_t codepoint;
    uint8_t cols[FONT5X7_WIDTH];
} font5x7_extra_t;

extern const uint8_t font5x7_ascii[FONT5X7_ASCII_COUNT][FONT5X7_WIDTH];
extern const uint8_t font5x7_cyr[FONT5X7_CYR_COUNT][FONT5X7_WIDTH];
extern const font5x7_extra_t font5x7_extra[FONT5X7_EXTRA_COUNT];
extern const uint8_t font5x7_fallback[FONT5X7_WIDTH];

/* Глиф для кодовой точки Unicode; для неизвестных — font5x7_fallback. Никогда не NULL. */
const uint8_t *font5x7_glyph(uint32_t codepoint);

/* 1, если для кодовой точки есть собственный глиф. */
int font5x7_has_glyph(uint32_t codepoint);

#endif /* FONT5X7_H */
