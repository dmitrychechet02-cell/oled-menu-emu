/*
 * Точка входа WebAssembly. Экспортирует API для web/emulator.js.
 * Собирается только clang --target=wasm32 (см. Makefile, цель wasm).
 */
#include "../../app/app.h"
#include "display_web.h"
#include "emu_api.h"

#define EMU_EXPORT(name) __attribute__((export_name(#name)))

EMU_EXPORT(emu_init) void emu_init(void)
{
    app_init();
}

EMU_EXPORT(emu_key) void emu_key(int32_t key)
{
    if (key < UI_KEY_NONE || key > UI_KEY_BACK) {
        return;
    }
    app_key((ui_key_t)key);
    app_render();
}

EMU_EXPORT(emu_tick) void emu_tick(uint32_t elapsed_ms)
{
    app_tick(elapsed_ms);
    app_render();
}

EMU_EXPORT(emu_display_ram) const uint8_t *emu_display_ram(void)
{
    return display_web_ram();
}

EMU_EXPORT(emu_contrast) uint32_t emu_contrast(void)
{
    return display_web_contrast();
}

EMU_EXPORT(emu_inverted) uint32_t emu_inverted(void)
{
    return display_web_inverted() ? 1u : 0u;
}

EMU_EXPORT(emu_powered) uint32_t emu_powered(void)
{
    return display_web_powered() ? 1u : 0u;
}

EMU_EXPORT(emu_flush_count) uint32_t emu_flush_count(void)
{
    return display_web_flush_count();
}

EMU_EXPORT(emu_screen_id) uint32_t emu_screen_id(void)
{
    return (uint32_t)app_screen_id();
}

EMU_EXPORT(emu_menu_depth) uint32_t emu_menu_depth(void)
{
    return app_nav()->depth;
}

EMU_EXPORT(emu_selected) uint32_t emu_selected(void)
{
    return menu_selected(app_nav());
}

EMU_EXPORT(emu_editing) uint32_t emu_editing(void)
{
    return app_nav()->editing ? 1u : 0u;
}

/* Заголовок текущего экрана, UTF-8 с нулём на конце. */
EMU_EXPORT(emu_title) const char *emu_title(void)
{
    return menu_title(app_nav());
}

EMU_EXPORT(emu_brightness) int32_t emu_brightness(void)
{
    return app_settings()->brightness;
}

EMU_EXPORT(emu_lang) uint32_t emu_lang(void)
{
    return app_settings()->lang;
}
