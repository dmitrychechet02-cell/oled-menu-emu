#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include <stdint.h>

#include "../fonts/icons.h"
#include "../gfx/gfx.h"

#define MENU_MAX_DEPTH    4
#define MENU_VISIBLE_ROWS 5
#define MENU_TITLE_H      11 /* высота инверсной шапки */
#define MENU_ROW_Y0       13 /* верх первой строки */
#define MENU_ROW_H        10

typedef enum {
    UI_KEY_NONE = 0,
    UI_KEY_UP = 1,
    UI_KEY_DOWN = 2,
    UI_KEY_OK = 3,
    UI_KEY_BACK = 4,
} ui_key_t;

/* Перевод строки по id (язык выбирает приложение). */
typedef const char *(*menu_tr_fn)(uint16_t str_id);

/* Полноэкранный экран: рисует область ниже шапки и сам обрабатывает кнопки. */
typedef struct {
    const char *(*title)(void);
    void (*draw)(gfx_t *g);
    bool (*on_key)(ui_key_t key); /* false -> закрыть экран */
} menu_screen_t;

typedef enum {
    MENU_ITEM_SUBMENU,
    MENU_ITEM_SCREEN,
    MENU_ITEM_ACTION,
    MENU_ITEM_TOGGLE,
    MENU_ITEM_VALUE,
    MENU_ITEM_CHOICE,
} menu_item_type_t;

typedef struct menu menu_t;

typedef struct {
    menu_item_type_t type;
    uint16_t label;
    const icon_t *icon;          /* может быть NULL */
    const menu_t *submenu;       /* SUBMENU */
    const menu_screen_t *screen; /* SCREEN */
    void (*action)(void);        /* ACTION */
    bool *flag;                  /* TOGGLE */
    int16_t *value;              /* VALUE */
    int16_t min, max, step;      /* VALUE */
    uint8_t *choice;             /* CHOICE: индекс выбранного */
    const uint16_t *choices;     /* CHOICE: id строк вариантов */
    uint8_t choice_count;        /* CHOICE */
    void (*on_change)(void);     /* TOGGLE/VALUE/CHOICE, может быть NULL */
} menu_item_t;

struct menu {
    uint16_t title;
    const menu_item_t *items;
    uint8_t count;
};

typedef struct {
    const menu_t *stack[MENU_MAX_DEPTH];
    uint8_t selected[MENU_MAX_DEPTH];
    uint8_t scroll[MENU_MAX_DEPTH];
    uint8_t depth; /* 0 — корневое меню */
    bool editing;  /* редактируется VALUE-пункт */
    const menu_screen_t *screen;
    menu_tr_fn tr;
    uint16_t str_on;
    uint16_t str_off;
} menu_nav_t;

void menu_init(menu_nav_t *nav, const menu_t *root, menu_tr_fn tr, uint16_t str_on,
               uint16_t str_off);
void menu_handle_key(menu_nav_t *nav, ui_key_t key);
void menu_draw(const menu_nav_t *nav, gfx_t *g);

const menu_t *menu_current(const menu_nav_t *nav);
uint8_t menu_selected(const menu_nav_t *nav);
const char *menu_title(const menu_nav_t *nav);

#endif /* MENU_H */
