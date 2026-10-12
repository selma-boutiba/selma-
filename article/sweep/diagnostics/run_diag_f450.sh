#!/usr/bin/env bash
# Pass-1 weight and influence maps at 45 % occlusion, pose 1 (RESULTS.md section 7.2).
# SCRATCH: directory holding cppbuild/ and campaign_tex/.
B=$SCRATCH/cppbuild/herpvs_campaign
for spec in "2 0 V2_Lc" "3 0 V3_Lc" "2 1 V2_Ld" "3 1 V3_Ld"; do
  set -- $spec
  $B -d -c -n 30 -w $1 -L $2 -x 1 -r $SCRATCH/campaign_tex/peppers.jpg -t $SCRATCH/campaign_tex/uniform_dark_f450.png \
     -P campaign/poses.csv -p 1 -o f450_$3_p1.csv -m f450_$3_p1 > f450_$3_p1.txt 2>&1 &
done
wait
