#include <stddef.h>
#include <string.h>

#include "../../src/menu/menu.h"
#include "tt.h"

/* Тестовое меню, независимое от приложения. */
enum { S_ROOT, S_SUB, S_A, S_B, S_C, S_D, S_E, S_F, S_G, S_ON, S_OFF, S_X, S_Y, S_COUNT };
static const char *names[S_COUNT] = {"Root", "Sub", "A",  "B",   "C", "D", "E",
                                     "F",    "G",   "on", "off", "X", "Y"};
static const char *t(uint16_t id)
{
    return names[id];
}

static bool flag;
static int16_t value = 5;
static uint8_t choice;
static int changes;
static int actions;
static void on_change(void)
{
    changes++;
}
static void act(void)
{
    actions++;
}

static const uint16_t xy[] = {S_X, S_Y};
static const menu_item_t sub_items[] = {
    {.type = MENU_ITEM_TOGGLE, .label = S_A, .flag = &flag, .on_change = on_change},
    {.type = MENU_ITEM_VALUE,
     .label = S_B,
     .value = &value,
     .min = 0,
     .max = 6,
     .step = 1,
     .on_change = on_change},
    {.type = MENU_ITEM_CHOICE,
     .label = S_C,
     .choice = &choice,
     .choices = xy,
     .choice_count = 2,
     .on_change = on_change},
    {.type = MENU_ITEM_ACTION, .label = S_D, .action = act},
};
static const menu_t sub = {S_SUB, sub_items, 4};

static bool screen_open_keys;
static const char *screen_title(void)
{
    return "Screen";
}
static void screen_draw(gfx_t *g)
{
    (void)g;
}
static bool screen_key(ui_key_t k)
{
    screen_open_keys = true;
    return k != UI_KEY_BACK;
}
static const menu_screen_t scr = {screen_title, screen_draw, screen_key};

static const menu_item_t root_items[] = {
    {.type = MENU_ITEM_SUBMENU, .label = S_SUB, .submenu = &sub},
    {.type = MENU_ITEM_SCREEN, .label = S_A, .screen = &scr},
    {.type = MENU_ITEM_ACTION, .label = S_B, .action = act},
    {.type = MENU_ITEM_ACTION, .label = S_C, .action = act},
    {.type = MENU_ITEM_ACTION, .label = S_D, .action = act},
    {.type = MENU_ITEM_ACTION, .label = S_E, .action = act},
    {.type = MENU_ITEM_ACTION, .label = S_F, .action = act},
    {.type = MENU_ITEM_ACTION, .label = S_G, .action = act},
};
static const menu_t root = {S_ROOT, root_items, 8};

static menu_nav_t nav;

static void setup(void)
{
    flag = false;
    value = 5;
    choice = 0;
    changes = 0;
    actions = 0;
    screen_open_keys = false;
    menu_init(&nav, &root, t, S_ON, S_OFF);
}

TT_TEST(menu_starts_at_root_first_item)
{
    setup();
    TT_ASSERT(menu_current(&nav) == &root);
    TT_ASSERT_EQ(menu_selected(&nav), 0);
    TT_ASSERT_STR(menu_title(&nav), "Root");
}

TT_TEST(menu_down_up_wraps_around)
{
    setup();
    menu_handle_key(&nav, UI_KEY_UP);
    TT_ASSERT_EQ(menu_selected(&nav), 7);
    menu_handle_key(&nav, UI_KEY_DOWN);
    TT_ASSERT_EQ(menu_selected(&nav), 0);
}

TT_TEST(menu_scroll_keeps_selection_visible)
{
    setup();
    for (int i = 0; i < 6; i++) {
        menu_handle_key(&nav, UI_KEY_DOWN);
    }
    TT_ASSERT_EQ(menu_selected(&nav), 6);
    TT_ASSERT_EQ(nav.scroll[0], 2); /* видны 2..6 */
    menu_handle_key(&nav, UI_KEY_UP);
    menu_handle_key(&nav, UI_KEY_UP);
    menu_handle_key(&nav, UI_KEY_UP);
    TT_ASSERT_EQ(menu_selected(&nav), 3);
    TT_ASSERT_EQ(nav.scroll[0], 2);
    menu_handle_key(&nav, UI_KEY_UP);
    TT_ASSERT_EQ(nav.scroll[0], 2);
    menu_handle_key(&nav, UI_KEY_UP);
    TT_ASSERT_EQ(nav.scroll[0], 1);
}

TT_TEST(menu_enter_and_leave_submenu)
{
    setup();
    menu_handle_key(&nav, UI_KEY_OK);
    TT_ASSERT(menu_current(&nav) == &sub);
    TT_ASSERT_EQ(nav.depth, 1);
    menu_handle_key(&nav, UI_KEY_BACK);
    TT_ASSERT(menu_current(&nav) == &root);
    menu_handle_key(&nav, UI_KEY_BACK); /* на корне BACK ничего не делает */
    TT_ASSERT(menu_current(&nav) == &root);
}

TT_TEST(menu_toggle_flips_and_notifies)
{
    setup();
    menu_handle_key(&nav, UI_KEY_OK);
    menu_handle_key(&nav, UI_KEY_OK);
    TT_ASSERT(flag);
    TT_ASSERT_EQ(changes, 1);
}

TT_TEST(menu_value_edit_clamps_to_range)
{
    setup();
    menu_handle_key(&nav, UI_KEY_OK);   /* в подменю */
    menu_handle_key(&nav, UI_KEY_DOWN); /* пункт B */
    menu_handle_key(&nav, UI_KEY_OK);   /* режим редактирования */
    TT_ASSERT(nav.editing);
    menu_handle_key(&nav, UI_KEY_UP);
    menu_handle_key(&nav, UI_KEY_UP);
    menu_handle_key(&nav, UI_KEY_UP);
    TT_ASSERT_EQ(value, 6);
    TT_ASSERT_EQ(changes, 1); /* упёрлись в max — лишних уведомлений нет */
    menu_handle_key(&nav, UI_KEY_BACK);
    TT_ASSERT(!nav.editing);
    TT_ASSERT(menu_current(&nav) == &sub); /* BACK вышел из редактирования, не из меню */
}

TT_TEST(menu_choice_cycles)
{
    setup();
    menu_handle_key(&nav, UI_KEY_OK);
    menu_handle_key(&nav, UI_KEY_DOWN);
    menu_handle_key(&nav, UI_KEY_DOWN);
    menu_handle_key(&nav, UI_KEY_OK);
    TT_ASSERT_EQ(choice, 1);
    menu_handle_key(&nav, UI_KEY_OK);
    TT_ASSERT_EQ(choice, 0);
}

TT_TEST(menu_screen_receives_keys_and_closes_on_back)
{
    setup();
    menu_handle_key(&nav, UI_KEY_DOWN);
    menu_handle_key(&nav, UI_KEY_OK);
    TT_ASSERT(nav.screen == &scr);
    TT_ASSERT_STR(menu_title(&nav), "Screen");
    menu_handle_key(&nav, UI_KEY_DOWN);
    TT_ASSERT(screen_open_keys);
    TT_ASSERT(nav.screen == &scr);
    TT_ASSERT_EQ(menu_selected(&nav), 1); /* кнопки не ушли в меню */
    menu_handle_key(&nav, UI_KEY_BACK);
    TT_ASSERT(nav.screen == NULL);
}

TT_TEST(menu_draw_highlights_selected_row)
{
    static gfx_t g;
    setup();
    menu_draw(&nav, &g);
    /* шапка залита */
    TT_ASSERT(gfx_get_pixel(&g, 0, 0));
    /* у первой строки (выбранной) левый край инвертирован, у второй — нет */
    TT_ASSERT(gfx_get_pixel(&g, 0, MENU_ROW_Y0));
    TT_ASSERT(!gfx_get_pixel(&g, 0, MENU_ROW_Y0 + MENU_ROW_H));
}
