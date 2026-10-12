/*
 * Проверка драйвера SSD1309 без железа: мок порта записывает поток байт,
 * который ушёл бы по SPI/I2C, и мы сверяем его с даташитом.
 */
#include <string.h>

#include "../../src/hal/ssd1309/ssd1309.h"
#include "tt.h"

#define LOG_MAX 4096

typedef struct {
    uint8_t is_data; /* 0 — команда, 1 — данные */
    uint8_t byte;
} logged_t;

static logged_t log_buf[LOG_MAX];
static size_t log_len;
static int resets;
static uint32_t delayed_ms;

static void log_bytes(uint8_t is_data, const uint8_t *b, size_t n)
{
    for (size_t i = 0; i < n && log_len < LOG_MAX; i++) {
        log_buf[log_len].is_data = is_data;
        log_buf[log_len].byte = b[i];
        log_len++;
    }
}

void ssd1309_port_write_cmd(const uint8_t *cmd, size_t len)
{
    log_bytes(0, cmd, len);
}

void ssd1309_port_write_data(const uint8_t *data, size_t len)
{
    log_bytes(1, data, len);
}

void ssd1309_port_reset(void)
{
    resets++;
}

void ssd1309_port_delay_ms(uint32_t ms)
{
    delayed_ms += ms;
}

static void reset_log(void)
{
    log_len = 0;
    resets = 0;
    delayed_ms = 0;
}

TT_TEST(ssd1309_init_resets_and_turns_display_on)
{
    reset_log();
    ssd1309_init();
    TT_ASSERT_EQ(resets, 1);
    TT_ASSERT(delayed_ms >= 1);
    TT_ASSERT(log_len > 10);
    TT_ASSERT_EQ(log_buf[0].byte, SSD1309_DISPLAY_OFF);
    TT_ASSERT_EQ(log_buf[log_len - 1].byte, SSD1309_DISPLAY_ON);
    for (size_t i = 0; i < log_len; i++) {
        TT_ASSERT_EQ(log_buf[i].is_data, 0);
    }
}

TT_TEST(ssd1309_flush_sends_8_pages_of_128_bytes)
{
    static uint8_t fb[1024];
    for (int i = 0; i < 1024; i++) {
        fb[i] = (uint8_t)(i * 7);
    }
    reset_log();
    ssd1309_flush(fb);
    TT_ASSERT_EQ(log_len, 8 * (3 + 128));

    size_t pos = 0;
    for (int page = 0; page < 8; page++) {
        TT_ASSERT_EQ(log_buf[pos].is_data, 0);
        TT_ASSERT_EQ(log_buf[pos].byte, 0xB0 + page);
        TT_ASSERT_EQ(log_buf[pos + 1].byte, 0x00);
        TT_ASSERT_EQ(log_buf[pos + 2].byte, 0x10);
        pos += 3;
        for (int col = 0; col < 128; col++) {
            TT_ASSERT_EQ(log_buf[pos].is_data, 1);
            TT_ASSERT_EQ(log_buf[pos].byte, fb[page * 128 + col]);
            pos++;
        }
    }
}

TT_TEST(ssd1309_contrast_invert_power_commands)
{
    reset_log();
    ssd1309_set_contrast(0x42);
    ssd1309_set_inverted(true);
    ssd1309_set_inverted(false);
    ssd1309_set_power(false);
    static const uint8_t expected[] = {0x81, 0x42, 0xA7, 0xA6, 0xAE};
    TT_ASSERT_EQ(log_len, sizeof expected);
    for (size_t i = 0; i < sizeof expected; i++) {
        TT_ASSERT_EQ(log_buf[i].byte, expected[i]);
    }
}
