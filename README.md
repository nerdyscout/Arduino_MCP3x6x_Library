# Arduino MCP3x6x Library

Arduino library for Microchip MCP346xR/MCP356xR 16/24-bit SAR ADCs over SPI (MCP3461R/3462R/3464R, MCP3561R/3562R/3564R).

## Features

- Template-based support for all six MCP3x6xR variants
- Fast commands: `CONVERSION`, `STANDBY`, `SHUTDOWN`, `FULL_SHUTDOWN`, `RESET`, `SREAD`, `IREAD`, `IWRITE`
- Single-ended/differential channels (`CH0`–`CH7`, `OFFSET`, `VCM`, `AVDD`, `TEMP`, `DIFFx`)
- Config/register access, continuous and scan modes, data-ready polling
- IRQ/MCLK helpers, calibration and temperature helpers, low-power modes
- Tests: mocked (ArduinoFake/Unity), HIL on real MCP hardware, and on-bus SPI verification with scope + sigrok-cli

## Quick start

### Installation

- **Arduino Library Manager:** search for **MCP3x6xR**
- **Manual:** clone into your Arduino libraries folder
  ```sh
  git clone https://github.com/nerdyscout/Arduino_MCP3x6x_Library.git ~/Arduino/libraries/MCP3x6xR
  ```
- **PlatformIO:**
  ```ini
  lib_deps = nerdyscout/MCP3x6xR
  ```

### Basic usage

```cpp
#include "Arduino_MCP3x6x_Library.hpp"

MCP3564 mcp;

void setup() {
  mcp.begin();
}

void loop() {
  int32_t raw = mcp.analogRead(MCP3x6x_CH0);
  double volt = raw * mcp.getReference() / mcp.getMaxValue();
  delay(100);
}
```

API reference: `src/Arduino_MCP3x6x_Library.hpp` or `doxygen Doxyfile` (https://nerdyscout.github.io/Arduino_MCP3x6x_Library/).

## Build examples

```sh
./examples/build.sh                  # all examples, all envs
./examples/build.sh ESP32 ATMEGA328P # selected envs
./examples/upload.sh MCP3564_DMA ATMEGA328P
```

See [examples/README.md](examples/README.md).

## Testing

Three tiers (details in [test/README.md](test/README.md)):

| Tier | Location | Requires | Purpose |
|---|---|---|---|
| Mocked | `test/test_mock/` | none (native) | Fully mockable unit tests (ArduinoFake + Unity), CI-gated |
| HIL | `test/test_hil/` | real MCP3x6x hardware | Functional tests on target board |
| HIL + SPI | `test/test_hil_spi/` | real hardware + oscilloscope | On-bus SPI fast-command verification (sigrok-cli decode vs MCP3x6x datasheet) |

```sh
pio test -e mocked                                # mocked (default)
pio run -t test_hil -e ATMEGA328P                 # HIL (real MCP hardware)
pio run -t test_hil_spi -e hil_spi                # build/upload + SPI bus verification
python3 test/test_hil_spi/verify_cmd.py           # SPI verify only (authoritative gate)
```

## PCB fabrication

Reference boards and KiBot configs in [`pcb/`](pcb/) (`pcb/render.kibot.yaml`, `pcb/fab.kibot.yaml`). Generate renders/fabs with `./pcb/kibot.sh`. See [pcb/README.md](pcb/README.md).

## Documentation

- [test/README.md](test/README.md) — test tiers, config, env vars
- [examples/README.md](examples/README.md) — build/upload examples
- [pcb/README.md](pcb/README.md) — PCB fabrication
- [test/test_hil_spi/README.md](test/test_hil_spi/README.md) — scope/sigrok SPI verification details

## Contributing

Submit changes via pull request against `develop`.

## License

MIT. See `LICENSES`.
