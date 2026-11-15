# Testing

## Tiers

| Tier | Directory | Requires | Scope |
|---|---|---|---|
| Mocked | `test/test_mock/` | none (native, `env:mocked`) | Fully mockable unit tests (ArduinoFake + Unity). No real SPI bus or MCP hardware. Runs in CI on push/PR to `main` and `develop`. |
| HIL | `test/test_hil/` | real MCP3x6x hardware on target board | Functional tests against real hardware (init/config/status/conversion). |
| HIL + SPI | `test/test_hil_spi/` | real hardware + oscilloscope (Rigol DS1000Z) | On-bus SPI communication verification of fast commands, decoded with sigrok-cli and gated against MCP3x6x datasheet values. |

Tests that require real MCP3x6x hardware belong in `test_hil`; tests that require actual SPI bus/scope verification belong in `test_hil_spi`. `test_mock` must remain fully mockable.

## Mocked (default)

```sh
pio test -e mocked
# or
pio test            # default_envs = mocked
```

Runs the Unity + ArduinoFake suite in `test/test_mock/` across MCP variants.

## HIL (real hardware)

```sh
pio run -t test_hil -e ATMEGA328P
```

Builds, uploads, and runs the Unity HIL suite in `test/test_hil/` (select variant with `-DTEST_ADC_TYPE=<variant>` as needed).

## HIL + SPI (real hardware + oscilloscope)

Host-paced verification: firmware steps one fast command per trigger, scope captures CS/SCLK/MOSI/MISO, waveforms are exported as VCD and decoded with `sigrok-cli` SPI PD, and decoded MOSI/MISO are compared to MCP3x6x datasheet fast-command values.

```sh
# build + upload + verify
pio run -t test_hil_spi -e hil_spi

# verify only (authoritative gate, firmware already flashed)
python3 test/test_hil_spi/verify_cmd.py
```

**Datasheet gate (5 fast commands):** reset `0x78/0x17`, conversion `0x68/0x17`, standby `0x6C/0x17`, shutdown `0x70/0x17`, full_shutdown `0x74/0x17` (MOSI/MISO).

**Config/env:** `test/hil_spi_config.json` or `HIL_SPI_CONFIG` (`resource`, `board_port`, `channels`, `trigger`, `timebase`, `acquire`, `volts_per_div`, `threshold_volts`). Env overrides: `OSCI_RESOURCE`, `BOARD_PORT`, `HIL_SPI_RESET=1`, `HIL_SPI_KEEP_ARTIFACTS=1`, `VERBOSE=1`.

**Dependencies:** `pyvisa`, `pyserial`, Rigol USBTMC access, `sigrok-cli` (libsigrokdecode). Local protocol decoders in `scripts/decoders/` are used via `SIGROKDECODE_DIR` when present.

Notes: hardware/scope tiers require a connected bench and are not run in CI. See [test/test_hil_spi/README.md](test_hil_spi/README.md) for capture/decode details.
