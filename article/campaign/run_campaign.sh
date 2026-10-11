#!/usr/bin/env bash
# =====================================================================
#  run_campaign.sh -- run one design of the simulation campaign.
#
#  Usage:   ./run_campaign.sh pilot|confirmatory|sweep|validate
#  Environment (defaults in brackets):
#    BIN     servoing program built with campaign.h            [./viewer15]
#    JOBS    runs in parallel; at most the number of cores     [3]
#    NITER   iteration cap                                      [600]
#    REF     clean texture (reference image)                    [peppers.jpg]
#    TEX     directory of occluded textures (make_occluders.py) [textures]
#    OUT     output root                                        [runs]
#    PIX     1 corrected principal point, 0 thesis convention   [1]
#
#  Resumable: a run whose log already has NITER data rows is skipped, so
#  the script can be interrupted and restarted. Each run writes its log
#  to a temporary name and renames it only on completion.
# =====================================================================
set -euo pipefail

DESIGN=${1:?usage: run_campaign.sh pilot|confirmatory|sweep|validate}
BIN=${BIN:-./viewer15}
JOBS=${JOBS:-3}
NITER=${NITER:-600}
REF=${REF:-peppers.jpg}
TEX=${TEX:-textures}
OUT=${OUT:-runs}
PIX=${PIX:-1}
HERE=$(cd "$(dirname "$0")" && pwd)

WEIGHTINGS="1 2 3 4 5"         # none tukey hermite penalised adaptive
INTERACTIONS="0 1"             # current desired
SCENARIOS=${SCENARIOS:-"nominal uniform_dark_disc textured_disc low_contrast_disc"}   # make_occluders.py refill

case "$DESIGN" in
  validate)      # reproduction of Chapter 4: pose 0, thesis pixel order
    POSES=""; IDS="0"; WEIGHTINGS="1 2 3"; INTERACTIONS="0"
    SCENARIOS="nominal ch4_disc"; PIX=0 ;;   # ch4_disc: the thesis' occluded texture, in $TEX
  pilot)
    POSES="$HERE/poses_pilot.csv"; IDS="101 102 103 104" ;;
  confirmatory)
    POSES="$HERE/poses.csv"; IDS=$(seq 1 32)
    WEIGHTINGS=${CONF_WEIGHTINGS:-$WEIGHTINGS} ;;
  sweep)         # breakdown: occluded fraction at the desired pose
    POSES="$HERE/poses.csv"; IDS=$(seq 1 32); WEIGHTINGS="2 3 5"
    SCENARIOS="uniform_dark_f150 uniform_dark_f250 uniform_dark_f350 uniform_dark_f450" ;;
  *) echo "unknown design $DESIGN" >&2; exit 2 ;;
esac

DIR="$OUT/$DESIGN"
mkdir -p "$DIR/logs" "$DIR/console"

servo_texture() {
  if [ "$1" = nominal ]; then echo "$REF"
  elif [ -f "$TEX/$1.png" ]; then echo "$TEX/$1.png"
  elif [ -f "$TEX/$1.jpg" ]; then echo "$TEX/$1.jpg"
  else echo "missing texture for scenario $1" >&2; return 1; fi
}

jobs() {
  for L in $INTERACTIONS; do
    for S in $SCENARIOS; do
      T=$(servo_texture "$S")
      for W in $WEIGHTINGS; do
        for P in $IDS; do
          echo "$L $S $T $W $P"
        done
      done
    done
  done
}

run_one() {
  local L=$1 S=$2 T=$3 W=$4 P=$5
  local name="L${L}_${S}_w${W}_p${P}"
  local log="$DIR/logs/$name.csv"
  if [ -f "$log" ] && [ "$(grep -vc '^#' "$log")" -gt "$NITER" ]; then return 0; fi
  local poseargs=()
  [ -n "$POSES" ] && poseargs=(-P "$POSES")
  if "$BIN" -d -c -n "$NITER" -w "$W" -L "$L" -x "$PIX" -r "$REF" -t "$T" \
        "${poseargs[@]}" -p "$P" -o "$log.part" > "$DIR/console/$name.txt" 2>&1; then
    mv "$log.part" "$log"
  else
    echo "run $name failed (see $DIR/console/$name.txt)" >&2
  fi
}
export -f run_one
export DIR NITER BIN PIX REF POSES

N=$(jobs | wc -l)
echo "$DESIGN: $N runs, $JOBS in parallel, logs in $DIR/logs"
jobs | xargs -P "$JOBS" -L 1 bash -c 'run_one "$@"' _
echo "done: $(ls "$DIR/logs"/*.csv 2>/dev/null | wc -l) of $N logs complete"
