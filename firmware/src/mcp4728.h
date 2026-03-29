#ifndef MCP4728_H
#define MCP4728_H

#include "hardware/i2c.h"
#include <stdint.h>
#include <stdbool.h>

// Handle for one MCP4728 chip.
typedef struct {
    i2c_inst_t *i2c;
    uint8_t     addr;           // 7-bit I2C address (0x60)
    uint16_t    shadow[4];      // last-written values per channel (0–4095)
} mcp4728_t;

// Global DAC instances (defined in mcp4728.c).
// dac0: CV 1–4 on I2C0 (GP4/GP5)
// dac1: CV 5–8 on I2C1 (GP6/GP7)
extern mcp4728_t dac0;
extern mcp4728_t dac1;

// Initialise both I2C buses and both DAC chips.
// Sets VREF = internal 2.048V, gain = 1×, power-down = normal, all outputs = 0V.
// Returns true if both chips acknowledged.
bool mcp4728_init_all(void);

// Write all 4 channels of one chip using the Fast Write command (8 bytes).
// values[0..3]: 12-bit unsigned (0–4095).
// Returns true on success.
bool mcp4728_fast_write(mcp4728_t *dac, const uint16_t values[4]);

// Write a single channel without disturbing the others (uses shadow register).
bool mcp4728_set_channel(mcp4728_t *dac, uint8_t channel, uint16_t value);

#endif // MCP4728_H
