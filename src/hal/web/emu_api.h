/* API эмулятора, экспортируемое из WebAssembly в web/emulator.js. */
#ifndef EMU_API_H
#define EMU_API_H

#include <stdint.h>

void emu_init(void);
void emu_key(int32_t key);
void emu_tick(uint32_t elapsed_ms);
const uint8_t *emu_display_ram(void);
uint32_t emu_contrast(void);
uint32_t emu_inverted(void);
uint32_t emu_powered(void);
uint32_t emu_flush_count(void);
uint32_t emu_screen_id(void);
uint32_t emu_menu_depth(void);
uint32_t emu_selected(void);
uint32_t emu_editing(void);
const char *emu_title(void);
int32_t emu_brightness(void);
uint32_t emu_lang(void);

#endif /* EMU_API_H */
