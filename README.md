# Arduino MCP3x6x/R Library [![build](https://gitlab.com/stefanherold/Arduino_MCP3x6x_Library/workflows/build/badge.svg)](https://gitlab.com/stefanherold/Arduino_MCP3x6x_Library/actions/workflows/build.yml)

Supports Microchip 16bit [MCP3461R/MCP3462R/MCP3464R](https://www.microchip.com/en-us/product/MCP3461r) and 24bit [MCP3561R/MCP3562R/MCP3564R](https://www.microchip.com/en-us/product/MCP3561r) Analog Digital Converters.

Without the `R` in the name seems to be early hardware versions which are [not recommended for designs](https://www.microchip.com/en-us/product/MCP3464) anymore. As they do have a subset of features this lib should mostly work as well.

## Supported Boards

This project is set up to be build with different MCUs, but I am not able the test each of them.
Personally I use the SAMD21, so this should be the most stable code.

### Introduction

This lib does not (yet) support all features of these ADCs, but should help to get you started.
Depending on which constructor is called the ADC will be configuerd in two different mode:

**Mux Mode** (SPI pins with defaults)
```
MCP3x6x(uint8_t pinCS = SS, uint8_t pinMOSI = MOSI, uint8_t pinMISO = MISO,
        uint8_t pinCLK = SCK, SPIClass *theSPI = &SPI,
        SPISettings theSPISettings = SPISettings())
```

**Scan Mode** (all pins required)
```
MCP3x6x(uint8_t pinIRQ, uint8_t pinMCLK, uint8_t pinCS, uint8_t pinMOSI,
        uint8_t pinMISO, uint8_t pinCLK, SPIClass *theSPI = &SPI,
        SPISettings theSPISettings = SPISettings())
```
So scan mode requires two additional pins: pinIRQ for the conversion interrupt flag and pinMCLK for the master clock (generated via `tone()`; attach with `attachMCLK()`).

Both constructors fall back to default definitions which are common in the Arduino world. So if no pins is explicit specified the default SPI pinout will be used. Therefor it boils down to something simple as `MCP3561 mcp(2, 3, 10);` or even `MCP3561 mcp(10);`.

Please take a look at the examples how to configure these different modes.

---

see also:
  - https://github.com/fhnw-ise-qcrypt/mcp3564
