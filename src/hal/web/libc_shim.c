/*
 * В сборке wasm32 нет libc. Компилятор может сам вставлять вызовы memset/memcpy
 * (например, при обнулении массивов), поэтому даём минимальные реализации.
 * На хосте и на плате используется libc, этот файл туда не попадает.
 */
#include <stddef.h>

void *memset(void *dst, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dst, const void *src, size_t n);

void *memset(void *dst, int c, size_t n)
{
    unsigned char *d = dst;
    while (n-- > 0u) {
        *d++ = (unsigned char)c;
    }
    return dst;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n-- > 0u) {
        *d++ = *s++;
    }
    return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    if (d < s) {
        while (n-- > 0u) {
            *d++ = *s++;
        }
    } else {
        while (n-- > 0u) {
            d[n] = s[n];
        }
    }
    return dst;
}
