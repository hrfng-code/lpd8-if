// Suppress the deprecation warning from usb_midi_host.h — we are intentionally
// using this library because TinyUSB 0.18.0 (bundled with pico-sdk) does not yet
// include the native MIDI host driver. Switch to the native driver when pico-sdk
// updates to a TinyUSB release that includes midi_host.c.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcpp"
#include "usb_midi_host.h"
#pragma GCC diagnostic pop

#include "midi_app.h"
#include "midi_handler.h"
#include "gate_driver.h"
#include "config.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdint.h>

// ── State ────────────────────────────────────────────────────────────────────

static uint8_t midi_dev_addr = 0;   // 0 = no device connected
static bool    dev_connected = false;

// ── Internal MIDI byte-stream parser ─────────────────────────────────────────
// tuh_midi_stream_read() returns raw MIDI bytes with status bytes present.
// We only process 3-byte messages (Note On/Off, CC).

static void parse_midi_stream(const uint8_t *buf, uint32_t len) {
    uint32_t i = 0;
    while (i < len) {
        uint8_t b = buf[i];
        if (b & 0x80) {
            // Status byte: need two data bytes.
            if (i + 2 < len) {
                midi_handler_process(buf[i], buf[i + 1], buf[i + 2]);
                i += 3;
            } else {
                break;  // incomplete at buffer edge
            }
        } else {
            i++;        // stray data byte, skip
        }
    }
}

// ── TinyUSB MIDI host callbacks ───────────────────────────────────────────────
// These override the TU_ATTR_WEAK stubs in usb_midi_host.h.
// In CFG_TUSB_OS = OPT_OS_NONE all callbacks fire synchronously inside
// tuh_task() — safe to call i2c_write_blocking() from here.

void tuh_midi_mount_cb(uint8_t dev_addr, uint8_t in_ep, uint8_t out_ep,
                       uint8_t num_cables_rx, uint16_t num_cables_tx) {
    (void)in_ep; (void)out_ep; (void)num_cables_rx; (void)num_cables_tx;
    if (!dev_connected) {
        midi_dev_addr = dev_addr;
        dev_connected = true;
        gpio_put(LED_PIN, 1);
    }
}

void tuh_midi_umount_cb(uint8_t dev_addr, uint8_t instance) {
    (void)instance;
    if (dev_addr == midi_dev_addr) {
        midi_dev_addr = 0;
        dev_connected = false;
        gpio_put(LED_PIN, 0);
        // Release all gates on disconnect; CVs hold last value intentionally.
        gate_set_mask(0x00);
    }
}

void tuh_midi_rx_cb(uint8_t dev_addr, uint32_t num_packets) {
    (void)num_packets;
    if (dev_addr != midi_dev_addr) return;

    uint8_t  cable;
    uint8_t  rx_buf[64];
    uint32_t n;
    while ((n = tuh_midi_stream_read(dev_addr, &cable, rx_buf, sizeof(rx_buf))) > 0) {
        parse_midi_stream(rx_buf, n);
    }
}

// ── Public API ───────────────────────────────────────────────────────────────

void midi_app_init(void) {
    midi_dev_addr = 0;
    dev_connected = false;
}

bool midi_app_connected(void) {
    return dev_connected;
}
