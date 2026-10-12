#!/usr/bin/env bash
cd $SCRATCH/diag
B=$SCRATCH/cppbuild/herpvs_campaign; R=$SCRATCH/campaign_tex2/coffee.png; T=$SCRATCH/campaign_tex2/coffee_dark_disc.png
run() { nice -n 10 $B -d -c -n 600 -w $1 -L $2 -x 1 -r $R -t $T -P $3 -p $4 -o $5.csv -m $5 > $5.txt 2>&1; }
run 2 1 poses_diag.csv 900 V2_Ld_from_desired &
run 3 1 poses_diag.csv 900 V3_Ld_from_desired &
wait
run 1 1 poses_diag.csv 900 V1_Ld_from_desired &
run 2 0 poses_diag.csv 900 V2_Lc_from_desired &
wait
echo DIAG_DONE
