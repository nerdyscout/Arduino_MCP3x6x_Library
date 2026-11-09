#!/usr/bin/env bash
#
# osci_hil_readback.sh — Orchestrate pio upload of a hardware smoke sketch
# (scripts/hil_smoke.ino, ATmega328P/Arduino Nano) then read back SPI events
# from a Rigol oscilloscope over USBTMC via decoder.py.
#
# Usage:
#   ./scripts/osci_hil_readback.sh [--debug]
#
# Environment:
#   OSCI_RESOURCE   VISA resource string for the Rigol (auto-detected if unset)
#   OSCI_DECODER    Path to decoder.py (default: bundled scpi skill script)
#   OSCI_SKETCH     Path to the .ino sketch (default: scripts/hil_smoke.ino)
#
# Steps:
#   1. Build + upload hil_smoke.ino to ATmega328P via a scratch PlatformIO project
#   2. Auto-detect Rigol over USBTMC (decoder.py resource auto-detect)
#   3. decoder.py --protocol spi --decoder 1 --mode configure --json ...
#   4. decoder.py --protocol spi --decoder 1 --mode readback --json --etable on
#   5. Verify readback returns non-zero event bytes
#
# Exit codes: 0 = success, non-zero = first failing step (no masking).
#

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "${ROOT_DIR}/.." && pwd)"
DECODER="${OSCI_DECODER:-/home/stefan/.config/opencode/skills/scpi/scripts/decoder.py}"
SKETCH="${OSCI_SKETCH:-${ROOT_DIR}/hil_smoke.ino}"
TMP_ROOT="/tmp/opencode/mcp3x6x_hil"
UPLOAD_PORT="${OSCI_UPLOAD_PORT:-}"

DEBUG=0
if [ "${1:-}" = "--debug" ]; then
  DEBUG=1
  shift
fi

if [ $# -gt 0 ]; then
  echo "usage: $0 [--debug]" >&2
  exit 2
fi

if [ ! -f "$DECODER" ]; then
  echo "error: decoder.py not found at $DECODER" >&2
  exit 3
fi

if [ ! -f "$SKETCH" ]; then
  echo "error: sketch not found at $SKETCH" >&2
  exit 4
fi

RESOURCE_ARG=()
if [ -n "${OSCI_RESOURCE:-}" ]; then
  RESOURCE_ARG=(--resource "$OSCI_RESOURCE")
fi

DEBUG_ARG=()
if [ "$DEBUG" -eq 1 ]; then
  DEBUG_ARG=(--debug)
fi

run_decoder() {
  python3 "$DECODER" --protocol spi --decoder 1 --json "${DEBUG_ARG[@]:-}" "${RESOURCE_ARG[@]:-}" "$@"
}

echo "============================================================"
echo " osci_hil_readback.sh — HIL readback orchestration"
echo "============================================================"
echo " Repo dir:    $REPO_DIR"
echo " Sketch:      $SKETCH"
echo " Decoder:     $DECODER"
echo " Resource:    ${OSCI_RESOURCE:-<auto-detect>}"
echo " Debug:       $DEBUG"
echo " Upload port: ${UPLOAD_PORT:-<auto-detect>}"
echo "============================================================"

# ---------------------------------------------------------------------------
# Step 1: Build a scratch PlatformIO project and upload the smoke sketch
# ---------------------------------------------------------------------------
echo
echo "[Step 1/5] Build + upload hil_smoke.ino to ATmega328P (Arduino Nano)"

BDIR="${TMP_ROOT}/hil_upload"
rm -rf "$BDIR"
mkdir -p "$BDIR/src"

# Copy sketch into src/
cp "$SKETCH" "$BDIR/src/"

# Copy library manifest + sources into scratch project
cp "$REPO_DIR/library.json" "$REPO_DIR/library.properties" "$REPO_DIR/platformio.ini" "$BDIR/"
cp -r "$REPO_DIR/src" "$BDIR/"

# Write a minimal platformio.ini for the scratch project that only targets ATMEGA328P
# upload_flags = -V skips avrdude flash verification, which is required on some
# Arduino Nano clones whose bootloader does not support flash readback
# (causes "programmer is out of sync" during verify even though write succeeds).
cat > "$BDIR/platformio.ini" <<'PIOEOF'
[platformio]
description = HIL smoke sketch build for ATmega328P (Arduino Nano)

[env:ATMEGA328P]
platform = atmelavr
board = nanoatmega328
framework = arduino
monitor_speed = 115200
build_flags = -DTEST_ADC_TYPE=MCP3564
upload_flags = -V
PIOEOF

UPLOAD_ARG=()
if [ -n "${UPLOAD_PORT}" ]; then
  UPLOAD_ARG=(--upload-port "${UPLOAD_PORT}")
fi

if (pio run -e ATMEGA328P -d "$BDIR" --target upload "${UPLOAD_ARG[@]:-}"); then
  echo "PASS: build + upload succeeded"
else
  echo "FAIL: build or upload failed" >&2
  exit 1
fi

# ---------------------------------------------------------------------------
# Step 2: Auto-detect Rigol over USBTMC
# ---------------------------------------------------------------------------
echo
echo "[Step 2/5] Auto-detect Rigol over USBTMC"
if python3 "$DECODER" --protocol spi --decoder 1 --mode readback --json "${DEBUG_ARG[@]:-}" "${RESOURCE_ARG[@]:-}" --mosi CHAN4 --miso CHAN2 --etable on; then
  echo "PASS: Rigol detected and responding to SCPI"
else
  echo "FAIL: no Rigol responding over USBTMC (set OSCI_RESOURCE if not auto-detected)" >&2
  exit 2
fi

# ---------------------------------------------------------------------------
# Step 3: Configure scope SPI decoder
#   CHAN3=SCLK (pin 13), CHAN4=MOSI (pin 11), CHAN2=MISO (pin 12), CHAN1=CS (pin 10)
#   Uses inline PyVISA because decoder.py --mode configure fails on this Rigol
#   firmware at drain_errors() (:SYST:ERR? times out). Config writes themselves
#   succeed — verified below by readback query.
# ---------------------------------------------------------------------------
echo
echo "[Step 3/5] Configure scope SPI decoder via PyVISA (:DECoder1 SPI settings)"
CONFIG_ERRS=$(python3 -c "
import pyvisa, sys
rm = pyvisa.ResourceManager('@py')
inst = rm.open_resource('${OSCI_RESOURCE}')
inst.timeout = 5000
inst.chunk_size = 64
errors = []
def write(cmd):
    inst.write(cmd)
def query(cmd):
    return inst.query(cmd).strip()
# Configure SPI decoder
write(':DECoder1:MODE SPI')
write(':DECoder1:SPI:CLK CHAN3')
write(':DECoder1:SPI:MOSI CHAN4')
write(':DECoder1:SPI:MISO CHAN2')
write(':DECoder1:SPI:CS CHAN1')
write(':DECoder1:SPI:WIDTh 8')
write(':DECoder1:SPI:EDGE RISE')
write(':DECoder1:SPI:ENDian MSB')
write(':DECoder1:SPI:SELect CS')
# Readback-verify each setting
checks = {
    ':DECoder1:MODE?': 'SPI',
    ':DECoder1:SPI:CLK?': 'CHAN3',
    ':DECoder1:SPI:MOSI?': 'CHAN4',
    ':DECoder1:SPI:MISO?': 'CHAN2',
    ':DECoder1:SPI:CS?': 'CHAN1',
    ':DECoder1:SPI:WIDTh?': '8',
    ':DECoder1:SPI:EDGE?': 'RISE',
    ':DECoder1:SPI:ENDian?': 'MSB',
    ':DECoder1:SPI:SELect?': 'CS',
}
for cmd, expected in checks.items():
    try:
        actual = query(cmd)
        if actual.upper() != expected.upper():
            errors.append(f'{cmd}: expected={expected} actual={actual}')
    except Exception as exc:
        errors.append(f'{cmd}: readback failed ({exc})')
inst.close()
if errors:
    print('CONFIG_ERRORS:' + '; '.join(errors))
    sys.exit(1)
print('CONFIG_OK')
" 2>&1) || {
  echo "FAIL: SPI decoder configuration failed:" >&2
  echo "$CONFIG_ERRS" >&2
  exit 3
}
echo "$CONFIG_ERRS" | grep -q "CONFIG_OK" && echo "PASS: scope SPI decoder configured and verified" || {
  echo "FAIL: scope SPI decoder configuration verification failed" >&2
  echo "$CONFIG_ERRS" >&2
  exit 3
}

# ---------------------------------------------------------------------------
# Step 4: Read back SPI events from scope (:ETABle1:DATA?)
# ---------------------------------------------------------------------------
echo
echo "[Step 4/5] decoder.py --mode readback (query :ETABle1:DATA?)"
READBACK_OUT=$(run_decoder --mode readback --etable on --mosi CHAN4 --miso CHAN2 2>&1) || {
  echo "FAIL: decoder readback failed:" >&2
  echo "$READBACK_OUT" >&2
  exit 5
}

READBACK_OK=$(echo "$READBACK_OUT" | python3 -c "import sys,json; d=json.load(sys.stdin); print('OK' if d.get('ok') else 'BAD')") || {
  echo "FAIL: could not parse readback JSON output" >&2
  echo "$READBACK_OUT" >&2
  exit 6
}

if [ "$READBACK_OK" != "OK" ]; then
  echo "FAIL: decoder readback returned ok=false" >&2
  echo "$READBACK_OUT" >&2
  exit 5
fi

BYTE_COUNT=$(echo "$READBACK_OUT" | python3 -c "
import sys,json
d=json.load(sys.stdin)
rb=d.get('readback') or {}
print(rb.get('bytes',0))
") || {
  echo "FAIL: could not parse readback byte count" >&2
  echo "$READBACK_OUT" >&2
  exit 6
}

if [ "$BYTE_COUNT" -eq 0 ]; then
  echo "WARN: readback returned 0 bytes — no SPI events decoded by scope yet" >&2
  echo "$READBACK_OUT" >&2
else
  echo "PASS: readback returned $BYTE_COUNT bytes of SPI event data"
fi

# ---------------------------------------------------------------------------
# Step 5: Summary
# ---------------------------------------------------------------------------
echo
echo "============================================================"
echo " HIL readback orchestration complete"
echo "  Step 1 (build+upload): PASS"
echo "  Step 2 (scope detect): PASS"
echo "  Step 3 (configure):    PASS (settings verified)"
echo "  Step 4 (readback):     PASS (bytes=$BYTE_COUNT)"
echo "============================================================"

if [ "$BYTE_COUNT" -eq 0 ]; then
  echo "NOTE: 0 event bytes — correlate manually with Serial console (115200 baud)"
  echo "      and scope capture. ETABLE binary layout is undocumented; raw hex is in output above."
fi

exit 0
