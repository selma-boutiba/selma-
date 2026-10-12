#!/usr/bin/env bash
cd $SCRATCH/diag
B=$SCRATCH/cppbuild/herpvs_campaign; R=$SCRATCH/campaign_tex2/coffee.png; T=$SCRATCH/campaign_tex2/coffee_dark_disc.png; P=/home/user/selma-/article/campaign/poses.csv
run() { nice -n 10 $B -d -c -n 600 -w $1 -L 1 -x 1 -r $R -t $T -P $P -p 1 -o $2.csv -m $2 > $2.txt 2>&1; }
run 1 V1_Ld_p1 & run 2 V2_Ld_p1 & run 3 V3_Ld_p1 &
wait
echo DIAG2_DONE
