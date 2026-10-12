/* Мок display.h для тестов приложения: запоминает последнее состояние. */
#include "display_mock.h"

#include <string.h>

#include "../../src/hal/display.h"

display_mock_t display_mock;

void display_init(void)
{
    memset(&display_mock, 0, sizeof display_mock);
    display_mock.initialized = true;
}

void display_flush(const uint8_t *fb)
{
    memcpy(display_mock.ram, fb, sizeof display_mock.ram);
    display_mock.flushes++;
}

void display_set_contrast(uint8_t contrast)
{
    display_mock.contrast = contrast;
}

void display_set_inverted(bool inverted)
{
    display_mock.inverted = inverted;
}

void display_set_power(bool on)
{
    display_mock.powered = on;
}
