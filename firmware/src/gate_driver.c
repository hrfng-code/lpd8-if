#include "gate_driver.h"
#include "config.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

void gate_init(void) {
    for (int i = 0; i < NUM_GATE_CHANNELS; i++) {
        uint pin = GATE_PIN_BASE + i;
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, 0);
    }
}

void gate_set(uint8_t channel, bool state) {
    if (channel >= NUM_GATE_CHANNELS) return;
    gpio_put(GATE_PIN_BASE + channel, state ? 1 : 0);
}

void gate_set_mask(uint8_t mask) {
    // GP8–GP15 occupy bits 8–15 of the GPIO bank.
    gpio_put_masked(0x0000FF00u, (uint32_t)mask << GATE_PIN_BASE);
}
