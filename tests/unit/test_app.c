#include <string.h>

#include "../../src/app/app.h"
#include "../../src/app/i18n.h"
#include "display_mock.h"
#include "tt.h"

static void press(ui_key_t k)
{
    app_key(k);
    app_render();
}

TT_TEST(app_init_renders_main_menu)
{
    app_init();
    TT_ASSERT(display_mock.initialized);
    TT_ASSERT(display_mock.powered);
    TT_ASSERT(display_mock.flushes >= 1);
    TT_ASSERT_STR(menu_title(app_nav()), "Главное меню");
    TT_ASSERT_EQ(app_screen_id(), APP_SCREEN_MENU);
    TT_ASSERT(memcmp(display_mock.ram, app_gfx()->fb, 1024) == 0);
}

TT_TEST(app_open_readings_and_back)
{
    app_init();
    press(UI_KEY_OK);
    TT_ASSERT_EQ(app_screen_id(), APP_SCREEN_READINGS);
    TT_ASSERT_STR(menu_title(app_nav()), "Показания");
    press(UI_KEY_BACK);
    TT_ASSERT_EQ(app_screen_id(), APP_SCREEN_MENU);
}

TT_TEST(app_language_switch_changes_titles)
{
    app_init();
    press(UI_KEY_DOWN); /* Настройки */
    press(UI_KEY_OK);
    TT_ASSERT_STR(menu_title(app_nav()), "Настройки");
    press(UI_KEY_DOWN);
    press(UI_KEY_DOWN); /* Язык */
    press(UI_KEY_OK);
    TT_ASSERT_EQ(app_settings()->lang, LANG_EN);
    TT_ASSERT_STR(menu_title(app_nav()), "Settings");
    press(UI_KEY_BACK);
    TT_ASSERT_STR(menu_title(app_nav()), "Main menu");
    strings_set_lang(LANG_RU);
}

TT_TEST(app_brightness_maps_to_contrast)
{
    app_init();
    TT_ASSERT_EQ(display_mock.contrast, 8 * 25 + 5);
    press(UI_KEY_DOWN);
    press(UI_KEY_OK);   /* Настройки */
    press(UI_KEY_DOWN); /* Яркость */
    press(UI_KEY_OK);   /* редактирование */
    press(UI_KEY_UP);
    press(UI_KEY_UP);
    press(UI_KEY_UP);
    TT_ASSERT_EQ(app_settings()->brightness, 10);
    TT_ASSERT_EQ(display_mock.contrast, 255);
}

TT_TEST(app_invert_and_reset)
{
    app_init();
    press(UI_KEY_DOWN);
    press(UI_KEY_OK);
    press(UI_KEY_UP); /* по кругу на последний пункт — Сброс */
    press(UI_KEY_UP); /* Инверсия */
    press(UI_KEY_OK);
    TT_ASSERT(display_mock.inverted);
    press(UI_KEY_DOWN); /* Сброс */
    press(UI_KEY_OK);
    TT_ASSERT(!display_mock.inverted);
    TT_ASSERT(!app_settings()->invert);
}

TT_TEST(app_autosleep_and_wake)
{
    app_init();
    app_tick(APP_SLEEP_TIMEOUT_MS - 1);
    TT_ASSERT(!app_is_asleep());
    app_tick(1);
    TT_ASSERT(app_is_asleep());
    TT_ASSERT(!display_mock.powered);
    press(UI_KEY_DOWN); /* только будит */
    TT_ASSERT(!app_is_asleep());
    TT_ASSERT(display_mock.powered);
    TT_ASSERT_EQ(menu_selected(app_nav()), 0);
}

TT_TEST(app_sensors_are_deterministic)
{
    app_init();
    TT_ASSERT_EQ(app_sensors()->temp_x10, 220);
    TT_ASSERT_EQ(app_sensors()->battery, 100);
    app_tick(10000);
    TT_ASSERT_EQ(app_sensors()->temp_x10, 230);
    app_tick(10000);
    TT_ASSERT_EQ(app_sensors()->temp_x10, 240);
}

TT_TEST(app_symbols_screen_pages)
{
    app_init();
    press(UI_KEY_DOWN);
    press(UI_KEY_DOWN);
    press(UI_KEY_OK);
    TT_ASSERT_EQ(app_screen_id(), APP_SCREEN_SYMBOLS);
    TT_ASSERT_STR(menu_title(app_nav()), "Символы 1/2");
    press(UI_KEY_DOWN);
    TT_ASSERT_STR(menu_title(app_nav()), "Символы 2/2");
    press(UI_KEY_BACK);
    TT_ASSERT_EQ(app_screen_id(), APP_SCREEN_MENU);
}
