#!/usr/bin/env bash
#
# build.sh — Build every example for every PlatformIO environment.
#
# Usage:
#   ./build.sh                     # build all examples × all envs
#   ./build.sh ESP32 SAMD21G18A     # build only these environments
#   ./build.sh MCP3564_Mux_SingleEnded  # build only this example (all envs)
#   ./build.sh ESP32 MCP3564_Mux_SingleEnded  # mixed: envs then examples
#
# The script auto-discovers:
#   * environments from [env:NAME] sections in platformio.ini
#   * examples from subdirectories of examples/
#
# Adding a new example or environment requires no script changes.
#

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="${ROOT_DIR}/../src"
EXAMPLES_DIR="${ROOT_DIR}"
PLATFORMIO_INI="${ROOT_DIR}/../platformio.ini"
TMP_ROOT="/tmp/opencode/mcp3x6x_builds"

# ---------------------------------------------------------------------------
# Discover environments from platformio.ini [env:NAME] sections
# ---------------------------------------------------------------------------
discover_envs() {
    grep -E '^\[env:' "$PLATFORMIO_INI" | sed -E 's/^\[env:(.*)\]/\1/' | tr '\n' ' '
}

# ---------------------------------------------------------------------------
# Discover examples from examples/ subdirectories
# ---------------------------------------------------------------------------
discover_examples() {
    local dirs=()
    for d in "$EXAMPLES_DIR"/*/; do
        dirs+=("$(basename "$d")")
    done
    printf '%s\n' "${dirs[@]}" | sort | tr '\n' ' '
}

# ---------------------------------------------------------------------------
# Build one example for one environment using a scratch repo copy
# ---------------------------------------------------------------------------
build_one() {
    local env="$1"
    local example="$2"
    local bdir="${TMP_ROOT}/${env}_${example}"

    rm -rf "$bdir"
    mkdir -p "$bdir/src"

    # Copy library sources + manifest files into scratch build dir
    cp -r "$SRC_DIR" "$SRC_DIR/.."/library.json "$SRC_DIR/.."/library.properties "$SRC_DIR/.."/platformio.ini "$bdir/"

    # Copy example .ino files into scratch src/
    local ino_count=0
    for f in "$EXAMPLES_DIR/$example"/*.ino; do
        [ -f "$f" ] || continue
        cp "$f" "$bdir/src/"
        ino_count=$((ino_count + 1))
    done

    if [ "$ino_count" -eq 0 ]; then
        echo "SKIP  [${env}] ${example}  (no .ino files)"
        return 0
    fi

    local out
    if out=$(pio run -e "$env" -d "$bdir" 2>&1); then
        local status_line
        status_line=$(echo "$out" | grep -E '^\s+[0-9]+ succeeded' || true)
        echo "PASS  [${env}] ${example}  ${status_line}"
        return 0
    else
        local status_line
        status_line=$(echo "$out" | grep -E '^\s+[0-9]+ failed' || echo "FAILED")
        echo "FAIL  [${env}] ${example}  ${status_line}"
        # Save the full log for debugging
        echo "$out" > "${TMP_ROOT}/${env}_${example}.log"
        return 1
    fi
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
ENVS_REQUESTED=()
EXAMPLES_REQUESTED=()

for arg in "$@"; do
    # Heuristic: if it matches an example dir name, treat as example; else env
    if [ -d "${EXAMPLES_DIR}/${arg}" ]; then
        EXAMPLES_REQUESTED+=("$arg")
    else
        ENVS_REQUESTED+=("$arg")
    fi
done

ALL_ENVS=$(discover_envs)
ALL_EXAMPLES=$(discover_examples)

# Resolve envs
if [ ${#ENVS_REQUESTED[@]} -eq 0 ]; then
    ENVS=($ALL_ENVS)
else
    ENVS=("${ENVS_REQUESTED[@]}")
fi

# Resolve examples
if [ ${#EXAMPLES_REQUESTED[@]} -eq 0 ]; then
    EXAMPLES=($ALL_EXAMPLES)
else
    EXAMPLES=("${EXAMPLES_REQUESTED[@]}")
fi

echo "============================================================"
echo " build.sh — example × environment build matrix"
echo "============================================================"
echo " Environments (${#ENVS[@]}): ${ENVS[*]}"
echo " Examples      (${#EXAMPLES[@]}): ${EXAMPLES[*]}"
echo "============================================================"

mkdir -p "$TMP_ROOT"

PASS=0
FAIL=0
SKIP=0
TOTAL=0

for env in "${ENVS[@]}"; do
    # Validate env exists
    if ! echo " $ALL_ENVS " | grep -q " $env "; then
        echo "WARNING: environment '${env}' not found in platformio.ini — skipping"
        continue
    fi

    for example in "${EXAMPLES[@]}"; do
        # Validate example exists
        if [ ! -d "${EXAMPLES_DIR}/${example}" ]; then
            echo "WARNING: example '${example}' not found — skipping"
            continue
        fi

        TOTAL=$((TOTAL + 1))
        if build_one "$env" "$example"; then
            PASS=$((PASS + 1))
        else
            FAIL=$((FAIL + 1))
        fi
    done
done

echo "============================================================"
echo " Summary: ${TOTAL} builds, ${PASS} passed, ${FAIL} failed, ${SKIP} skipped"
echo "============================================================"

if [ "$FAIL" -gt 0 ]; then
    echo "Failed build logs are in ${TMP_ROOT}/<env>_<example>.log"
    exit 1
fi

exit 0
