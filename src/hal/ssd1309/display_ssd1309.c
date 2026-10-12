/* Привязка display.h к драйверу SSD1309 — собирается только для настоящей платы. */
#include "../display.h"
#include "ssd1309.h"

void display_init(void)
{
    ssd1309_init();
}

void display_flush(const uint8_t *fb)
{
    ssd1309_flush(fb);
}

void display_set_contrast(uint8_t contrast)
{
    ssd1309_set_contrast(contrast);
}

void display_set_inverted(bool inverted)
{
    ssd1309_set_inverted(inverted);
}

void display_set_power(bool on)
{
    ssd1309_set_power(on);
}
