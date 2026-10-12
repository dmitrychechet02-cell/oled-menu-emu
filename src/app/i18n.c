#include "i18n.h"

static lang_t current = LANG_RU;

/* Строки не длиннее 13 символов, иначе не влезут рядом со значением в строке меню. */
static const char *const table[STR_COUNT][LANG_COUNT] = {
    [STR_MAIN] = {"Главное меню", "Main menu"},
    [STR_READINGS] = {"Показания", "Readings"},
    [STR_SETTINGS] = {"Настройки", "Settings"},
    [STR_SYMBOLS] = {"Символы", "Symbols"},
    [STR_ABOUT] = {"Об устройстве", "About"},
    [STR_AUTOSLEEP] = {"Автосон", "Auto sleep"},
    [STR_BRIGHTNESS] = {"Яркость", "Brightness"},
    [STR_LANGUAGE] = {"Язык", "Language"},
    [STR_INVERT] = {"Инверсия", "Invert"},
    [STR_RESET] = {"Сброс", "Reset"},
    [STR_ON] = {"ВКЛ", "ON"},
    [STR_OFF] = {"ВЫКЛ", "OFF"},
    [STR_LANG_RU] = {"RU", "RU"},
    [STR_LANG_EN] = {"EN", "EN"},
    [STR_HUMIDITY] = {"Влажность", "Humidity"},
    [STR_BATTERY] = {"Батарея", "Battery"},
    [STR_DONE] = {"Готово!", "Done!"},
    [STR_VERSION] = {"Версия", "Version"},
    [STR_FONT] = {"Шрифт", "Font"},
};

void strings_set_lang(lang_t lang)
{
    if (lang < LANG_COUNT) {
        current = lang;
    }
}

lang_t strings_lang(void)
{
    return current;
}

const char *tr(uint16_t id)
{
    if (id >= STR_COUNT) {
        return "?";
    }
    return table[id][current];
}
