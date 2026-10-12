# Article: robust weighting in photometric visual servoing — simulation study

- `PLAN.md` — study plan: decision rule, hypotheses, design, statistics, outline.
- `VALIDATION.md` — reproduction of Chapter 4 with the thesis program, and the
  findings it produced (occluder coverage, translation metric, principal point).
- `PILOT.md` — results of the 160-run pilot (development set); outputs in `pilot/`.
- `PREREGISTRATION.md` — hypotheses and decision rule for the confirmatory
  campaign, committed before its first run.
- `PREREGISTRATION_ADDENDUM.md` — replication on a second texture and occlusion sweep.
- `RESULTS.md` — results of all campaigns; outputs in `confirmatory/`, `replication/`, `sweep/`.
- `paper/` — the article draft (`paper/README.md`).

**To run everything on your own computer, follow `RUN_ON_YOUR_PC.md`.** In short:

```bash
python3 -m venv .venv && source .venv/bin/activate && pip install -r requirements.txt
cmake -S cpp -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
./build/test_campaign campaign/poses.csv && python3 analysis/selftest.py
mkdir -p textures && cp /path/to/peppers.jpg /path/to/peppers_o_b.jpg textures/
tools/make_textures.sh
JOBS=4 campaign/run_campaign.sh validate        # then pilot, confirmatory, replication, sweep
```
