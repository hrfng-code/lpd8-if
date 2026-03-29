#ifndef MIDI_APP_H
#define MIDI_APP_H

#include <stdbool.h>

// Call once after tusb_init() to reset internal state.
void midi_app_init(void);

// Returns true when an LPD8 (or any USB MIDI device) is connected.
bool midi_app_connected(void);

#endif // MIDI_APP_H
