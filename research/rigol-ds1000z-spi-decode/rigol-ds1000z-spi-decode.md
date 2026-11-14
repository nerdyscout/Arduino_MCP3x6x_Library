# Research findings: Rigol DS1000Z SPI decoder settings, `:ETABle<n>:DATA?` format, and USBTMC limits

- Ticket: [#34 — Research: Rigol DS1000Z SPI decoder settings, ETABle format, and USBTMC limits](https://github.com/nerdyscout/Arduino_MCP3x6x_Library/issues/34)
- Branch: `research/rigol-ds1000z-spi-decode`
- Local findings file: `research/rigol-ds1000z-spi-decode/rigol-ds1000z-spi-decode.md`

## Question

Is `decoder.py`'s `:ETABle1:DATA?` readback a reliable gate for the MCP3x6x SPI tier, and what does it
take to make it reliable?

Sub-questions:

1. Documented DS1000Z SPI decoder settings for 4 MHz MSB-first MODE0 capture (timebase, sample rate,
   acquisition mode, decoder thresholds).
2. The `:ETABle<n>:DATA?` response format and how `decoder.py` parses it.
3. Known USBTMC limitations (binary transfer, timeouts, `:SYST:ERR?` behaviour) and how `decoder.py`
   works around them.
4. Whether MISO decode is trustworthy enough to compare, or MOSI-only is the sane first cut.
5. Documented causes of empty/partial event tables and recommended mitigations.

## TL;DR

**Partly — the readback plumbing exists and is mostly correct, but it is not yet a trustworthy
autonomous "gate" for the MCP3x6x SPI tier.**

- The DS1000Z does expose instrument-side SPI decoding plus an event table, and the SCPI surface used
  by `decoder.py` (`:DECoder<n>:*`, `:ETABle<n>:*`) is real and documented.
- The `:ETABle<n>:DATA?` reply is a Rigol **TMC block** (`#N<NNNNNNN…>` header + payload), which
  `decoder.py._parse_tmc_block` strips — but the **event payload layout itself is not specified in the
  manual**, so `decoder.py` only returns raw bytes/hex/base64 and explicitly flags the result as
  `"uncalibrated — :ETABle<n>:DATA? binary layout not specified in the instrument manual"`.
- Reliability work is needed on the scope side (decoder config, timebase/sample rate, thresholds,
  acquisition) and on the transport side (USBTMC deviations, read timeouts/chunking).

## 1. Documented DS1000Z SPI decoder settings (4 MHz MSB-first MODE0 capture)

Sources: RIGOL DS1000Z Programming Guide (PGA19110-1110, SW 00.04.03.SP2), DS1000Z User Guide Ch. 8
(Protocol Decoding), DS1000Z datasheet.

- Decoder type: `:DECoder<n>:MODE <PARallel|UART|SPI|IIC>`; default `PARallel`. Per the Programming
  Guide, **RS232/UART, I2C and SPI decoding are options** and the command is only available when the
  corresponding option is installed.
- Enable: `:DECoder<n>:DISPlay <ON|OFF>`. The event table (`:ETABle<n>:DISP`) is only valid when the
  decoder is turned on.
- SPI parameter set (Programming Guide, Ch. 2): `:DECoder<n>:SPI:CLK`, `:SPI:MISO`, `:SPI:MOSI`,
  `:SPI:CS`, `:SPI:SELect`, `:SPI:MODE`, `:SPI:TIMeout`, `:SPI:POLarity`, `:SPI:EDGE`, `:SPI:ENDian`,
  `:SPI:WIDTh`.

MODE0 mapping onto Rigol nomenclature:

| SPI MODE0 concept | Rigol setting | Command / value |
|---|---|---|
| CPOL = 0 (clock idle high) | `POSitive` (high level = 1) | `:DECoder<n>:SPI:POLarity POSitive` |
| CPHA = 0 (sample on rising edge) | `RISE` | `:DECoder<n>:SPI:EDGE RISE` |
| MSB first | MSB | `:DECoder<n>:SPI:ENDian MSB` |
| 8 data bits | 8 | `:DECoder<n>:SPI:WIDTh 8` |

These are exactly the defaults/flags exercised by `decoder.py` (`--polarity pos`, `--edge rise`,
`--endian msb`, `--width 8`) and by the HIL script's scope configuration block. Frame synchronization is
via `:DECoder<n>:SPI:MODE CS|TIMeout` (default `TIMeout`); in CS mode one sets
`:DECoder<n>:SPI:CS <src>` and `:DECoder<n>:SPI:SELect NCS|CS` (NCS = low level valid, CS = high level
valid). In TIMeout mode the timeout must be greater than the maximum clock pulse width and lower than
the idle time between frames (default 1.00 µs; DS1000Z-E: 100 ns–1 s).

- Sample rate: `:DECoder<n>:CONFig:SRATe?` (digital sample rate). Data source `Trace` (default) ties
  digital SR to the horizontal timebase; `Memory` ties it to sample rate × memory depth. Real time
  digital SR ≈ 1 GSa/s (8-channel) / 500 MSa/s (16-channel); analog ≈ 1 GSa/s single-channel. Horizontal
  timebase range 5 ns/div – 50 s/div; "screen sample rate = 100 / horizontal timebase". For a 4 MHz,
  8-bit SPI stream (2 µs/frame) the timebase/sample rate must resolve that (deep memory / fast SR). On-
  screen memory is ~1200 points; deep memory up to 24 Mpts (`:ACQ:MDEPth`, `:WAV:STAR`/`:WAV:STOP`).
- Acquisition: `:ACQuire:TYPE` (`NORMal|AVERages|PEAK|HRESolution`, default `NORMal`; averages 2–1024),
  `:ACQuire:MDEPth`, `:ACQuire:AVERages`.
- Thresholds: `:DECoder<n>:THREshold:CHANnel<n> <thre>` valid range
  (−4×VerticalScale − VerticalOffset, 4×VerticalScale + VerticalOffset); auto by default
  (`:DECoder<n>:THREshold:AUTO`). User logic thresholds (digital channels) e.g. TTL 1.4 V, CMOS
  presets, user ±15 V (10 mV steps). `decoder.py` uses `--threshold` (default 1.5 V) for software
  slicing and `--set-instrument-threshold` to write AUTO OFF + per-channel thresholds.

## 2. `:ETABle<n>:DATA?` response format and `decoder.py` parsing

- Subcommands: `:ETABle<n>:DISP`, `:ETABle<n>:FORMat`, `:ETABle<n>:VIEW`, `:ETABle<n>:COLumn`,
  `:ETABle<n>:ROW`, `:ETABle<n>:SORT`, `:ETABle<n>:DATA?`.
- Semantics: `DISP` only valid when decoder on; `ROW?` returns current row, **0 if the event table is
  empty**; `VIEW` ∈ {PACKage, DETail, PAYLoad}; `COLumn` ∈ {DATA, TX, RX, MISO, MOSI} (SPI: MISO|MOSI);
  `SORT` ∈ {ASCend, DESCend}.
- `DATA?` return: event table data in TMC format — `#9000000098` is the TMC data description header
  followed by the event table data; format is `#900000dddd`, `dddd` = number of bytes of valid data
  following the description header (example: `#9000000098` → **98 bytes** of valid data; content
  following = event table data).
- General Rigol TMC block format (waveform/image): `#NXXXXXX` (`#` denoter, N ≤ 9 = width of the length
  field, figures following = byte count). Example `#9001152054` → N = 9, length = 001152054 = 1152054
  bytes. Waveform: header is 11 bytes (`#`, N, N digits, data, terminator 0x0A); effective waveform
  data starts at byte 12 (1-based) / index 11 and ends before the terminator (MATLAB: `data(12:len-1)`).

- `decoder.py` side: `_parse_tmc_block()` strips the header (`ndigits = int(raw[1])`,
  `length_str = raw[2:2+ndigits]`, payload = `raw[2+ndigits : 2+ndigits+length]`).
  `readback_mode()` forces `:DECoder<n>:DISPlay ON` and `:ETABle<n>:DISPlay ON`, optionally sets
  format/view/column/sort, writes `:ETABle<n>:DATA?`, reads via `inst.read_raw()`, returns
  `{bytes, hex, base64, note: "uncalibrated — :ETABle<n>:DATA? binary layout not specified in the instrument manual"}`.

- **Implication for the gate:** the event-payload layout (timestamp, line number, data) is not
  documented → `decoder.py` cannot yet autonomously interpret events or compare them to expected MOSI
  bytes. It can only hand back raw payload bytes.

## 3. USBTMC limitations and how `decoder.py` works around them

Known Rigol-over-USBTMC quirks (pyvisa-py / PyVISA issue trackers; Rigol deviation from USBTMC):

- Rigol does not send the USBTMC "short packet ends transfer" termination the way the spec (and
  pyvisa-py PR #449) assumed → read regressions/hangs (pyvisa-py #463, DS1102Z-E).
- First data packet length can be off by the IEEE header size (10 bytes) (pyvisa-py #472, DS1102E);
  long-memory reads time out; zero-length packet errors; `timeout = 10000` ms often required or it
  hangs (pyvisa-py #472, #414).
- DS1104Z Plus via pyvisa-py: `:WAV:DATA?` returned only 9 points / truncated (pyvisa-py #414);
  ASCII vs binary handling matters.
- ASCII `:WAV:DATA?` also prefixed with TMC-ish header; naive `query()` truncates at ~9 numbers
  (pyvisa-py #586) → use binary/`query_binary_values`/`read_raw`.

`decoder.py` / `scpi_base.py` workarounds:

- `chunk_size = 64` for USBTMC binary reads (pyvisa-py default ~20 kB times out); `query_binary_values(..., datatype="B", chunk_size=64)`.
- `read_raw()` for `:ETABle<n>:DATA?`.
- Default timeout 8 s (HIL script: 10 s scope / 30 s during read), `ERRQ_DRAIN_CAP = 12` drain of
  `SYST:ERR?` (stop when line starts with `"0,"`).
- Error format `"<msg>,<content>"`, e.g. `-113,"Undefined header; command cannot be found"`.
- Verified on DS1104Z FW 00.04.05.SP1; note `:SYSTem:SETup` stream write is a silent no-op on that
  firmware → snapshot + readback-verify path used instead.

## 4. MISO vs MOSI — sanity of the comparison

- Scope decoder supports MISO|MOSI columns; both cannot be OFF; for SPI the event-table column can be
  MISO or MOSI.
- HIL design (`test/hil_spi.py`): compares **decoded MOSI bytes against the expected MCU→ADC command
  table**; scope channels: CS=CHAN1, SCLK=CHAN3, MISO=CHAN2, MOSI=CHAN4 (Nano: CS=10, SCK=13,
  MISO=12, MOSI=11).
- **MOSI-only is the sane first cut**: MOSI is master-driven (deterministic, directly comparable to the
  expected SPI command bytes); MISO reflects the ADC's response and depends on ADC data/behavior. The
  "SPI tier" gate (did the scope decode the master's transactions correctly) is best served by MOSI.
- Caveat: instrument-side SPI decode itself can be lossy — reports of DS1000Z capturing only a few
  bytes at a time / subsampling; cross-check with `decoder.py capture` (software decode → VCD/CSV) when
  instrument decode looks wrong.

## 5. Empty/partial event tables — likely causes and mitigations

Documented / strongly implied:

- Event table requires decoder ON; `:ETABle<n>:DISP` only valid when `:DECoder<n>:DISPlay` on; empty →
  `:ETABle<n>:ROW?` returns 0.
- Decoder must actually be configured for SPI (MODE), enabled, with valid source channels/CS; column
  (MISO/MOSI) cannot be set if the corresponding source is OFF.
- Frames must occur during acquisition (RUN + triggered capture; CS framing or timeout).
- Transport: partial/empty reads can come from binary transfer issues/timeouts → mitigated by chunk
  size, raw reads, timeouts, error drain; retry.
- Mitigation checklist: run `decoder.py --protocol spi --mode configure` (write + readback-verify),
  then `capture` (software decode → VCD/CSV) and/or `readback`; prefer CS mode; adequate sample rate;
  set thresholds; verify event table via `:ETABle<n>:ROW?`; cross-check instrument events vs software
  decode.

## Sources

- RIGOL DS1000Z Programming Guide (local): `/home/stefan/.config/opencode/skills/scpi/mans.io-MZFKPOun.pdf`
  (PGA19110-1110; SW 00.04.03.SP2) — Ch. 2 (`:DECoder`, `:ETABle`, `:ACQuire`, `:TIMebase`, `:TRIGger:SPI`),
  TMC block format.
- RIGOL DS1000Z User Guide (web): <https://www.rigol.com/dam/global/downloads/brochures/en/user-manual/oscillosopes/DS1000Z_UserGuide_EN.pdf> —
  Ch. 8 Protocol Decoding, thresholds, event table.
- RIGOL DS1000Z datasheet (web): <https://www.rigol.com/dam/global/downloads/brochures/en/data-sheet/oscilloscopes/DS1000Z_DataSheet_EN.pdf> —
  sample rates, memory depth, SPI trigger/decode specs.
- pyvisa-py / PyVISA issues: #20, #414, #463, #472, #586 — USBTMC read behavior with Rigol scopes.
- PyVISA docs: reading values / binary reads — termination handling.
- Local tooling: `scpi/scripts/decoder.py`, `scpi/scripts/scpi_base.py`, `scpi/scripts/README.md`,
  `scpi/SKILL.md`, `scpi/scripts/devices/*`; repo `test/hil_spi.py`, `test/_shared.py`, `test/README.md`.

## Open questions for follow-up implementation

1. If the goal is a reliable "SPI tier gate", is the instrument `:ETABle<n>:DATA?` path (given the
   undocumented payload layout) or the software-decode path (VCD/CSV → own decoder) the better
   authoritative source?
2. Event payload layout needs to be reverse-engineered/validated before bytes can be compared to the
   expected MOSI table via the readback alone.
3. Scope-side decode reliability for the MCP3x6x SPI traffic (sample rate/threshold/CS/timeout) should
   be validated against real captures before treating `:ETABle1:DATA?` output as final.
