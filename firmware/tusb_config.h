// tusb_config.h — TinyUSB host-mode configuration for lpd8-if
// This file is #included by TinyUSB internally; it must be reachable via the
// compiler's include path (project root is added in CMakeLists.txt).

#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

// ── MCU ──────────────────────────────────────────────────────────────────────
// pico-sdk sets CFG_TUSB_MCU to OPT_MCU_RP2040 for both RP2040 and RP2350.
// This is intentional: the USB controller is register-compatible.
#ifndef CFG_TUSB_MCU
  #error "CFG_TUSB_MCU must be set by the build system (pico-sdk provides it)"
#endif

// ── Task model ───────────────────────────────────────────────────────────────
// pico-sdk sets this via -D on the command line; guard to avoid redefinition.
#ifndef CFG_TUSB_OS
  #define CFG_TUSB_OS             OPT_OS_NONE   // bare-metal cooperative polling
#endif

// ── Debug ────────────────────────────────────────────────────────────────────
#define CFG_TUSB_DEBUG          0               // set to 2 for verbose USB logs on UART

// ── Memory ───────────────────────────────────────────────────────────────────
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN      __attribute__((aligned(4)))

// ── Host stack ───────────────────────────────────────────────────────────────
#define CFG_TUH_ENABLED         1

// Use the native RP2350 USB controller (port 0 = physical USB connector).
// Declare port 0 as host mode — required by TinyUSB 0.16+.
#define BOARD_TUH_RHPORT        0
#define CFG_TUSB_RHPORT0_MODE   OPT_MODE_HOST

// ── Device limits ────────────────────────────────────────────────────────────
#define CFG_TUH_DEVICE_MAX      1               // only the LPD8, no hub
#define CFG_TUH_HUB             0
#define CFG_TUH_ENUMERATION_BUFSIZE  256

// ── Class drivers ────────────────────────────────────────────────────────────
// MIDI host is provided by the rppicomidi/usb_midi_host external library
// via the usbh_app_driver_get_cb() extension point — NOT via CFG_TUH_MIDI.
// CFG_TUH_MIDI is intentionally not defined here.
#define CFG_TUH_CDC             0
#define CFG_TUH_HID             0
#define CFG_TUH_MSC             0
#define CFG_TUH_VENDOR          0

// ── MIDI buffers (consumed by usb_midi_host library) ─────────────────────────
#define CFG_TUH_MIDI_RX_BUFSIZE     64
#define CFG_TUH_MIDI_TX_BUFSIZE     64
#define CFG_TUH_MIDI_EP_BUFSIZE     64

#endif // TUSB_CONFIG_H
