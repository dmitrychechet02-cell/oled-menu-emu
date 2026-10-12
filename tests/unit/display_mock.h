#ifndef DISPLAY_MOCK_H
#define DISPLAY_MOCK_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool initialized;
    bool powered;
    bool inverted;
    uint8_t contrast;
    uint32_t flushes;
    uint8_t ram[1024];
} display_mock_t;

extern display_mock_t display_mock;

#endif /* DISPLAY_MOCK_H */
