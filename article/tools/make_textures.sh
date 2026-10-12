#!/usr/bin/env bash
# =====================================================================
#  make_textures.sh -- every texture of the study, from the two thesis
#  textures.
#
#  Usage:   tools/make_textures.sh [texture_dir]          (default: textures)
#  Needs, in texture_dir:
#    peppers.jpg       clean reference texture of Chapter 4
#    peppers_o_b.jpg   the same texture with the Chapter 4 black disc
#  Writes, in texture_dir:
#    uniform_dark_disc.png, uniform_bright_disc.png, low_contrast_disc.png,
#    textured_disc.png, mask_disc.png              confirmatory scenes
#    uniform_dark_f150 ... f450 .png (+ mask, json) occlusion sweep
#    coffee.png, coffee_dark_disc.png, mask_coffee.png   replication
#    ch4_disc.jpg                                  validation (copy)
# =====================================================================
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(dirname "$HERE")
T=${1:-$ROOT/textures}
P=$ROOT/campaign/poses.csv

for f in peppers.jpg peppers_o_b.jpg; do
  [ -f "$T/$f" ] || { echo "missing $T/$f (copy it from the thesis files)" >&2; exit 1; }
done

echo "== confirmatory scenes: the Chapter 4 disc, four appearances"
python3 "$HERE/make_occluders.py" refill --clean "$T/peppers.jpg" --occluded "$T/peppers_o_b.jpg" \
    --out-dir "$T" --poses "$P" > "$T/coverage_disc.txt"

echo "== validation scene"
cp "$T/peppers_o_b.jpg" "$T/ch4_disc.jpg"

echo "== occlusion sweep: dark ellipses"
for f in 0.15 0.25 0.35 0.45; do
  python3 "$HERE/make_occluders.py" generate --texture "$T/peppers.jpg" --kind uniform_dark \
      --fraction "$f" --out-dir "$T" > "$T/coverage_f$f.txt"
done

echo "== replication: second texture and the same disc"
python3 "$HERE/make_second_texture.py" --out "$T/coffee.png"
python3 "$HERE/make_occluders.py" refill --clean "$T/peppers.jpg" --occluded "$T/peppers_o_b.jpg" \
    --onto "$T/coffee.png" --kinds uniform_dark --tag coffee --out-dir "$T" > "$T/coverage_coffee.txt"
mv "$T/uniform_dark_coffee.png" "$T/coffee_dark_disc.png"

echo "done: textures in $T"
