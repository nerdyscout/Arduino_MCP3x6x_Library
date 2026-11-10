#!/usr/bin/env bash
# Generate 2D board renders (top + bottom) for each breakout PCB using KiBot.
#
# Output filenames use _top / _bottom suffixes (e.g. Feather_top.svg).
# All renders go into the same folder as the .kicad_pcb file.
#
# KiCad variables ${DATE} and ${VERSION} are defined in each board's
# .kicad_pro (text_variables) and .kicad_pcb (title block date/rev),
# so they appear on the rendered board image itself.
#
# Usage: ./scripts/run_kibot.sh   (run from repo root)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

RENDER_CONFIG="$REPO_ROOT/pcb/render.kibot.yaml"
FAB_CONFIG="$REPO_ROOT/pcb/fab.kibot.yaml"

# board name -> kicad_pcb path (relative to repo root)
declare -A BOARDS=(
  ["Feather"]="pcb/Adafruit/Feather.kicad_pcb"
  ["Uno"]="pcb/Arduino/Uno.kicad_pcb"
  ["ProMicro"]="pcb/Sparkfun/ProMicro.kicad_pcb"
)

for board in "${!BOARDS[@]}"; do
  pcb_file="$REPO_ROOT/${BOARDS[$board]}"
  board_dir="$(dirname "$pcb_file")"

  if [[ ! -f "$pcb_file" ]]; then
    echo "skip $board: $pcb_file not found" >&2
    continue
  fi

  echo "render $board top -> $board_dir"
  kibot -b "$pcb_file" -c "$RENDER_CONFIG" -d "$board_dir" board_render_top

  echo "render $board bottom -> $board_dir"
  kibot -b "$pcb_file" -c "$RENDER_CONFIG" -d "$board_dir" board_render_bottom

  echo "fab $board -> $board_dir/fab"
  kibot -b "$pcb_file" -c "$FAB_CONFIG" -d "$board_dir"
done

echo "done: renders next to each .kicad_pcb (Feather_top.svg, Feather_bottom.svg, ...)"
echo "done: fab outputs in pcb/<board>/fab/ (<board>_fab.zip, gerbers, bom, cpl)"
