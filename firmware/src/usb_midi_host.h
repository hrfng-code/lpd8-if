#ifndef USB_MIDI_HOST_H
#define USB_MIDI_HOST_H

#include <stdbool.h>

// Call once after tusb_init() to reset internal state.
void usb_midi_host_init(void);

// Returns true when an LPD8 (or any USB MIDI device) is connected.
bool usb_midi_host_connected(void);

#endif // USB_MIDI_HOST_H
