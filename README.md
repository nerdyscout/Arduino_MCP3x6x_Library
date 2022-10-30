# Arduino MCP3x6x Library [![build](https://github.com/nerdyscout/Arduino_MCP3x6x_Library/workflows/build/badge.svg)](https://github.com/nerdyscout/Arduino_MCP3x6x_Library/actions/workflows/build.yml)

Supports Microchip 16bit [MCP3461/MCP3462/MCP3464/](https://www.microchip.com/en-us/product/MCP3461r) and 24bit [MCP3561/MCP3562/MCP3564/](https://www.microchip.com/en-us/product/MCP3561r) Analog Digital Converters.

## Supported Boards

This project is set up to be build with different MCUs, but I am not able the test each of them.
Personally I use the SAMD most, so this should be the most stable code.

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

Both constructors fall back to default definitions which are common in the Arduino world. So if nothing else is explicit specified the default SPI pinout will be used. But to run in Scan Mode it is mandatory to give one interruptcapable pin as well as one PWM pin (altough MCLK functions are not implemented).

For initialization of the ADC you must run once:
```
  if (!mcp.begin()) {
    // failed to initialize
    while (1)
      ;
  }
```
After that you could simple do ```int32_t adcdata0 = mcp.analogRead(0);``` to get a value of the single ended channel0.

An initialization like this:
```
  if (!mcp.begin(2)) {
    // failed to initialize
    while (1)
      ;
  }
  attachInterrupt(digitalPinToInterrupt(8), mcp_handler, FALLING);
```
will enable reading differential channel0, in that case ```int32_t adcdata0 = mcp.analogRead(8);``` will give the result.
This also requires an interupt handler ```void mcp_wrapper() { mcp.IRQ_handler(); }```.

***Setting up and reading (differential) channels will change in further version!***
