#include "mcp4728.h"
#include "config.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// ── Global DAC instances ─────────────────────────────────────────────────────

mcp4728_t dac0 = { .i2c = i2c0, .addr = MCP4728_I2C_ADDR, .shadow = {0,0,0,0} };
mcp4728_t dac1 = { .i2c = i2c1, .addr = MCP4728_I2C_ADDR, .shadow = {0,0,0,0} };

// ── Internal helpers ─────────────────────────────────────────────────────────

// Send the Multi-Write init sequence to one chip.
// Sets all 4 channels: VREF=internal 2.048V, Gx=1×, PD=normal, value=0.
//
// Multi-Write command byte format (per channel):
//   0_1_0_0_0_CH1_CH0_UDAC   (0x40 | channel<<1 | 0)
// Config byte:
//   VREF_PD1_PD0_Gx_D11..D8  → 0x80 (VREF=1, PD=00, Gx=0, data=0)
// Data byte:
//   D7..D0                   → 0x00
//
// Resulting 12-byte buffer (UDAC=0 on last channel triggers latching all at once):
//   { 0x40,0x80,0x00,  0x42,0x80,0x00,  0x44,0x80,0x00,  0x47,0x80,0x00 }
//                                                          ^^ ch3 + UDAC=1
static bool mcp4728_init_chip(const mcp4728_t *dac) {
    uint8_t buf[12] = {
        0x40, 0x80, 0x00,   // channel 0, UDAC=0
        0x42, 0x80, 0x00,   // channel 1, UDAC=0
        0x44, 0x80, 0x00,   // channel 2, UDAC=0
        0x47, 0x80, 0x00,   // channel 3, UDAC=1 → latch all outputs simultaneously
    };
    return i2c_write_blocking(dac->i2c, dac->addr, buf, sizeof(buf), false) == sizeof(buf);
}

// ── Public API ───────────────────────────────────────────────────────────────

bool mcp4728_init_all(void) {
    // I2C0 — GP4 (SDA), GP5 (SCL)
    i2c_init(i2c0, I2C_BAUD_HZ);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);

    // I2C1 — GP6 (SDA), GP7 (SCL)
    i2c_init(i2c1, I2C_BAUD_HZ);
    gpio_set_function(6, GPIO_FUNC_I2C);
    gpio_set_function(7, GPIO_FUNC_I2C);
    gpio_pull_up(6);
    gpio_pull_up(7);

    bool ok = true;
    ok &= mcp4728_init_chip(&dac0);
    ok &= mcp4728_init_chip(&dac1);
    return ok;
}

bool mcp4728_fast_write(mcp4728_t *dac, const uint16_t values[4]) {
    // Fast Write: 8 bytes, 2 per channel, no command prefix.
    // Byte 0: 0 0 PD1 PD0 D11 D10 D09 D08  (PD=00 for normal operation)
    // Byte 1: D07 D06 D05 D04 D03 D02 D01 D00
    uint8_t buf[8];
    for (int ch = 0; ch < 4; ch++) {
        uint16_t v = values[ch] > DAC_MAX_VALUE ? DAC_MAX_VALUE : values[ch];
        buf[ch * 2]     = (uint8_t)((v >> 8) & 0x0F);
        buf[ch * 2 + 1] = (uint8_t)(v & 0xFF);
        dac->shadow[ch] = v;
    }
    return i2c_write_blocking(dac->i2c, dac->addr, buf, sizeof(buf), false) == sizeof(buf);
}

bool mcp4728_set_channel(mcp4728_t *dac, uint8_t channel, uint16_t value) {
    if (channel >= 4) return false;
    dac->shadow[channel] = value > DAC_MAX_VALUE ? DAC_MAX_VALUE : value;
    return mcp4728_fast_write(dac, dac->shadow);
}
