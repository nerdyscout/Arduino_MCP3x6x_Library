# Arduino MCP3x6x/R Library [![build](https://github.com/nerdyscout/Arduino_MCP3x6x_Library/workflows/build/badge.svg)](https://github.com/nerdyscout/Arduino_MCP3x6x_Library/actions/workflows/build.yml)

Supports Microchip 16bit [MCP3461R/MCP3462R/MCP3464R](https://www.microchip.com/en-us/product/MCP3461r) and 24bit [MCP3561R/MCP3562R/MCP3564R](https://www.microchip.com/en-us/product/MCP3561r) Analog Digital Converters.

## Supported Boards

This project is set up to be build with different MCUs, but I am not able the test each of them.
Personally I use the SAMD21, so this should be the most stable code.

### Introduction

This lib does not (yet) support all features of these ADCs, but should help to get you started.
Depending on which constructor is called the ADC will be configuerd in two different mode:

**Mux Mode**
```
MCP3x6x(const uint8_t pinCS = SS,SPIClass *theSPI = &SPI,
const uint8_t pinMOSI = MOSI, const uint8_t pinMISO = MISO, const uint8_t pinCLK = SCK)
```

**Scan Mode**
```
MCP3x6x(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS, SPIClass *theSPI = &SPI,
const uint8_t pinMOSI = MOSI, const uint8_t pinMISO = MISO, const uint8_t pinCLK = SCK)
```
So scan mode requieres two additional pins, pinIQR for the conversion interrupt flag and pinMCLK (which is not implemented at all).

Both constructors fall back to default definitions which are common in the Arduino world. So if no pins is explicit specified the default SPI pinout will be used. Therefor it boils down to something simple as `MCP3561 mcp(2, 3, 10);` or even `MCP3561 mcp(10);`.

Please take a look at the examples how to configure these different modes.

Further documentation is available [here](http://nerdyscout.github.io/Arduino_MCP3x6x_Library).

---

see also:
  - https://github.com/fhnw-ise-qcrypt/mcp3564
