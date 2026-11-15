# HIL SPI verification (`test_hil_spi`)

Host-paced, on-bus SPI verification for MCP3x6x fast commands on real hardware (Nano + Rigol DS1000Z) using **sigrok-cli/libsigrokdecode** as the decode/gating path.

## How it works

1. Resets the board (DTR) and steps one fast command per capture via serial (`hil_spi` firmware, one command per `n`).
2. Arms a single CS-falling trigger, captures stopped analog waveforms (CS/SCLK/MOSI/MISO), thresholds to digital (1.6 V), exports a VCD.
3. Decodes with `sigrok-cli` SPI PD (`cpol=0 cpha=0 bitorder=msb-first wordsize=8 cs_polarity=active-low`, channels CS=CH1 SCLK=CH3 MOSI=CH4 MISO=CH2).
4. Gates pass/fail by comparing decoded bytes against **MCP3x6x datasheet** fast-command values (and observed MISO status):

| label | MOSI (datasheet) | MISO (status) |
|---|---:|---:|
| reset | 0x78 | 0x17 |
| conversion | 0x68 | 0x17 |
| standby | 0x6C | 0x17 |
| shutdown | 0x70 | 0x17 |
| full_shutdown | 0x74 | 0x17 |

## Usage

```sh
# build + upload + verify (custom target)
pio run -e hil_spi -t test_hil_spi

# or verify only (firmware already flashed)
python3 test/test_hil_spi/verify_cmd.py
```

## Configuration

- `test/hil_spi_config.json` (or `HIL_SPI_CONFIG`): `channels`, `trigger`, `timebase`, `acquire`, `volts_per_div`, `threshold_volts`, `board_port`, `resource`.
- Env: `OSCI_RESOURCE`, `BOARD_PORT`, `HIL_SPI_KEEP_ARTIFACTS=1` (keep `logs/hil_spi/step_*.vcd`).

## Dependencies

- `pyvisa`, `pyserial`, Rigol USBTMC/`usbtmc` access, `sigrok-cli` (libsigrokdecode). Local protocol decoders in `decoders/` are used if present via `SIGROKDECODE_DIR`.

## Notes

- The scope VISA session is reopened/reconfigured per capture for stability on Rigol USBTMC.
- Default scope settings: 2 V/div, 0 V offset, DC 1×, 2 µs/div +8 µs offset, `ACQ NORM MDEPTH 12000000`, edge trig CH1 NEG 2.0 V.
