#include <string.h>

#include "../../src/fonts/font5x7.h"
#include "../../src/gfx/utf8.h"
#include "tt.h"

TT_TEST(utf8_decodes_ascii_and_cyrillic)
{
    const char *s = "Aя°";
    TT_ASSERT_EQ(utf8_next(&s), 'A');
    TT_ASSERT_EQ(utf8_next(&s), 0x044F);
    TT_ASSERT_EQ(utf8_next(&s), 0x00B0);
    TT_ASSERT_EQ(utf8_next(&s), 0);
    TT_ASSERT_EQ(utf8_next(&s), 0); /* конец строки — указатель не двигается */
}

TT_TEST(utf8_broken_sequence_gives_replacement)
{
    const char bad[] = {(char)0xD0, 'A', 0};
    const char *s = bad;
    TT_ASSERT_EQ(utf8_next(&s), UTF8_REPLACEMENT);
    TT_ASSERT_EQ(utf8_next(&s), 'A');
}

TT_TEST(utf8_length_counts_codepoints)
{
    TT_ASSERT_EQ(utf8_length("Привет"), 6);
    TT_ASSERT_EQ(utf8_length("OK"), 2);
    TT_ASSERT_EQ(utf8_length(""), 0);
}

TT_TEST(font_has_all_ascii_printable)
{
    for (uint32_t cp = 0x20; cp <= 0x7E; cp++) {
        TT_ASSERT(font5x7_has_glyph(cp));
    }
}

TT_TEST(font_has_full_russian_alphabet)
{
    const char *s = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя";
    uint32_t cp;
    int n = 0;
    while ((cp = utf8_next(&s)) != 0) {
        TT_ASSERT(font5x7_has_glyph(cp));
        n++;
    }
    TT_ASSERT_EQ(n, 66);
}

TT_TEST(font_unknown_codepoint_falls_back)
{
    TT_ASSERT(!font5x7_has_glyph(0x4E2D)); /* 中 */
    TT_ASSERT(font5x7_glyph(0x4E2D) == font5x7_fallback);
    TT_ASSERT(font5x7_glyph(0x4E2D) != NULL);
}

TT_TEST(font_glyphs_fit_seven_rows)
{
    /* Бит 7 (восьмая строка) должен быть пуст: шрифт 7 px в высоту. */
    for (uint32_t cp = 0x20; cp <= 0x7E; cp++) {
        const uint8_t *g = font5x7_glyph(cp);
        for (int c = 0; c < FONT5X7_WIDTH; c++) {
            TT_ASSERT_EQ(g[c] & 0x80, 0);
        }
    }
}

TT_TEST(font_glyph_A_shape)
{
    /* .###. / #...# / #...# / ##### / #...# / #...# / #...# */
    static const uint8_t expected[5] = {0x7E, 0x09, 0x09, 0x09, 0x7E};
    TT_ASSERT(memcmp(font5x7_glyph('A'), expected, 5) == 0);
}
