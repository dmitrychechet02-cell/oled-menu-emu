/* АВТОГЕНЕРАЦИЯ: tools/gen_font.py. Не редактировать вручную. */
#ifndef ICONS_H
#define ICONS_H

#include <stdint.h>

/* Иконка высотой 8 px: width столбцов, бит 0 — верхний пиксель. */
typedef struct {
    uint8_t width;
    const uint8_t *cols;
} icon_t;

extern const icon_t icon_wifi;
extern const icon_t icon_thermo;
extern const icon_t icon_gear;
extern const icon_t icon_info;
extern const icon_t icon_chart;
extern const icon_t icon_font;
extern const icon_t icon_drop;
extern const icon_t icon_check;

#endif /* ICONS_H */
