#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cmake -S "$ROOT" -B "$ROOT/build" -G Ninja -DCMAKE_BUILD_TYPE=Release "$@"
ninja -C "$ROOT/build" $(portageq envvar MAKEOPTS)
ctest --test-dir "$ROOT/build" --output-on-failure
