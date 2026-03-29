#include "midi_handler.h"
#include "config.h"
#include "mcp4728.h"
#include "gate_driver.h"

// ── Mapping tables (from config.h) ───────────────────────────────────────────

static const uint8_t cv_cc_map[NUM_CV_CHANNELS]      = CV_CC_MAP;
static const uint8_t gate_note_map[NUM_GATE_CHANNELS] = GATE_NOTE_MAP;

// ── State ────────────────────────────────────────────────────────────────────

static uint16_t cv_values[NUM_CV_CHANNELS];  // current 12-bit DAC values
static uint8_t  gate_mask;                   // bitmask of active gates

// ── Public API ───────────────────────────────────────────────────────────────

void midi_handler_init(void) {
    for (int i = 0; i < NUM_CV_CHANNELS; i++) cv_values[i] = 0;
    gate_mask = 0;
}

void midi_handler_process(uint8_t status, uint8_t data1, uint8_t data2) {
    uint8_t msg_type = status & 0xF0;
    uint8_t channel  = status & 0x0F;  // 0-based

    if (channel != (uint8_t)(MIDI_CHANNEL - 1)) return;

    if (msg_type == 0xB0) {
        // ── Control Change: knob → CV ────────────────────────────────────────
        for (int i = 0; i < NUM_CV_CHANNELS; i++) {
            if (data1 != cv_cc_map[i]) continue;

            // Scale 0–127 to 0–4095 with no integer overshoot:
            // 127 * 4095 / 127 = 4095 exactly.
            cv_values[i] = (uint16_t)(((uint32_t)data2 * DAC_MAX_VALUE) / 127u);

            // Update the relevant DAC chip.
            // CV 1–4 → dac0 (channels 0–3), CV 5–8 → dac1 (channels 0–3).
            if (i < 4) {
                mcp4728_fast_write(&dac0, cv_values);
            } else {
                mcp4728_fast_write(&dac1, cv_values + 4);
            }
            break;
        }

    } else if (msg_type == 0x90 && data2 > 0) {
        // ── Note On (velocity > 0): pad → gate high ──────────────────────────
        for (int i = 0; i < NUM_GATE_CHANNELS; i++) {
            if (data1 != gate_note_map[i]) continue;
            gate_mask |= (uint8_t)(1u << i);
            gate_set(i, true);
            break;
        }

    } else if (msg_type == 0x80 || (msg_type == 0x90 && data2 == 0)) {
        // ── Note Off (or Note On velocity 0): pad → gate low ─────────────────
        for (int i = 0; i < NUM_GATE_CHANNELS; i++) {
            if (data1 != gate_note_map[i]) continue;
            gate_mask &= (uint8_t)~(1u << i);
            gate_set(i, false);
            break;
        }
    }
    // All other message types (program change, pitch bend, etc.) ignored in v1.
}
