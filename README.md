# Arduino MCP3x6x Library

Arduino library for Microchip MCP346xR / MCP356xR 16/24-bit SAR analog-to-digital converters over SPI.

Supports Microchip MCP3461R/MCP3462R/MCP3464R (16-bit) and MCP3561R/MCP3562R/MCP3564R (24-bit) ADC families.

## Features

- Template-based device support — one code base for all six MCP3x6xR variants
- SPI register interface with fast commands (CONVERSION, STANDBY, SHUTDOWN, FULL_SHUTDOWN, RESET, SREAD, IREAD, IWRITE)
- Single-ended and differential channel selection (CH0–CH7, internal channels: OFFSET, VCM, AVDD, TEMP, DIFFx)
- Config0–Config3 register access (gain, OSR, data rate, conversion mode, etc.)
- Continuous conversion and scan modes
- Data-ready (DR) status and data-available polling
- IRQ and MCLK pin attachment
- Offset / gain calibration and temperature conversion helpers
- Low-power modes: standby, shutdown, full shutdown
- PlatformIO-mocked unit tests (ArduinoFake + Unity), hardware smoke tests, and hardware-in-the-loop SPI verification

## Supported devices

| Device | Resolution |
|--------|------------|
| MCP3461R / MCP3462R / MCP3464R | 16-bit |
| MCP3561R / MCP3562R / MCP3564R | 24-bit |

## Quick start

### Installation

**Arduino Library Manager:** search for **MCP3x6xR**.

**Manual / git:**

```sh
git clone https://github.com/nerdyscout/Arduino_MCP3x6x_Library.git \
  ~/Arduino/libraries/MCP3x6xR
```

**PlatformIO:** add to `platformio.ini`:

```ini
lib_deps = nerdyscout/MCP3x6xR
```

### Basic usage

```cpp
#include "Arduino_MCP3x6x_Library.hpp"

MCP3461 mcp;  // or MCP3564, MCP3464, etc.

void setup() {
  mcp.begin();
}

void loop() {
  int32_t raw = mcp.analogRead(MCP3x6x_CH0);
  double volt = raw * mcp.getReference() / mcp.getMaxValue();
  delay(100);
}
```

See `src/Arduino_MCP3x6x_Library.hpp` for the full class API, or build the Doxygen reference (`doxygen Doxyfile`).

## Building and testing

This project uses PlatformIO.

### Build examples

Build all examples for all environments:

```sh
./examples/build.sh
```

Build specific environments:

```sh
./examples/build.sh ESP32 ATMEGA328P SAMD21G18A
```

Build only one example:

```sh
./examples/build.sh MCP3564_DMA
```

Upload a single example to your board:

```sh
./examples/upload.sh MCP3564_DMA ATMEGA328P
```

See `examples/README.md` for full `build.sh` / `upload.sh` usage details.

### Run mocked unit tests (no hardware required)

```sh
pio test -e mocked
```

The mocked environment compiles and executes the same mock test suite for all six MCP variants via `FOR_EACH_ADC` macro expansion, verifying chip-ID validation and per-variant register behavior.

### Hardware smoke test (real board)

Build and upload the hardware smoke test to a physical board (default: `ATMEGA328P` / Arduino Nano):

```sh
pio test -e ATMEGA328P
```

Select the MCP variant with `-DTEST_ADC_TYPE=<variant>`:

```sh
pio test -e ATMEGA328P --build-flags "-DTEST_ADC_TYPE=MCP3564"
```

### Hardware-in-the-loop test (real board + Unity Serial readback)

```sh
# Run on default ATMEGA328P / MCP3564
pio run -t test_hil -e ATMEGA328P

# Run with a specific MCP variant
pio run -t test_hil -e ATMEGA328P --build-flags "-DTEST_ADC_TYPE=MCP3464"
```

### SPI communication test (real board + oscilloscope readback)

Verify real SPI transactions end-to-end using the `test_hil_spi` custom target:

```sh
pio run -t test_hil_spi -e ATMEGA328P
pio run -t test_hil_spi -e ATMEGA328P --build-flags "-DTEST_ADC_TYPE=MCP3464"
```

See `test/README.md` for the full testing guide (mocked, hardware, HIL, and SPI HIL).

## PCB fabrication

The `pcb/` directory contains KiCad projects and KiBot configs for three reference boards:

| Board | Top render | Bottom render |
|-------|------------|---------------|
| Adafruit Feather | [Feather_top.svg](pcb/Adafruit/Feather_top.svg) | [Feather_bottom.svg](pcb/Adafruit/Feather_bottom.svg) |
| Arduino Uno | [Uno_top.svg](pcb/Arduino/Uno_top.svg) | [Uno_bottom.svg](pcb/Arduino/Uno_bottom.svg) |
| Sparkfun ProMicro | [ProMicro_top.svg](pcb/Sparkfun/ProMicro_top.svg) | [ProMicro_bottom.svg](pcb/Sparkfun/ProMicro_bottom.svg) |

Generate top/bottom board renders and fabrication zips:

```sh
./pcb/kibot.sh
```

- `pcb/render.kibot.yaml` — board renders
- `pcb/fab.kibot.yaml` — fabrication outputs

Outputs are written next to each `.kicad_pcb` file in `pcb/`. See `pcb/README.md` for details.

## Documentation

- `pcb/README.md` — PCB fabrication (KiBot renders and fabrication zips)
- `examples/README.md` — Examples build and upload
- `test/README.md` — Native, hardware, and HIL testing
- `docs/` — Doxygen API reference (`doxygen Doxyfile`), also hosted at https://nerdyscout.github.io/Arduino_MCP3x6x_Library/

## Contributing

See `pcb/README.md`, `examples/README.md`, and `test/README.md` for domain-specific guides. Submit changes via pull request against `develop`.

## License

MIT. See `LICENSES` for details.
