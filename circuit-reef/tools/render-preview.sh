#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -euo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="${REEF_BIN:-$ROOT/build/circuit-reef}"
OUT="${1:-$ROOT/Circuit_Reef_preview.mp4}"
[[ -x "$BIN" ]] || { printf 'Build the executable first: %s\n' "$BIN" >&2; exit 1; }
command -v ffmpeg >/dev/null || { printf 'ffmpeg is required for video export.\n' >&2; exit 1; }
"$BIN" --raw-video - --size 1280x720 --fps 60 --duration 24 --seed 2026 --demo |
ffmpeg -nostdin -hide_banner -loglevel warning -y -f rawvideo -pixel_format bgra -video_size 1280x720 -framerate 60 -i - \
  -an -c:v libx264 -preset veryfast -threads 2 -crf 19 -pix_fmt yuv420p -movflags +faststart "$OUT"
printf 'Created %s\n' "$OUT"
