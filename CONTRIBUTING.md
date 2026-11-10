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
./scripts/build_examples_sh
```

Build only specific environments:

```sh
./scripts/build_examples_sh ESP32 SAMD21G18A
```

Build only a specific example (all environments):

```sh
./scripts/build_examples_sh MCP3461_Mux_SingleEnded
```

See `scripts/build_examples_sh` for full usage details.

## Running Tests

Run the native unit tests (ArduinoFake + Unity):

```sh
pio test -c platformio_ci.ini -e native_ci
```

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
