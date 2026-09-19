#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cmake -S "$ROOT" -B "$ROOT/build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$HOME/.local"
ninja -C "$ROOT/build" $(portageq envvar MAKEOPTS)
ctest --test-dir "$ROOT/build" --output-on-failure
cmake --install "$ROOT/build"
printf '\nInstalled to %s/.local. Nothing has been enabled at login or on idle.\n' "$HOME"
