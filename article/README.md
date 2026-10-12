# Article: robust weighting in photometric visual servoing — simulation study

- `PLAN.md` — study plan: decision rule, hypotheses, design, statistics, outline.
- `VALIDATION.md` — reproduction of Chapter 4 with the thesis program, and the
  findings it produced (occluder coverage, translation metric, principal point).
- `PILOT.md` — results of the 160-run pilot (development set); outputs in `pilot/`.
- `PREREGISTRATION.md` — hypotheses and decision rule for the confirmatory
  campaign, committed before its first run.
- `RESULTS.md` — confirmatory results (1 024 runs); outputs in `confirmatory/`.

Quick start (textures are not in the repository; copy the thesis' `peppers.jpg`
and `peppers_o_b.jpg` beside this file):

```bash
cmake -S cpp -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
./build/test_campaign campaign/poses.csv                          # unit checks
python3 tools/make_occluders.py refill --clean peppers.jpg --occluded peppers_o_b.jpg \
        --out-dir campaign/textures                               # occluded scenarios
cp peppers_o_b.jpg campaign/textures/ch4_disc.jpg
BIN=build/herpvs_campaign REF=peppers.jpg TEX=campaign/textures JOBS=4 \
    campaign/run_campaign.sh validate                             # Chapter 4 pose, thesis conventions
BIN=build/herpvs_campaign REF=peppers.jpg TEX=campaign/textures JOBS=4 \
    campaign/run_campaign.sh pilot
python3 analysis/analyze.py campaign/runs/pilot/logs --poses campaign/poses_pilot.csv --out results/pilot
python3 analysis/selftest.py                                       # pipeline check on synthetic data
```

`build/herpvs_campaign -h` lists the options. It needs ViSP ≥ 3.3 (core, io,
robot) and no modified ViSP class. Python: numpy, scipy, pandas, matplotlib,
pillow.
