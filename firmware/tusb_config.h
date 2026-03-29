// tusb_config.h — TinyUSB host-mode configuration for lpd8-if
// This file is #included by TinyUSB internally; it must be reachable via the
// compiler's include path (project root is added in CMakeLists.txt).

#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

// ── MCU ──────────────────────────────────────────────────────────────────────
// pico-sdk sets CFG_TUSB_MCU to OPT_MCU_RP2040 for both RP2040 and RP2350.
// This is intentional: the USB controller is register-compatible and the
// native host driver compiles and runs correctly on the Pico 2 (RP2350).
#ifndef CFG_TUSB_MCU
  #error "CFG_TUSB_MCU must be set by the build system (pico-sdk provides it)"
#endif

// ── Task model ───────────────────────────────────────────────────────────────
#define CFG_TUSB_OS             OPT_OS_NONE     // bare-metal cooperative polling

// ── Debug ────────────────────────────────────────────────────────────────────
#define CFG_TUSB_DEBUG          0               // set to 2 for verbose USB logs on UART

// ── Memory ───────────────────────────────────────────────────────────────────
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN      __attribute__((aligned(4)))

// ── Host stack ───────────────────────────────────────────────────────────────
#define CFG_TUH_ENABLED         1

// Use the native RP2350 USB controller (port 0 = physical USB connector).
// PIO-USB is not needed and not used.
// If you ever reroute USB to a custom connector via PIO, comment out
// BOARD_TUH_RHPORT 0 and instead set CFG_TUH_RPI_PIO_USB 1 + RHPORT 1.
#define BOARD_TUH_RHPORT        0

// ── Device limits ────────────────────────────────────────────────────────────
#define CFG_TUH_DEVICE_MAX      1               // only the LPD8, no hub
#define CFG_TUH_HUB             0
#define CFG_TUH_ENUMERATION_BUFSIZE  256

// ── Class drivers ────────────────────────────────────────────────────────────
#define CFG_TUH_MIDI            1               // MIDI host class only
#define CFG_TUH_CDC             0
#define CFG_TUH_HID             0
#define CFG_TUH_MSC             0
#define CFG_TUH_VENDOR          0

// ── MIDI buffers ─────────────────────────────────────────────────────────────
#define CFG_TUH_MIDI_RX_BUFSIZE     64
#define CFG_TUH_MIDI_TX_BUFSIZE     64
#define CFG_TUH_MIDI_EP_BUFSIZE     64
#define CFG_TUH_MIDI_STREAM_API     1           // enable tuh_midi_stream_read()

#endif // TUSB_CONFIG_H
