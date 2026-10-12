#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include <stdint.h>

#include "../gfx/gfx.h"
#include "../menu/menu.h"

#define APP_VERSION          "0.1.0"
#define APP_SLEEP_TIMEOUT_MS 30000u
#define APP_TOAST_MS         1500u

typedef struct {
    bool autosleep;
    int16_t brightness; /* 1..10 */
    uint8_t lang;       /* lang_t */
    bool invert;
} app_settings_t;

typedef struct {
    int16_t temp_x10; /* температура * 10, °C */
    int16_t humidity; /* % */
    int16_t battery;  /* % */
} app_sensors_t;

/* Идентификаторы экранов — для эмулятора и тестов. */
typedef enum {
    APP_SCREEN_MENU = 0,
    APP_SCREEN_READINGS = 1,
    APP_SCREEN_SYMBOLS = 2,
    APP_SCREEN_ABOUT = 3,
} app_screen_id_t;

void app_init(void);
void app_key(ui_key_t key);
void app_tick(uint32_t elapsed_ms);
void app_render(void);

const gfx_t *app_gfx(void);
const menu_nav_t *app_nav(void);
const app_settings_t *app_settings(void);
const app_sensors_t *app_sensors(void);
app_screen_id_t app_screen_id(void);
bool app_is_asleep(void);

#endif /* APP_H */
