#include "ssd1309.h"

#define SSD1309_WIDTH 128u
#define SSD1309_PAGES 8u

/* Последовательность инициализации для модуля 2.42" 128x64 (SSD1309). */
/* clang-format off */
static const uint8_t init_seq[] = {
    SSD1309_DISPLAY_OFF,
    0xD5u, 0x80u,               /* частота тактирования */
    0xA8u, 0x3Fu,               /* multiplex ratio: 64 строки */
    0xD3u, 0x00u,               /* смещение дисплея */
    0x40u,                      /* start line = 0 */
    0x20u, 0x02u,               /* режим адресации: page */
    0xA1u,                      /* segment remap */
    0xC8u,                      /* COM scan: снизу вверх */
    0xDAu, 0x12u,               /* COM pins */
    SSD1309_SET_CONTRAST, 0x7Fu,
    0xD9u, 0xF1u,               /* pre-charge */
    0xDBu, 0x34u,               /* VCOMH */
    0xA4u,                      /* вывод из RAM */
    SSD1309_NORMAL,
    SSD1309_DISPLAY_ON,
};
/* clang-format on */

void ssd1309_init(void)
{
    ssd1309_port_reset();
    ssd1309_port_delay_ms(10u);
    ssd1309_port_write_cmd(init_seq, sizeof(init_seq));
}

void ssd1309_flush(const uint8_t *fb)
{
    for (uint8_t page = 0; page < SSD1309_PAGES; page++) {
        const uint8_t addr[3] = {SSD1309_PAGE_ADDR(page), SSD1309_COL_LOW(0u),
                                 SSD1309_COL_HIGH(0u)};
        ssd1309_port_write_cmd(addr, sizeof(addr));
        ssd1309_port_write_data(&fb[page * SSD1309_WIDTH], SSD1309_WIDTH);
    }
}

void ssd1309_set_contrast(uint8_t contrast)
{
    const uint8_t cmd[2] = {SSD1309_SET_CONTRAST, contrast};
    ssd1309_port_write_cmd(cmd, sizeof(cmd));
}

void ssd1309_set_inverted(bool inverted)
{
    const uint8_t cmd = inverted ? SSD1309_INVERSE : SSD1309_NORMAL;
    ssd1309_port_write_cmd(&cmd, 1u);
}

void ssd1309_set_power(bool on)
{
    const uint8_t cmd = on ? SSD1309_DISPLAY_ON : SSD1309_DISPLAY_OFF;
    ssd1309_port_write_cmd(&cmd, 1u);
}
