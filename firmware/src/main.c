#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "tusb.h"
#include "config.h"
#include "mcp4728.h"
#include "gate_driver.h"
#include "midi_handler.h"
#include "usb_midi_host.h"

int main(void) {
    // ── SDK and stdio init ────────────────────────────────────────────────────
    // stdio_init_all() enables UART0 on GP0/GP1 at 115200 baud for debug.
    // Disable pico_enable_stdio_uart in CMakeLists.txt for the final panel build.
    stdio_init_all();

    // ── LED ───────────────────────────────────────────────────────────────────
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    // ── Gate outputs ──────────────────────────────────────────────────────────
    gate_init();

    // ── DAC init ──────────────────────────────────────────────────────────────
    // If the MCP4728 boards are not present or not responding, the LED stays on
    // as a hardware fault indicator and the firmware still runs (gates work).
    bool dac_ok = mcp4728_init_all();
    if (!dac_ok) {
        gpio_put(LED_PIN, 1);   // solid LED = DAC init failed
    }

    // ── MIDI handler ──────────────────────────────────────────────────────────
    midi_handler_init();

    // ── TinyUSB host ──────────────────────────────────────────────────────────
    tusb_init();
    usb_midi_host_init();

    // ── Main loop ─────────────────────────────────────────────────────────────
    // All MIDI processing (DAC writes, gate updates) happens inside
    // tuh_midi_rx_cb(), which TinyUSB calls synchronously from tuh_task().
    while (true) {
        tuh_task();
    }

    return 0;
}
