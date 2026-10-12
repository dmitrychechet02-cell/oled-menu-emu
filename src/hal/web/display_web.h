#ifndef DISPLAY_WEB_H
#define DISPLAY_WEB_H

#include <stdbool.h>
#include <stdint.h>

#define DISPLAY_WEB_RAM_SIZE 1024u

const uint8_t *display_web_ram(void);
uint8_t display_web_contrast(void);
bool display_web_inverted(void);
bool display_web_powered(void);
uint32_t display_web_flush_count(void);

#endif /* DISPLAY_WEB_H */
