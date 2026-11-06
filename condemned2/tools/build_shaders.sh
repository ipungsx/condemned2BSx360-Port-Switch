#!/bin/bash
# Build Condemned 2's shader library from the containers the NRO dumped on the console.
#
#   DKSH=1 condemned2/tools/build_shaders.sh <shaders_dump folder> <output folder>
#
# Runs the shared toolchain (shaders/build_library.sh: XenosRecomp -> HLSL -> SPIR-V -> library, DKSH cache with
# DKSH=1), then names the library the way condemned2 loads it: c2_shaders.dslib. Copy c2_shaders.dslib and
# shadercache_dksh.bin next to condemned2-nx.nro on the SD card.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
IN="${1:?usage: build_shaders.sh <shaders_dump> <out>}"
OUT="${2:?usage: build_shaders.sh <shaders_dump> <out>}"
"$ROOT/shaders/build_library.sh" "$IN" "$OUT"
if [ -f "$OUT/cco_shaders.dslib" ]; then
  mv -f "$OUT/cco_shaders.dslib" "$OUT/c2_shaders.dslib"
fi
ls -la "$OUT/c2_shaders.dslib" "$OUT"/shadercache_dksh.bin 2>/dev/null || true
