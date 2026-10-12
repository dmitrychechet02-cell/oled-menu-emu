#include "font5x7.h"

#include <stddef.h>

static const uint8_t *lookup(uint32_t cp)
{
    if (cp >= 0x20u && cp <= 0x7Eu) {
        return font5x7_ascii[cp - 0x20u];
    }
    if (cp >= 0x0410u && cp <= 0x044Fu) {
        return font5x7_cyr[cp - 0x0410u];
    }
    for (size_t i = 0; i < FONT5X7_EXTRA_COUNT; i++) {
        if (font5x7_extra[i].codepoint == cp) {
            return font5x7_extra[i].cols;
        }
    }
    return NULL;
}

const uint8_t *font5x7_glyph(uint32_t codepoint)
{
    const uint8_t *g = lookup(codepoint);
    return g != NULL ? g : font5x7_fallback;
}

int font5x7_has_glyph(uint32_t codepoint)
{
    return lookup(codepoint) != NULL;
}
