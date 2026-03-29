#ifndef GATE_DRIVER_H
#define GATE_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

// Initialise GP8–GP15 as outputs, all LOW.
void gate_init(void);

// Set one gate channel (0–7) high or low.
void gate_set(uint8_t channel, bool state);

// Set all 8 gates atomically from a bitmask (bit 0 = channel 0 = GP8).
void gate_set_mask(uint8_t mask);

#endif // GATE_DRIVER_H
