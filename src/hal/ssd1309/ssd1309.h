#ifndef SSD1309_H
#define SSD1309_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Команды контроллера SSD1309 (совместимы с SSD1306). */
#define SSD1309_DISPLAY_OFF  0xAEu
#define SSD1309_DISPLAY_ON   0xAFu
#define SSD1309_SET_CONTRAST 0x81u
#define SSD1309_NORMAL       0xA6u
#define SSD1309_INVERSE      0xA7u
#define SSD1309_PAGE_ADDR(p) (uint8_t)(0xB0u | ((p) & 0x07u))
#define SSD1309_COL_LOW(c)   (uint8_t)((c) & 0x0Fu)
#define SSD1309_COL_HIGH(c)  (uint8_t)(0x10u | (((c) >> 4) & 0x0Fu))

/*
 * Порт — то, что предоставляет конкретная плата (STM32 HAL, ESP-IDF, Arduino...).
 * В тестах подменяется моком, который записывает поток байт.
 */
void ssd1309_port_write_cmd(const uint8_t *cmd, size_t len);
void ssd1309_port_write_data(const uint8_t *data, size_t len);
void ssd1309_port_reset(void);
void ssd1309_port_delay_ms(uint32_t ms);

void ssd1309_init(void);
void ssd1309_flush(const uint8_t *fb);
void ssd1309_set_contrast(uint8_t contrast);
void ssd1309_set_inverted(bool inverted);
void ssd1309_set_power(bool on);

#endif /* SSD1309_H */
