#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#include <stdint.h>

// Initialise internal state (zero CV values and gate mask).
void midi_handler_init(void);

// Process one decoded MIDI message.
// status: MIDI status byte (0x80–0xFF, running status already resolved)
// data1, data2: the two data bytes
void midi_handler_process(uint8_t status, uint8_t data1, uint8_t data2);

#endif // MIDI_HANDLER_H
