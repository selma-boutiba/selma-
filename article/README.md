# Article: robust weighting in photometric visual servoing — simulation study

`PLAN.md` is the study plan: decision rule, hypotheses, design, statistics,
outline and timeline. Quick start:

```bash
python3 tools/make_poses.py --check-chapter4                      # 40.0 / 65.0 px
python3 tools/make_occluders.py generate --texture peppers.jpg --kind uniform_dark \
        --fraction 0.07 --out-dir campaign/textures
cmake -S cpp -B build && cmake --build build && ./build/test_campaign campaign/poses.csv
BIN=./viewer15 JOBS=3 campaign/run_campaign.sh validate           # after cpp/INTEGRATION.md
BIN=./viewer15 JOBS=3 campaign/run_campaign.sh pilot
python3 analysis/analyze.py campaign/runs/pilot/logs --poses campaign/poses_pilot.csv --out results/pilot
python3 analysis/selftest.py                                       # pipeline check on synthetic data
```

Python: numpy, scipy, pandas, matplotlib, pillow. C++: ViSP ≥ 3.6 (core).
