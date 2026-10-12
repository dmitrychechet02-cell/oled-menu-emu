#include "menu.h"

#include <stddef.h>

const menu_t *menu_current(const menu_nav_t *nav)
{
    return nav->stack[nav->depth];
}

uint8_t menu_selected(const menu_nav_t *nav)
{
    return nav->selected[nav->depth];
}

const char *menu_title(const menu_nav_t *nav)
{
    if (nav->screen != NULL) {
        return nav->screen->title();
    }
    return nav->tr(menu_current(nav)->title);
}

void menu_init(menu_nav_t *nav, const menu_t *root, menu_tr_fn tr, uint16_t str_on,
               uint16_t str_off)
{
    for (uint8_t i = 0; i < MENU_MAX_DEPTH; i++) {
        nav->stack[i] = NULL;
        nav->selected[i] = 0u;
        nav->scroll[i] = 0u;
    }
    nav->stack[0] = root;
    nav->depth = 0u;
    nav->editing = false;
    nav->screen = NULL;
    nav->tr = tr;
    nav->str_on = str_on;
    nav->str_off = str_off;
}

static void fix_scroll(menu_nav_t *nav)
{
    uint8_t d = nav->depth;
    if (nav->selected[d] < nav->scroll[d]) {
        nav->scroll[d] = nav->selected[d];
    } else if (nav->selected[d] >= nav->scroll[d] + MENU_VISIBLE_ROWS) {
        nav->scroll[d] = (uint8_t)(nav->selected[d] - MENU_VISIBLE_ROWS + 1u);
    }
}

static void notify(const menu_item_t *it)
{
    if (it->on_change != NULL) {
        it->on_change();
    }
}

static void edit_value(const menu_item_t *it, ui_key_t key)
{
    int16_t v = *it->value;
    if (key == UI_KEY_UP) {
        v = (int16_t)(v + it->step);
    } else if (key == UI_KEY_DOWN) {
        v = (int16_t)(v - it->step);
    }
    if (v > it->max) {
        v = it->max;
    }
    if (v < it->min) {
        v = it->min;
    }
    if (v != *it->value) {
        *it->value = v;
        notify(it);
    }
}

static void activate(menu_nav_t *nav, const menu_item_t *it)
{
    switch (it->type) {
    case MENU_ITEM_SUBMENU:
        if (nav->depth + 1u < MENU_MAX_DEPTH && it->submenu != NULL) {
            nav->depth++;
            nav->stack[nav->depth] = it->submenu;
            nav->selected[nav->depth] = 0u;
            nav->scroll[nav->depth] = 0u;
        }
        break;
    case MENU_ITEM_SCREEN:
        nav->screen = it->screen;
        break;
    case MENU_ITEM_ACTION:
        if (it->action != NULL) {
            it->action();
        }
        break;
    case MENU_ITEM_TOGGLE:
        *it->flag = !*it->flag;
        notify(it);
        break;
    case MENU_ITEM_VALUE:
        nav->editing = true;
        break;
    case MENU_ITEM_CHOICE:
        *it->choice = (uint8_t)((*it->choice + 1u) % it->choice_count);
        notify(it);
        break;
    }
}

void menu_handle_key(menu_nav_t *nav, ui_key_t key)
{
    if (key == UI_KEY_NONE) {
        return;
    }
    if (nav->screen != NULL) {
        if (!nav->screen->on_key(key)) {
            nav->screen = NULL;
        }
        return;
    }

    const menu_t *m = menu_current(nav);
    uint8_t d = nav->depth;
    const menu_item_t *it = &m->items[nav->selected[d]];

    if (nav->editing) {
        if (key == UI_KEY_OK || key == UI_KEY_BACK) {
            nav->editing = false;
        } else {
            edit_value(it, key);
        }
        return;
    }

    switch (key) {
    case UI_KEY_UP:
        nav->selected[d] =
            nav->selected[d] == 0u ? (uint8_t)(m->count - 1u) : (uint8_t)(nav->selected[d] - 1u);
        fix_scroll(nav);
        break;
    case UI_KEY_DOWN:
        nav->selected[d] = (uint8_t)((nav->selected[d] + 1u) % m->count);
        fix_scroll(nav);
        break;
    case UI_KEY_OK:
        activate(nav, it);
        break;
    case UI_KEY_BACK:
        if (nav->depth > 0u) {
            nav->stack[nav->depth] = NULL;
            nav->depth--;
        }
        break;
    case UI_KEY_NONE:
        break;
    }
}

static void draw_title(gfx_t *g, const char *title)
{
    gfx_fill_rect(g, 0, 0, GFX_WIDTH, MENU_TITLE_H, GFX_ON);
    gfx_text(g, 3, 2, title, 1u, GFX_OFF);
}

/* Текст значения справа от пункта; "" — ничего не рисовать. */
static const char *item_value(const menu_nav_t *nav, const menu_item_t *it, bool editing, char *buf)
{
    switch (it->type) {
    case MENU_ITEM_SUBMENU:
    case MENU_ITEM_SCREEN:
        return ">";
    case MENU_ITEM_TOGGLE:
        return nav->tr(*it->flag ? nav->str_on : nav->str_off);
    case MENU_ITEM_CHOICE:
        return nav->tr(it->choices[*it->choice]);
    case MENU_ITEM_VALUE:
        if (editing) {
            buf[0] = '<';
            gfx_itoa(*it->value, buf + 1);
            uint8_t n = 0;
            while (buf[n] != '\0') {
                n++;
            }
            buf[n] = '>';
            buf[n + 1u] = '\0';
            return buf;
        }
        return gfx_itoa(*it->value, buf);
    case MENU_ITEM_ACTION:
        break;
    }
    return "";
}

static void draw_scrollbar(gfx_t *g, uint8_t count, uint8_t scroll)
{
    const int16_t track_y = MENU_ROW_Y0;
    const int16_t track_h = MENU_VISIBLE_ROWS * MENU_ROW_H;
    int16_t thumb_h = (int16_t)(track_h * MENU_VISIBLE_ROWS / count);
    if (thumb_h < 4) {
        thumb_h = 4;
    }
    int16_t max_scroll = (int16_t)(count - MENU_VISIBLE_ROWS);
    int16_t thumb_y = (int16_t)(track_y + (track_h - thumb_h) * scroll / max_scroll);
    for (int16_t y = track_y; y < track_y + track_h; y += 2) {
        gfx_pixel(g, GFX_WIDTH - 2, y, GFX_ON);
    }
    gfx_fill_rect(g, GFX_WIDTH - 3, thumb_y, 3, thumb_h, GFX_ON);
}

void menu_draw(const menu_nav_t *nav, gfx_t *g)
{
    gfx_clear(g);
    draw_title(g, menu_title(nav));

    if (nav->screen != NULL) {
        nav->screen->draw(g);
        return;
    }

    const menu_t *m = menu_current(nav);
    uint8_t d = nav->depth;
    bool scrollbar = m->count > MENU_VISIBLE_ROWS;
    int16_t row_w = scrollbar ? GFX_WIDTH - 5 : GFX_WIDTH;
    int16_t value_right = (int16_t)(row_w - 3);
    char buf[16];

    for (uint8_t row = 0; row < MENU_VISIBLE_ROWS; row++) {
        uint8_t idx = (uint8_t)(nav->scroll[d] + row);
        if (idx >= m->count) {
            break;
        }
        const menu_item_t *it = &m->items[idx];
        bool sel = idx == nav->selected[d];
        int16_t y = (int16_t)(MENU_ROW_Y0 + row * MENU_ROW_H);

        int16_t text_x = 3;
        if (it->icon != NULL) {
            gfx_icon(g, 2, (int16_t)(y + 1), it->icon, GFX_ON);
            text_x = 12;
        }
        gfx_text(g, text_x, (int16_t)(y + 1), nav->tr(it->label), 1u, GFX_ON);

        const char *val = item_value(nav, it, sel && nav->editing, buf);
        if (val[0] != '\0') {
            int16_t vx = (int16_t)(value_right - gfx_text_width(val, 1u));
            gfx_text(g, vx, (int16_t)(y + 1), val, 1u, GFX_ON);
        }
        if (sel) {
            gfx_fill_rect(g, 0, (int16_t)(y - 1), row_w, MENU_ROW_H, GFX_INVERT);
        }
    }

    if (scrollbar) {
        draw_scrollbar(g, m->count, nav->scroll[d]);
    }
}
