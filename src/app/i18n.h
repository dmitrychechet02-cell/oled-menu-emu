#ifndef I18N_H
#define I18N_H

#include <stdint.h>

typedef enum { LANG_RU = 0, LANG_EN = 1, LANG_COUNT } lang_t;

typedef enum {
    STR_MAIN,
    STR_READINGS,
    STR_SETTINGS,
    STR_SYMBOLS,
    STR_ABOUT,
    STR_AUTOSLEEP,
    STR_BRIGHTNESS,
    STR_LANGUAGE,
    STR_INVERT,
    STR_RESET,
    STR_ON,
    STR_OFF,
    STR_LANG_RU,
    STR_LANG_EN,
    STR_HUMIDITY,
    STR_BATTERY,
    STR_DONE,
    STR_VERSION,
    STR_FONT,
    STR_COUNT
} str_id_t;

void strings_set_lang(lang_t lang);
lang_t strings_lang(void);
const char *tr(uint16_t id);

#endif /* I18N_H */
