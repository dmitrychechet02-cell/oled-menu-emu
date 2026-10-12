#include "app.h"

#include <stddef.h>

#include "../fonts/font5x7.h"
#include "../fonts/icons.h"
#include "../hal/display.h"
#include "i18n.h"

static gfx_t gfx;
static menu_nav_t nav;
static app_settings_t settings;
static app_sensors_t sensors;
static uint32_t uptime_ms;
static uint32_t idle_ms;
static uint32_t toast_ms;
static bool asleep;
static uint8_t symbols_page;

/* ---------- утилиты строк без libc ---------- */

static char *str_append(char *dst, const char *src)
{
    while (*src != '\0') {
        *dst++ = *src++;
    }
    *dst = '\0';
    return dst;
}

/* ---------- настройки ---------- */

static void settings_defaults(void)
{
    settings.autosleep = true;
    settings.brightness = 8;
    settings.lang = LANG_RU;
    settings.invert = false;
}

static void apply_settings(void)
{
    strings_set_lang((lang_t)settings.lang);
    display_set_contrast((uint8_t)(settings.brightness * 25 + 5));
    display_set_inverted(settings.invert);
}

static void action_reset(void)
{
    settings_defaults();
    apply_settings();
    toast_ms = APP_TOAST_MS;
}

/* ---------- имитация датчиков (детерминированная, для тестов) ---------- */

static void sensors_update(void)
{
    uint32_t t = uptime_ms / 1000u;
    uint32_t phase = t % 40u;
    uint32_t tri = phase < 20u ? phase : 40u - phase;    /* 0..20 */
    sensors.temp_x10 = (int16_t)(220u + tri);            /* 22.0..24.0 °C */
    sensors.humidity = (int16_t)(45u + (t / 7u) % 8u);   /* 45..52 % */
    sensors.battery = (int16_t)(100u - (t / 20u) % 81u); /* 100..20 % */
}

/* ---------- экран «Показания» ---------- */

static const char *readings_title(void)
{
    return tr(STR_READINGS);
}

static void draw_battery(gfx_t *g, int16_t x, int16_t y, int16_t percent)
{
    gfx_rect(g, x, y, 14, 7, GFX_ON);
    gfx_vline(g, (int16_t)(x + 14), (int16_t)(y + 2), 3, GFX_ON);
    int16_t fill = (int16_t)(percent * 10 / 100);
    gfx_fill_rect(g, (int16_t)(x + 2), (int16_t)(y + 2), fill, 3, GFX_ON);
}

static void readings_draw(gfx_t *g)
{
    char buf[24];
    char num[12];
    int16_t t = sensors.temp_x10;
    int16_t whole = (int16_t)(t / 10);
    int16_t frac = (int16_t)(t % 10);
    if (frac < 0) {
        frac = (int16_t)-frac;
    }

    char *p = str_append(buf, gfx_itoa(whole, num));
    p = str_append(p, ".");
    str_append(p, gfx_itoa(frac, num));

    gfx_icon(g, 2, 20, &icon_thermo, GFX_ON);
    int16_t x = gfx_text(g, 14, 16, buf, 3u, GFX_ON);
    gfx_text(g, (int16_t)(x + 1), 16, "°C", 1u, GFX_ON);

    gfx_hline(g, 0, 40, GFX_WIDTH, GFX_ON);

    p = str_append(buf, tr(STR_HUMIDITY));
    p = str_append(p, " ");
    p = str_append(p, gfx_itoa(sensors.humidity, num));
    str_append(p, "%");
    gfx_icon(g, 2, 44, &icon_drop, GFX_ON);
    gfx_text(g, 12, 45, buf, 1u, GFX_ON);

    p = str_append(buf, tr(STR_BATTERY));
    p = str_append(p, " ");
    p = str_append(p, gfx_itoa(sensors.battery, num));
    str_append(p, "%");
    draw_battery(g, 1, 55, sensors.battery);
    gfx_text(g, 20, 55, buf, 1u, GFX_ON);
}

static bool readings_key(ui_key_t key)
{
    return key != UI_KEY_BACK;
}

static const menu_screen_t screen_readings = {readings_title, readings_draw, readings_key};

/* ---------- экран «Символы»: таблица шрифта ---------- */

#define SYMBOLS_PAGES 2u
#define SYMBOLS_COLS  16u

static uint16_t symbols_count(uint8_t page)
{
    return page == 0u ? FONT5X7_ASCII_COUNT : FONT5X7_CYR_COUNT + FONT5X7_EXTRA_COUNT;
}

static uint32_t symbols_codepoint(uint8_t page, uint16_t i)
{
    if (page == 0u) {
        return 0x20u + i;
    }
    if (i < FONT5X7_CYR_COUNT) {
        return 0x0410u + i;
    }
    return font5x7_extra[i - FONT5X7_CYR_COUNT].codepoint;
}

static const char *symbols_title(void)
{
    static char title[32];
    char num[12];
    char *p = str_append(title, tr(STR_SYMBOLS));
    p = str_append(p, " ");
    p = str_append(p, gfx_itoa(symbols_page + 1, num));
    p = str_append(p, "/");
    str_append(p, gfx_itoa(SYMBOLS_PAGES, num));
    return title;
}

static void symbols_draw(gfx_t *g)
{
    uint16_t n = symbols_count(symbols_page);
    for (uint16_t i = 0; i < n; i++) {
        int16_t x = (int16_t)((i % SYMBOLS_COLS) * 8u + 1u);
        int16_t y = (int16_t)(12u + (i / SYMBOLS_COLS) * 9u);
        const uint8_t *glyph = font5x7_glyph(symbols_codepoint(symbols_page, i));
        gfx_bitmap8(g, x, y, glyph, FONT5X7_WIDTH, 1u, GFX_ON);
    }
}

static bool symbols_key(ui_key_t key)
{
    switch (key) {
    case UI_KEY_DOWN:
    case UI_KEY_OK:
        symbols_page = (uint8_t)((symbols_page + 1u) % SYMBOLS_PAGES);
        return true;
    case UI_KEY_UP:
        symbols_page = (uint8_t)((symbols_page + SYMBOLS_PAGES - 1u) % SYMBOLS_PAGES);
        return true;
    case UI_KEY_BACK:
        symbols_page = 0u;
        return false;
    case UI_KEY_NONE:
        break;
    }
    return true;
}

static const menu_screen_t screen_symbols = {symbols_title, symbols_draw, symbols_key};

/* ---------- экран «Об устройстве» ---------- */

static const char *about_title(void)
{
    return tr(STR_ABOUT);
}

static void about_draw(gfx_t *g)
{
    char buf[32];
    char *p;
    gfx_text(g, 2, 15, "OLED 128x64 2.42\"", 1u, GFX_ON);
    gfx_text(g, 2, 25, "SSD1309 SPI/I2C", 1u, GFX_ON);
    p = str_append(buf, tr(STR_VERSION));
    p = str_append(p, " ");
    str_append(p, APP_VERSION);
    gfx_text(g, 2, 35, buf, 1u, GFX_ON);
    p = str_append(buf, tr(STR_FONT));
    str_append(p, " 5x7 RU/EN");
    gfx_text(g, 2, 45, buf, 1u, GFX_ON);
    gfx_icon(g, 118, 54, &icon_check, GFX_ON);
}

static bool about_key(ui_key_t key)
{
    return key != UI_KEY_BACK && key != UI_KEY_OK;
}

static const menu_screen_t screen_about = {about_title, about_draw, about_key};

/* ---------- структура меню ---------- */

static const uint16_t lang_choices[] = {STR_LANG_RU, STR_LANG_EN};

static const menu_item_t settings_items[] = {
    {.type = MENU_ITEM_TOGGLE, .label = STR_AUTOSLEEP, .flag = &settings.autosleep},
    {.type = MENU_ITEM_VALUE,
     .label = STR_BRIGHTNESS,
     .value = &settings.brightness,
     .min = 1,
     .max = 10,
     .step = 1,
     .on_change = apply_settings},
    {.type = MENU_ITEM_CHOICE,
     .label = STR_LANGUAGE,
     .choice = &settings.lang,
     .choices = lang_choices,
     .choice_count = 2,
     .on_change = apply_settings},
    {.type = MENU_ITEM_TOGGLE,
     .label = STR_INVERT,
     .flag = &settings.invert,
     .on_change = apply_settings},
    {.type = MENU_ITEM_ACTION, .label = STR_RESET, .action = action_reset},
};

static const menu_t settings_menu = {STR_SETTINGS, settings_items,
                                     sizeof(settings_items) / sizeof(settings_items[0])};

static const menu_item_t main_items[] = {
    {.type = MENU_ITEM_SCREEN,
     .label = STR_READINGS,
     .icon = &icon_chart,
     .screen = &screen_readings},
    {.type = MENU_ITEM_SUBMENU,
     .label = STR_SETTINGS,
     .icon = &icon_gear,
     .submenu = &settings_menu},
    {.type = MENU_ITEM_SCREEN, .label = STR_SYMBOLS, .icon = &icon_font, .screen = &screen_symbols},
    {.type = MENU_ITEM_SCREEN, .label = STR_ABOUT, .icon = &icon_info, .screen = &screen_about},
};

static const menu_t main_menu = {STR_MAIN, main_items, sizeof(main_items) / sizeof(main_items[0])};

/* ---------- публичный API ---------- */

void app_init(void)
{
    uptime_ms = 0u;
    idle_ms = 0u;
    toast_ms = 0u;
    asleep = false;
    symbols_page = 0u;
    settings_defaults();
    sensors_update();
    menu_init(&nav, &main_menu, tr, STR_ON, STR_OFF);
    display_init();
    apply_settings();
    display_set_power(true);
    app_render();
}

void app_key(ui_key_t key)
{
    if (key == UI_KEY_NONE) {
        return;
    }
    idle_ms = 0u;
    if (asleep) {
        /* Первое нажатие только будит экран. */
        asleep = false;
        display_set_power(true);
        return;
    }
    menu_handle_key(&nav, key);
}

void app_tick(uint32_t elapsed_ms)
{
    uptime_ms += elapsed_ms;
    idle_ms += elapsed_ms;
    toast_ms = toast_ms > elapsed_ms ? toast_ms - elapsed_ms : 0u;
    sensors_update();

    if (settings.autosleep && !asleep && idle_ms >= APP_SLEEP_TIMEOUT_MS) {
        asleep = true;
        display_set_power(false);
    }
}

static void draw_toast(gfx_t *g)
{
    const char *msg = tr(STR_DONE);
    int16_t w = (int16_t)(gfx_text_width(msg, 1u) + 12);
    int16_t x = (int16_t)((GFX_WIDTH - w) / 2);
    gfx_fill_rect(g, x, 24, w, 17, GFX_OFF);
    gfx_rect(g, x, 24, w, 17, GFX_ON);
    gfx_text(g, (int16_t)(x + 6), 29, msg, 1u, GFX_ON);
}

void app_render(void)
{
    menu_draw(&nav, &gfx);
    if (toast_ms > 0u) {
        draw_toast(&gfx);
    }
    display_flush(gfx.fb);
}

const gfx_t *app_gfx(void)
{
    return &gfx;
}

const menu_nav_t *app_nav(void)
{
    return &nav;
}

const app_settings_t *app_settings(void)
{
    return &settings;
}

const app_sensors_t *app_sensors(void)
{
    return &sensors;
}

app_screen_id_t app_screen_id(void)
{
    if (nav.screen == &screen_readings) {
        return APP_SCREEN_READINGS;
    }
    if (nav.screen == &screen_symbols) {
        return APP_SCREEN_SYMBOLS;
    }
    if (nav.screen == &screen_about) {
        return APP_SCREEN_ABOUT;
    }
    return APP_SCREEN_MENU;
}

bool app_is_asleep(void)
{
    return asleep;
}
