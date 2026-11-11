# Arduino MCP3x6x Library

Arduino library for Microchip MCP346xR / MCP356xR 16/24-bit SAR analog-to-digital converters over SPI.

## Supported devices

- MCP3461R / MCP3462R / MCP3464R (16-bit)
- MCP3561R / MCP3562R / MCP3564R (24-bit)

## Getting Started

### Install PlatformIO

Install [PlatformIO](https://platformio.org/), then clone the library:

```sh
git clone https://github.com/nerdyscout/Arduino_MCP3x6x_Library.git
cd Arduino_MCP3x6x_Library
```

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

### Run tests

Run the native unit tests (ArduinoFake + Unity, no hardware required):

```sh
pio test -e native
```

### Hardware testing

Build and upload the hardware smoke test to a physical board:

```sh
pio test -e ATMEGA328P
```

Run hardware-in-the-loop SPI communication verification:

```sh
pio run -t test_hil_spi -e ATMEGA328P
```

## Documentation

- `pcb/README.md` — PCB fabrication (KiBot)
- `examples/README.md` — Examples build and upload
- `test/README.md` — Native, hardware, and HIL testing
- `docs/` — Doxygen API reference (`doxygen Doxyfile`)

## Contributing

See `pcb/README.md`, `examples/README.md`, and `test/README.md` for domain-specific guides. Submit changes via pull request against `develop`.
