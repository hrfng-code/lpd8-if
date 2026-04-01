# CLAUDE.md — AI Assistant Guide for lpd8-if

## Project Overview

**lpd8-if** is embedded firmware for a Raspberry Pi Pico 2 that acts as a USB MIDI host, converting MIDI input from an Akai LPD8 controller into analog CV and gate signals for modular synthesizers:

- 8 CV outputs (0–5 V) via two Microchip MCP4728 12-bit DACs over I2C
- 8 gate outputs (0/3.3 V) via GPIO pins GP8–GP15
- MIDI CC messages (knobs) → CV; MIDI Note On/Off (pads) → gates

The codebase is ~380 lines of C11, cleanly modularized, targeting the RP2350 (Pico 2) with no OS.

---

## Repository Structure

```
lpd8-if/
├── build                      # Bash build wrapper script (executable)
├── requirements.txt           # Python deps for manual testing (mido, pyserial)
├── .python-version            # Python 3.13
├── .gitmodules                # usb_midi_host submodule declaration
└── firmware/
    ├── CMakeLists.txt         # CMake build config
    ├── config.h               # User-configurable MIDI/pin mappings
    ├── tusb_config.h          # TinyUSB host-mode settings
    ├── pico_sdk_import.cmake  # Pico SDK import helper
    ├── lib/
    │   └── usb_midi_host/     # Git submodule: rppicomidi USB MIDI host driver
    └── src/
        ├── main.c             # Entry point: init + main event loop
        ├── midi_app.c/h       # TinyUSB MIDI callbacks; raw byte-stream parser
        ├── midi_handler.c/h   # CC→CV and note→gate mapping logic
        ├── mcp4728.c/h        # MCP4728 I2C DAC driver
        └── gate_driver.c/h    # GPIO gate output driver
```

---

## Architecture

The firmware is layered:

```
TinyUSB (USB host stack)
    └── usb_midi_host driver  [firmware/lib/usb_midi_host/]
            └── midi_app      [src/midi_app.c]   – mounts/unmounts device, reads raw bytes
                    └── midi_handler  [src/midi_handler.c]  – parses MIDI messages, drives outputs
                            ├── mcp4728   [src/mcp4728.c]   – I2C DAC for CV outputs
                            └── gate_driver [src/gate_driver.c] – GPIO for gate outputs
```

**main.c** initialises all subsystems and runs the bare-metal event loop (`tuh_task()` + `midi_app_task()`).

### Key Hardware Assignments

| Signal | Peripheral | Pins |
|--------|-----------|------|
| DAC 1 (CV 1–4) | I2C0, MCP4728 | GP4 (SDA), GP5 (SCL) |
| DAC 2 (CV 5–8) | I2C1, MCP4728 | GP6 (SDA), GP7 (SCL) |
| Gate 1–8 | GPIO | GP8–GP15 |
| Debug UART | UART0 | GP0 (TX), GP1 (RX) |
| Onboard LED | GPIO | GP25 |

### State Model

- **CV values** held in `midi_handler.c` static array; written to DAC shadow registers
- **Gate state** held as 8-bit bitmask; all gates released on USB disconnect
- **Device address** stored in `midi_app.c` static; cleared on unmount
- LED: on = device connected; off = no device; solid on at boot = DAC init fault

---

## Configuration

All user-tunable parameters live in `firmware/config.h`:

```c
#define MIDI_CHANNEL      1       // LPD8 default channel
#define CV_CC_MAP         {1, 2, 3, 4, 5, 6, 7, 8}   // K1–K8 CC numbers
#define GATE_NOTE_MAP     {40, 41, 42, 43, 48, 49, 50, 51}  // pad note numbers
#define MCP4728_I2C_ADDR  0x60    // both DACs (on different I2C buses)
#define I2C_BAUD_HZ       400000  // Fast Mode
#define GATE_PIN_BASE     8       // GP8–GP15
#define LED_PIN           25
```

TinyUSB settings are in `firmware/tusb_config.h`; do not modify unless changing USB behaviour.

---

## Build System

### Prerequisites

- ARM GCC cross-compiler (`arm-none-eabi-gcc`)
- CMake ≥ 3.13
- Pico SDK checked out at `../pico-sdk` relative to the repo root (or set `PICO_SDK_PATH`)
- Git submodule initialised: `git submodule update --init --recursive`

### Building

```bash
# From repo root
./build          # configure + build (Release, target: pico2)
./build clean    # wipe firmware/build/ and rebuild
```

Output files in `firmware/build/`:
- `lpd8_if.uf2` — flash via USB bootloader (drag-and-drop)
- `lpd8_if.elf` — for debugging with OpenOCD/GDB
- `lpd8_if.map`, `lpd8_if.dis` — symbol map and disassembly

### CMake Details

- Standard: C11, C++17
- Board: `pico2` (`-DPICO_BOARD=pico2`)
- Build type: `Release`
- `compile_commands.json` generated (`-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`)
- Compile flags: `-Wall -Wextra` (unused-parameter suppressed for TinyUSB callbacks)
- UART stdio on GP0/GP1; USB CDC stdio disabled

---

## Development Workflow

### Flashing

1. Hold BOOTSEL button, plug Pico 2 into USB — appears as USB mass storage
2. Copy `firmware/build/lpd8_if.uf2` to the drive
3. Pico reboots automatically

### Debugging

Connect a USB-to-UART adapter to GP0 (TX) / GP1 (RX), baud 115200:

```bash
# With pyserial
python -m serial.tools.miniterm /dev/ttyUSB0 115200

# Or screen
screen /dev/ttyUSB0 115200
```

### Testing (Manual)

No automated test framework. Functional testing:

1. Flash firmware, connect LPD8 via USB OTG adapter
2. LED lights when LPD8 is detected
3. Turn knobs → measure CV on DAC output pins (0–5 V range)
4. Press pads → measure gate on GP8–GP15 (0 or 3.3 V)
5. Monitor UART for debug messages

Python utilities (install from `requirements.txt` into a venv):

```bash
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

- `mido` — send/receive test MIDI messages
- `pyserial` — monitor UART debug output

---

## Code Conventions

### Naming

| Pattern | Usage |
|---------|-------|
| `module_verb_noun()` | Public functions (e.g., `mcp4728_set_cv()`, `gate_driver_set_all()`) |
| `UPPER_SNAKE_CASE` | Constants and macros |
| `lower_snake_case` | Variables, parameters |
| `type_t` | Typedef'd structs (e.g., `mcp4728_t`) |
| `tuh_` prefix | TinyUSB host callbacks (required by library) |

### File Organisation

- Each hardware peripheral gets a `.c`/`.h` pair
- Public API in `.h`; implementation details (`static` variables/functions) in `.c`
- `config.h` is the single place for hardware/MIDI customisation — no magic numbers in `.c` files
- `main.c` only calls `_init()` functions and runs the event loop

### Style

- 4-space indentation
- K&R brace style (opening brace on same line)
- `// ──` horizontal rules to separate logical sections within a file
- Line comments preferred; block comments for complex algorithms
- `bool` return type for init functions (true = success)

### Error Handling

- Initialisation failures return `false`; caller checks and may set LED fault indicator
- No dynamic allocation; no exceptions; no RTOS
- All state in `static` module-level variables; no globals exported

---

## External Dependencies

### usb_midi_host (Git submodule)

- Location: `firmware/lib/usb_midi_host/`
- Source: https://github.com/rppicomidi/usb_midi_host
- Provides MIDI class driver for TinyUSB host mode (not yet in TinyUSB 0.18.0 natively)
- Must be initialised: `git submodule update --init --recursive`
- Do not edit files inside this directory; update via `git submodule update --remote`

### Pico SDK

- Expected at `../pico-sdk` (sibling directory to repo root)
- Override with `PICO_SDK_PATH` env var or `-DPICO_SDK_PATH=...` CMake arg
- Bundles TinyUSB 0.18.0

---

## Important Constraints

- **Bare-metal, no RTOS**: all processing happens in the main loop or TinyUSB callbacks; keep callbacks fast and non-blocking
- **Single USB device**: `CFG_TUH_DEVICE_MAX=1`; no USB hub support
- **I2C addresses**: both MCP4728 DACs share address `0x60` but are on separate I2C buses (I2C0 and I2C1)
- **CV range**: DAC output is 0–4095 (12-bit) mapped to 0–5 V via external resistor network or op-amp; MIDI CC 0–127 scaled to fill full range
- **Gate voltage**: GPIO high = 3.3 V; external level shifting required for 5 V gate signals
- **No USB CDC**: `stdio` goes to UART only; `printf` debug output on GP0/GP1

---

## Branch and Commit Conventions

**Always commit directly to `main`.** Do not create feature branches or session branches. Every commit goes straight to `main` unless the user explicitly asks for a branch.

Commit message style:
- `feat(scope): description` — new functionality
- `fix(scope): description` — bug fixes
- `chore: description` — tooling, build, non-functional changes

Scope examples: `firmware`, `build`, `hardware`
