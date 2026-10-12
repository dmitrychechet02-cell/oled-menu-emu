#ifndef UTF8_H
#define UTF8_H

#include <stdint.h>

#define UTF8_REPLACEMENT 0xFFFDu

/*
 * Декодирует одну кодовую точку из UTF-8 и сдвигает *s.
 * На конце строки возвращает 0 и не двигает указатель.
 * Битые последовательности -> UTF8_REPLACEMENT (сдвиг на 1 байт).
 */
uint32_t utf8_next(const char **s);

/* Количество кодовых точек в строке. */
uint16_t utf8_length(const char *s);

#endif /* UTF8_H */
