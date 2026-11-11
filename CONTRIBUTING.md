# Contributing to Arduino MCP3x6x Library

Thank you for contributing! This guide covers getting started, building, testing, and submitting changes.

## Introduction

The **Arduino MCP3x6x Library** is an Arduino library for Microchip MCP346xR / MCP356xR 16/24-bit SAR analog-to-digital converters over SPI.

Supported devices:
- MCP3461R / MCP3462R / MCP3464R (16-bit)
- MCP3561R / MCP3562R / MCP3564R (24-bit)

## Getting Started

### Prerequisites

- **PlatformIO** (`pio`) for building examples and running tests
- **pre-commit** for the pre-commit hook suite
- **doxygen** for generating API documentation
- **kibot** for PCB fabrication outputs
- **KiCad** (for `kibot` renders)

### Clone

```sh
git clone https://github.com/nerdyscout/Arduino_MCP3x6x_Library.git
cd Arduino_MCP3x6x_Library
```

### Install the pre-commit hook

```sh
pre-commit install
```

## Building Examples

Build every example for every environment:

```sh
./examples/build.sh
```

Build only specific environments:

```sh
./examples/build.sh ESP32 SAMD21G18A
```

Build only a specific example (all environments):

```sh
./examples/build.sh MCP3461_Mux_SingleEnded
```

Build and upload a single example to a board:

```sh
./examples/upload.sh MCP3564_DMA ATMEGA328P

# Or via PlatformIO custom target:
pio run -t upload_MCP3564_DMA -e ATMEGA328P
```

See `examples/build.sh` and `examples/upload.sh` for full usage details.

## Running Tests

### Native mock tests (CI — no hardware)

Run the native unit test suite via ArduinoFake + Unity:

```sh
pio test -e native
```

The native environment compiles and executes the same mock test suite for all
6 MCP variants via `FOR_EACH_ADC` macro expansion, verifying chip-ID validation
and per-variant register behavior.

### Hardware test (real board)

Build and upload the hardware smoke test to a physical board. The default
environment is `ATMEGA328P` (Arduino Nano); select the MCP variant with
`-DTEST_ADC_TYPE=<variant>`:

```sh
pio test -e ATMEGA328P
```

The hardware `setup()` runs `runUnityTests()` which prints PASS/FAIL for every
test to Serial at 115200 baud for oscilloscope correlation.

### Hardware-in-the-loop test (real board + Unity Serial readback)

Run the full HIL test on real hardware using Unity asserts read back via Serial:

```sh
# Run on default ATMEGA328P (uses MCP3564 variant)
pio run -t test_hil -e ATMEGA328P

# Run on any environment with a specific MCP variant
pio run -t test_hil -e ATMEGA328P --build-flags "-DTEST_ADC_TYPE=MCP3564"
```

This builds, uploads, and monitors the hardware test via Serial. `pio test`
parses the Unity output and reports results.

### SPI communication test (real board + decoder.py readback)

Verify real SPI transactions end-to-end using the `test_hil_spi` custom target,
which builds/uploads the hardware test, resets the board, reads back decoded SPI
events from a Rigol oscilloscope via `decoder.py`, and compares decoded MOSI
bytes against the expected table:

```sh
# Run on default ATMEGA328P / MCP3564
pio run -t test_hil_spi -e ATMEGA328P

# Run with verbose decoder output
pio run -t test_hil_spi -e ATMEGA328P --build-flags "-DTEST_ADC_TYPE=MCP3464"

# Scope VISA resource and serial port are auto-detected when exactly one
# candidate exists; otherwise use OSCI_RESOURCE / OSCI_PORT env vars.
```

The target verifies the decoded MOSI byte sequence: `reset()`, `chip-ID read`,
`config0()` through `config3()`, `irq()`, `mux()`, `conversion()`, and
`readADCDATA()`.

Options are controlled via the PlatformIO environment and build flags:
- `-e <env>` — PlatformIO environment (default: `ATMEGA328P`)
- `--build-flags "-DTEST_ADC_TYPE=<variant>"` — MCP variant (default: `MCP3564`)
- `OSCI_RESOURCE` — VISA resource string for the scope (auto-detected if single candidate)
- `OSCI_PORT` — Board serial port for DTR reset (auto-detected if single candidate)

## Pre-Commit Hooks

Run all hooks on all files:

```sh
pre-commit run --all-files
```

Hooks that run:
- `trailing-whitespace`
- `end-of-file-fixer`
- `check-yaml`
- `check-json`
- `clang-format`
- `cpplint`
- `cppcheck`

See `.pre-commit-config.yaml` for the full hook configuration.

## PCB Fabrication

Generate top/bottom board renders and fabrication zips:

```sh
./scripts/run_kibot.sh
```

KiBot config files:
- `pcb/render.kibot.yaml` — board renders
- `pcb/fab.kibot.yaml` — fabrication outputs

Outputs are written next to each `.kicad_pcb` file in `pcb/`.

## Doxygen Docs

Generate API documentation:

```sh
doxygen Doxyfile
```

Docs are written to `docs/`.

## Submitting Changes

1. Fork the repository.
2. Create a feature branch from `develop`.
3. Make your changes.
4. Run pre-commit: `pre-commit run --all-files`
5. Submit a pull request against `develop`.

## Code Style

Follow the existing `cpplint` and `clang-format` conventions enforced by the pre-commit hooks. Match the formatting of surrounding code in `src/`.
