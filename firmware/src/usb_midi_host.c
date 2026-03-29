#include "usb_midi_host.h"
#include "midi_handler.h"
#include "gate_driver.h"
#include "config.h"
#include "tusb.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <string.h>
#include <stdint.h>

// ── State ────────────────────────────────────────────────────────────────────

static uint8_t midi_dev_idx  = UINT8_MAX;   // TinyUSB MIDI device index
static bool    dev_connected = false;

// ── Internal MIDI byte-stream parser ─────────────────────────────────────────
// tuh_midi_stream_read() returns raw MIDI bytes with status bytes present
// (TinyUSB re-inserts them from USB MIDI event packets, so running status
// is already resolved for us). We only handle 3-byte messages here.

static void parse_midi_stream(const uint8_t *buf, uint32_t len) {
    uint32_t i = 0;
    while (i < len) {
        uint8_t b = buf[i];
        if (b & 0x80) {
            // Status byte: expect two data bytes to follow.
            if (i + 2 < len) {
                midi_handler_process(buf[i], buf[i + 1], buf[i + 2]);
                i += 3;
            } else {
                // Incomplete message at buffer edge; discard.
                break;
            }
        } else {
            // Stray data byte without preceding status — skip.
            i++;
        }
    }
}

// ── TinyUSB host callbacks ───────────────────────────────────────────────────
// These are weak symbols in TinyUSB, overridden here.
// In CFG_TUSB_OS = OPT_OS_NONE mode, all callbacks fire synchronously
// inside tuh_task() — NOT from hardware ISR context. It is safe to call
// i2c_write_blocking() (and therefore mcp4728_fast_write) from here.

void tuh_midi_mount_cb(uint8_t dev_addr, uint8_t in_ep, uint8_t out_ep,
                       uint8_t num_cables_rx, uint16_t num_cables_tx) {
    (void)in_ep; (void)out_ep; (void)num_cables_rx; (void)num_cables_tx;

    if (midi_dev_idx == UINT8_MAX) {
        midi_dev_idx  = dev_addr;
        dev_connected = true;
        gpio_put(LED_PIN, 1);
    }
}

void tuh_midi_umount_cb(uint8_t dev_addr, uint8_t instance) {
    (void)instance;

    if (dev_addr == midi_dev_idx) {
        midi_dev_idx  = UINT8_MAX;
        dev_connected = false;
        gpio_put(LED_PIN, 0);

        // Safe disconnect: release all gates.
        // CVs intentionally hold their last value to avoid abrupt pitch jumps.
        gate_set_mask(0x00);
    }
}

void tuh_midi_rx_cb(uint8_t dev_addr, uint32_t num_packets) {
    (void)num_packets;

    if (dev_addr != midi_dev_idx) return;

    uint8_t  cable;
    uint8_t  rx_buf[64];
    uint32_t n;

    // Drain all available MIDI bytes from TinyUSB's internal buffer.
    while ((n = tuh_midi_stream_read(dev_addr, &cable, rx_buf, sizeof(rx_buf))) > 0) {
        parse_midi_stream(rx_buf, n);
    }
}

// ── Public API ───────────────────────────────────────────────────────────────

void usb_midi_host_init(void) {
    midi_dev_idx  = UINT8_MAX;
    dev_connected = false;
}

bool usb_midi_host_connected(void) {
    return dev_connected;
}
