/*
 * Реализация display.h для эмулятора: «RAM дисплея» — отдельный буфер,
 * который обновляется только по display_flush(), как у настоящего SSD1309.
 * JS читает его из памяти WebAssembly через emu_display_ram().
 */
#include "display_web.h"

#include "../display.h"

static uint8_t ram[DISPLAY_WEB_RAM_SIZE];
static uint8_t contrast = 0x7Fu;
static bool inverted;
static bool powered;
static uint32_t flush_count;

void display_init(void)
{
    for (uint16_t i = 0; i < DISPLAY_WEB_RAM_SIZE; i++) {
        ram[i] = 0u;
    }
    contrast = 0x7Fu;
    inverted = false;
    powered = true;
    flush_count = 0u;
}

void display_flush(const uint8_t *fb)
{
    for (uint16_t i = 0; i < DISPLAY_WEB_RAM_SIZE; i++) {
        ram[i] = fb[i];
    }
    flush_count++;
}

void display_set_contrast(uint8_t c)
{
    contrast = c;
}

void display_set_inverted(bool inv)
{
    inverted = inv;
}

void display_set_power(bool on)
{
    powered = on;
}

const uint8_t *display_web_ram(void)
{
    return ram;
}

uint8_t display_web_contrast(void)
{
    return contrast;
}

bool display_web_inverted(void)
{
    return inverted;
}

bool display_web_powered(void)
{
    return powered;
}

uint32_t display_web_flush_count(void)
{
    return flush_count;
}
