# Testing

## Layout

- `test/test_mock/` — mocked unit tests (ArduinoFake + Unity). Only expectations that can be fully simulated.
- `test/test_hil/` — hardware-in-the-loop Unity tests (real board).
- `test/test_hil_spi/` — SPI communication verification (real board + oscilloscope config).

## Mocked tests (default, no hardware)

```sh
pio test
# or
pio test -e mocked -f test_mock
```

Runs from `test/test_mock/` across MCP variants via `FOR_EACH_ADC`.

## HIL (real board)

```sh
pio run -t test_hil -e ATMEGA328P
```

Builds/uploads/runs the Unity HIL suite in `test/test_hil/` on the selected hardware env.

## HIL + SPI (real board + oscilloscope)

```sh
# Full build+upload+verify (includes pin-probe for channel mapping)
pio run -t test_hil_spi -e ATMEGA328P

# Fast verify (skips pin-probe wait, for an already-flashed/running board)
pio test -e hil_spi
# or: python3 test/test_hil_spi/verify_cmd.py
```

Behavior:
1. Pin-probe (unless `-DSKIP_PIN_PROBE`, e.g. `env:hil_spi`): toggles SS/SCK/MOSI/MISO with serial markers for scope channel mapping/verification.
2. Emits all MCP3x6x fast commands via the library instance (`mcp.reset()`, `mcp.conversion()`, `mcp.standby()`, `mcp.shutdown()`, `mcp.full_shutdown()`) and verifies the MOSI sequence from the board’s serial JSON (testcase-emitted, single source of truth).

Reusable scope settings: `test/hil_spi_config.json` (probe, volts/div, offset, trigger level, timebase/acquire, channel mapping). Env vars: `BOARD_PORT`, `OSCI_RESOURCE`, `HIL_SPI_CONFIG`, `SKIP_PIN_PROBE`, `HIL_SPI_RESET`. VISA resources are closed after use.
