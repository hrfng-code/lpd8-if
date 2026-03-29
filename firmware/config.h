// config.h — User-configurable mapping for lpd8-if
// Edit MIDI channel, CC numbers, and note numbers to match your LPD8 preset.

#ifndef CONFIG_H
#define CONFIG_H

// ── MIDI channel (1-based) ───────────────────────────────────────────────────
#define MIDI_CHANNEL        1

// ── Knob CC numbers → CV outputs 1–8 ────────────────────────────────────────
// LPD8 Program 1 defaults: K1–K8 send CC 1–8 on channel 1.
// Adjust to match your LPD8 program preset.
#define CV_CC_MAP           { 1, 2, 3, 4, 5, 6, 7, 8 }

// ── Pad note numbers → Gate outputs 1–8 ─────────────────────────────────────
// LPD8 Program 1 defaults: pads 1–8 send notes 40–43, 48–51.
// Adjust to match your LPD8 program preset.
#define GATE_NOTE_MAP       { 40, 41, 42, 43, 48, 49, 50, 51 }

// ── MCP4728 I2C addresses ────────────────────────────────────────────────────
#define MCP4728_I2C_ADDR    0x60    // both chips, same addr, separate I2C buses

// ── I2C bus speed ────────────────────────────────────────────────────────────
#define I2C_BAUD_HZ         400000  // 400 kHz Fast Mode

// ── Gate GPIO base pin ───────────────────────────────────────────────────────
// Gates use GP8–GP15 (GATE_PIN_BASE + 0 .. + 7)
#define GATE_PIN_BASE       8

// ── LED ──────────────────────────────────────────────────────────────────────
#define LED_PIN             25      // onboard LED: off = no device, on = connected

// ── Derived constants ────────────────────────────────────────────────────────
#define NUM_CV_CHANNELS     8
#define NUM_GATE_CHANNELS   8
#define DAC_MAX_VALUE       4095u   // 12-bit

#endif // CONFIG_H
