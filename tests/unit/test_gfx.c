#include <string.h>

#include "../../src/gfx/gfx.h"
#include "tt.h"

static gfx_t g;

TT_TEST(gfx_clear_zeroes_framebuffer)
{
    memset(g.fb, 0xFF, sizeof g.fb);
    gfx_clear(&g);
    for (int i = 0; i < GFX_FB_SIZE; i++) {
        TT_ASSERT_EQ(g.fb[i], 0);
    }
}

TT_TEST(gfx_pixel_uses_ssd1309_page_layout)
{
    gfx_clear(&g);
    gfx_pixel(&g, 5, 0, GFX_ON);    /* страница 0, бит 0 */
    gfx_pixel(&g, 5, 7, GFX_ON);    /* страница 0, бит 7 */
    gfx_pixel(&g, 127, 63, GFX_ON); /* страница 7, последний байт, бит 7 */
    TT_ASSERT_EQ(g.fb[5], 0x81);
    TT_ASSERT_EQ(g.fb[1023], 0x80);
    gfx_pixel(&g, 2, 9, GFX_ON); /* страница 1, бит 1 */
    TT_ASSERT_EQ(g.fb[128 + 2], 0x02);
}

TT_TEST(gfx_pixel_clips_out_of_bounds)
{
    gfx_clear(&g);
    gfx_pixel(&g, -1, 0, GFX_ON);
    gfx_pixel(&g, 128, 0, GFX_ON);
    gfx_pixel(&g, 0, 64, GFX_ON);
    gfx_pixel(&g, 0, -3, GFX_ON);
    for (int i = 0; i < GFX_FB_SIZE; i++) {
        TT_ASSERT_EQ(g.fb[i], 0);
    }
    TT_ASSERT(!gfx_get_pixel(&g, 200, 200));
}

TT_TEST(gfx_invert_toggles_pixel)
{
    gfx_clear(&g);
    gfx_pixel(&g, 10, 10, GFX_INVERT);
    TT_ASSERT(gfx_get_pixel(&g, 10, 10));
    gfx_pixel(&g, 10, 10, GFX_INVERT);
    TT_ASSERT(!gfx_get_pixel(&g, 10, 10));
}

TT_TEST(gfx_rect_draws_only_border)
{
    gfx_clear(&g);
    gfx_rect(&g, 10, 10, 5, 4, GFX_ON);
    TT_ASSERT(gfx_get_pixel(&g, 10, 10));
    TT_ASSERT(gfx_get_pixel(&g, 14, 13));
    TT_ASSERT(!gfx_get_pixel(&g, 12, 11)); /* внутри пусто */
}

TT_TEST(gfx_fill_rect_counts_pixels)
{
    gfx_clear(&g);
    gfx_fill_rect(&g, 0, 0, 10, 3, GFX_ON);
    int on = 0;
    for (int y = 0; y < GFX_HEIGHT; y++) {
        for (int x = 0; x < GFX_WIDTH; x++) {
            on += gfx_get_pixel(&g, (int16_t)x, (int16_t)y);
        }
    }
    TT_ASSERT_EQ(on, 30);
}

TT_TEST(gfx_line_diagonal_hits_endpoints)
{
    gfx_clear(&g);
    gfx_line(&g, 0, 0, 7, 7, GFX_ON);
    for (int i = 0; i < 8; i++) {
        TT_ASSERT(gfx_get_pixel(&g, (int16_t)i, (int16_t)i));
    }
}

TT_TEST(gfx_text_width_matches_advance)
{
    TT_ASSERT_EQ(gfx_text_width("", 1), 0);
    TT_ASSERT_EQ(gfx_text_width("A", 1), 5);
    TT_ASSERT_EQ(gfx_text_width("AB", 1), 11);
    TT_ASSERT_EQ(gfx_text_width("Яр", 1), 11); /* кириллица считается по символам, не байтам */
    TT_ASSERT_EQ(gfx_text_width("12", 3), 33);
}

TT_TEST(gfx_text_returns_cursor_after_text)
{
    gfx_clear(&g);
    TT_ASSERT_EQ(gfx_text(&g, 10, 0, "Hi", 1, GFX_ON), 22);
    TT_ASSERT_EQ(gfx_text(&g, 0, 20, "Да", 2, GFX_ON), 24);
}

TT_TEST(gfx_itoa_handles_sign_and_zero)
{
    char buf[12];
    TT_ASSERT_STR(gfx_itoa(0, buf), "0");
    TT_ASSERT_STR(gfx_itoa(42, buf), "42");
    TT_ASSERT_STR(gfx_itoa(-7, buf), "-7");
    TT_ASSERT_STR(gfx_itoa(-2147483647 - 1, buf), "-2147483648");
}
