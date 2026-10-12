#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Абстракция дисплея. Реализации:
 *   hal/ssd1309/display_ssd1309.c — настоящая плата (SPI/I2C через порт);
 *   hal/web/display_web.c        — эмулятор в браузере (WebAssembly);
 *   tests/unit/display_mock.c    — unit-тесты на хосте.
 */
void display_init(void);
void display_flush(const uint8_t *fb); /* 1024 байта в раскладке SSD1309 */
void display_set_contrast(uint8_t contrast);
void display_set_inverted(bool inverted);
void display_set_power(bool on);

#endif /* DISPLAY_H */
