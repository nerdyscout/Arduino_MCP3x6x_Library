# Arduino MCP3x6x/R Library [![build](https://gitlab.com/stefanherold/Arduino_MCP3x6x_Library/workflows/build/badge.svg)](https://gitlab.com/stefanherold/Arduino_MCP3x6x_Library/actions/workflows/build.yml)

Supports Microchip 16-bit [MCP3461R/MCP3462R/MCP3464R](https://www.microchip.com/en-us/product/MCP3461r) and 24-bit [MCP3561R/MCP3562R/MCP3564R](https://www.microchip.com/en-us/product/MCP3561r) analog-to-digital converters.

Without the `R` in the name seems to be early hardware versions which are [not recommended for designs](https://www.microchip.com/en-us/product/MCP3464) anymore. As they do have a subset of features this lib should mostly work as well.

## Supported Boards

This project builds with multiple MCU architectures (ESP32, ESP8266, AVR, SAMD, STM32, nRF52, Teensy), but only SAMD21 is personally tested.

## Operating Modes

The library supports two operating modes — both use the same constructor:

```cpp
explicit MCP3x6x(uint8_t pinCS = SS, uint8_t pinMOSI = MOSI, uint8_t pinMISO = MISO,
                  uint8_t pinCLK = SCK, SPIClass *theSPI = &SPI,
                  SPISettings theSPISettings = SPISettings())
```

### Mux Mode (simple)

Behaves like Arduino's `analogRead()`. Select a channel, trigger a conversion, wait, return the value. No interrupts needed.

```cpp
MCP3561 mcp;

void setup() {
  mcp.begin();
}
void loop() {
  int32_t val = mcp.analogRead(MCP3x6x_CH0);
  double v = val * mcp.getReference() / mcp.getMaxValue();
}
```

### Scan Mode (advanced)

The ADC cycles through a list of enabled channels continuously. An IRQ pin fires each time a conversion completes, and `IRQ_handler()` reads and caches the result. `analogRead()` in scan mode returns cached values with no SPI delay.

```cpp
MCP3561 mcp;

void mcp_wrapper() { mcp.IRQ_handler(); }

void setup() {
  mcp.begin();
  mcp.attachIRQ(6, mcp_wrapper);      // wire IRQ pin
  mcp.attachMCLK(9);                   // generate master clock
  mcp.enableScanChannel(MCP3x6x_CH0);   // add channels to scan
  mcp.enableScanChannel(MCP3x6x_CH1);
  mcp.startScan();                      // configure ADC + start cycle
}
void loop() {
  int32_t val0 = mcp.analogRead(MCP3x6x_CH0);  // cached, non-blocking
  int32_t val1 = mcp.analogRead(MCP3x6x_CH1);
}
```

Scan mode also supports the Arduino `Stream` interface for non-blocking polling:

```cpp
if (mcp.available()) {
  int32_t val = mcp.read();
}
```

## Examples

| Example | ADC | Mode | What it shows |
|---------|-----|------|---------------|
| `MCP3461_Mux_SingleEnded` | MCP3461 | Mux | Basic single-ended read |
| `MCP3461_Mux_InternalChannels` | MCP3461 | Mux | TEMP sensor + AVDD voltage |
| `MCP3462_Scan_SingleEnded` | MCP3462 | Scan | Basic scan mode |
| `MCP3462_Scan_Stream` | MCP3462 | Scan | Stream polling (`available()` / `read()`) |
| `MCP3464_Mux_Differential` | MCP3464 | Mux | Differential pair |
| `MCP3464_Scan_Continuous` | MCP3464 | Scan | Continuous conversion |
| `MCP3561_Scan_SingleEnded` | MCP3561 | Scan | Basic scan (24-bit) |
| `MCP3562_Scan_Differential` | MCP3562 | Scan | Differential scan |
| `MCP3562_Mux_ExternalReference` | MCP3562 | Mux | External VREF |
| `MCP3564_Mux_SingleEnded` | MCP3564 | Mux | Basic mux (24-bit) |
| `MCP3564_Mux_GainOSR` | MCP3564 | Mux | Gain + oversampling |

---

see also:
  - https://github.com/fhnw-ise-qcrypt/mcp3564
