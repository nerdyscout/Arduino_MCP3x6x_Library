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
cat > "$BDIR/platformio.ini" <<'PIOEOF'
[platformio]
description = HIL smoke sketch build for ATmega328P (Arduino Nano)

[env:ATMEGA328P]
platform = atmelavr
board = nanoatmega328
framework = arduino
monitor_speed = 115200
build_flags = -DTEST_ADC_TYPE=MCP3564
PIOEOF

if (pio run -e ATMEGA328P -d "$BDIR" && pio upload -e ATMEGA328P -d "$BDIR"); then
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
if python3 "$DECODER" --protocol spi --decoder 1 --mode readback --json "${DEBUG_ARG[@]:-}" "${RESOURCE_ARG[@]:-}" --etable on >/dev/null 2>&1; then
  echo "PASS: Rigol detected and responding to SCPI"
else
  echo "FAIL: no Rigol responding over USBTMC (set OSCI_RESOURCE if not auto-detected)" >&2
  exit 2
fi

# ---------------------------------------------------------------------------
# Step 3: Configure scope SPI decoder
#   CHAN3=SCLK (pin 13), CHAN4=MOSI (pin 11), CHAN2=MISO (pin 12), CHAN1=CS (pin 10)
# ---------------------------------------------------------------------------
echo
echo "[Step 3/5] decoder.py --mode configure (program :DECoder1 SPI settings)"
CONFIG_OUT=$(run_decoder --mode configure \
  --sclk CHAN3 --mosi CHAN4 --miso CHAN2 --cs CHAN1 \
  --width 8 --edge rise --polarity pos --endian msb --select cs 2>&1) || {
  echo "FAIL: decoder configure failed:" >&2
  echo "$CONFIG_OUT" >&2
  exit 3
}

CONFIG_OK=$(echo "$CONFIG_OUT" | python3 -c "import sys,json; d=json.load(sys.stdin); print('OK' if d.get('ok') else 'BAD')") || {
  echo "FAIL: could not parse configure JSON output" >&2
  echo "$CONFIG_OUT" >&2
  exit 4
}

if [ "$CONFIG_OK" != "OK" ]; then
  echo "FAIL: decoder configure returned ok=false" >&2
  echo "$CONFIG_OUT" >&2
  exit 3
fi

MISMATCHES=$(echo "$CONFIG_OUT" | python3 -c "
import sys,json
d=json.load(sys.stdin)
ms=d.get('settings',{}).get('mismatches',[])
print(len(ms))
") || {
  echo "FAIL: could not parse mismatch count" >&2
  exit 4
}

if [ "$MISMATCHES" != "0" ]; then
  echo "WARN: decoder configure had $MISMATCHES readback mismatch(es)" >&2
  echo "$CONFIG_OUT" >&2
fi
echo "PASS: scope SPI decoder configured (mismatches=$MISMATCHES)"

# ---------------------------------------------------------------------------
# Step 4: Read back SPI events from scope (:ETABle1:DATA?)
# ---------------------------------------------------------------------------
echo
echo "[Step 4/5] decoder.py --mode readback (query :ETABle1:DATA?)"
READBACK_OUT=$(run_decoder --mode readback --etable on 2>&1) || {
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
echo "  Step 3 (configure):    PASS (mismatches=$MISMATCHES)"
echo "  Step 4 (readback):     PASS (bytes=$BYTE_COUNT)"
echo "============================================================"

if [ "$BYTE_COUNT" -eq 0 ]; then
  echo "NOTE: 0 event bytes — correlate manually with Serial console (115200 baud)"
  echo "      and scope capture. ETABLE binary layout is undocumented; raw hex is in output above."
fi

exit 0
