# Running the study on your own computer

This guide takes a fresh computer to the full set of results: build, one
servoing run, the five campaigns, the analyses and the article. Every command
below was tested on a fresh clone of this branch (Ubuntu 24.04, ViSP 3.6.0,
GCC 13, Python 3.12). A run started there from the same pose reproduced the
committed result to every printed digit.

Commands are run from the `article/` directory unless stated otherwise.

## 1. What you need

| Item | Version | Note |
|---|---|---|
| Operating system | Ubuntu 22.04 or 24.04 | On Windows, use WSL 2 with Ubuntu (§2.2) |
| ViSP | 3.3 or later | modules core, io, robot; no modified ViSP class is needed |
| CMake, C++ compiler | CMake 3.10+, C++17 | `g++` 9 or later |
| Python | 3.9 or later | packages in `requirements.txt` |
| Your two thesis textures | `peppers.jpg`, `peppers_o_b.jpg` | not in the repository (copyright) |
| Disk | about 1 GB | 280 KB of log per run |
| Memory | about 250 MB per parallel run | |

**Run time.** One run of 600 iterations takes about 2.5 min on one core. The
campaigns take hours (§6), and they can be interrupted and resumed.

## 2. Install the software

### 2.1 Ubuntu

```bash
sudo apt update
sudo apt install -y git cmake g++ python3 python3-venv libvisp-dev
```

`libvisp-dev` is ViSP 3.6 on Ubuntu 24.04 and 3.5 on 22.04; both work. If you
already use ViSP built from source for the thesis, skip `libvisp-dev` and give
its build directory to CMake in §4 with `-DVISP_DIR=/path/to/visp-build`.

### 2.2 Windows (WSL 2)

1. In PowerShell, as administrator, run:

   ```powershell
   wsl --install -d Ubuntu-24.04
   ```

   Restart Windows, open **Ubuntu** from the Start menu, and choose a user name
   and password.
2. In the Ubuntu window, run the commands of §2.1.
3. Keep the repository in the Linux home directory (`~/...`), not under
   `/mnt/c/`. Runs are much slower on the Windows file system.
4. Windows files are under `/mnt/c/`. For example:

   ```bash
   cp /mnt/c/Users/<you>/Documents/peppers.jpg textures/
   ```

5. Linux files are visible in Windows Explorer at
   `\\wsl$\Ubuntu-24.04\home\<user>\`, so you can open the figures and the
   PDF there.

A native Windows build, with Visual Studio and ViSP compiled from source, can
compile the C++ program. The campaign scripts, however, need `bash`. That
route has not been tested.

### 2.3 macOS

`brew install visp cmake python` provides the dependencies; the rest of the
guide applies unchanged. This route has not been tested.

## 3. Get the code and the Python packages

The repository is private. Clone it with your GitHub account (HTTPS with a
personal access token, or SSH):

```bash
git clone -b claude/chapter-4-improvements-fqjzce https://github.com/selma-boutiba/selma-.git
cd selma-/article
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

Run `source .venv/bin/activate` again in every new terminal before using the
Python scripts.

## 4. Build and check

```bash
cmake -S cpp -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/test_campaign campaign/poses.csv    # last line: PASSED (0 failures)
python3 analysis/selftest.py                 # nine lines starting with "ok"
```

Build in `Release` mode. A `Debug` build is several times slower.

## 5. Textures

```bash
mkdir -p textures
cp /path/to/peppers.jpg /path/to/peppers_o_b.jpg textures/
tools/make_textures.sh
```

The script writes every texture of the study into `textures/`:
- the four appearances of the Chapter 4 disc;
- the four ellipses of the sweep;
- the second texture (`coffee.png`) and its disc.

It also writes the measured coverages in `textures/coverage_*.txt`. With the
thesis textures, the generated files are pixel-identical to those used for the
reported results.

## 6. One run

```bash
./build/herpvs_campaign -d -c -n 600 -w 3 -L 1 -x 1 \
    -r textures/peppers.jpg -t textures/uniform_dark_disc.png \
    -P campaign/poses.csv -p 1 -o one_run.csv
python3 analysis/plot_run.py one_run.csv     # prints the final errors, writes one_run.png
```

The last line printed by the program should be:

```
### final pass 600  errorpose(m, rad) 4.16788e-07 9.00742e-07 -1.53515e-05 0.000299727 0.000133889 -3.75872e-06  |e|^2 1.59696e+10  lambda 2.5175  mu 5.69713e-09  |Tc| 3.7993e-06
```

`plot_run.py` should report a camera-centre error of 0.4307 mm and a rotation
error of 0.01894°. A different compiler or ViSP version can change the last
digits (see `VALIDATION.md` §2).

The program has no display window. It writes one CSV line per iteration; the
columns are listed in `cpp/campaign.h`. Its options (`./build/herpvs_campaign -h`):

| Option | Meaning |
|---|---|
| `-w 1..4` | weighting: V1 none, V2 Tukey, V3 structure-informed (Hermite), V4 structure-penalised (5: adaptive, pilot only) |
| `-L 0\|1` | interaction matrix from the current image (thesis) or the desired image |
| `-p ID` | initial pose: 1–32 from `-P campaign/poses.csv`; 0 is the Chapter 4 pose |
| `-r`, `-t` | reference (desired) texture and servoing texture |
| `-n N` | number of iterations |
| `-x 0\|1` | pixel convention: thesis (swapped) or corrected; the study uses 1 |
| `-G 0\|1`, `-F 0\|1` | structure measure from summed or first-order kernels; floor on the damped Hessian |
| `-o FILE`, `-m PREFIX` | per-iteration log; weight and influence maps at the first and last iterations |

## 7. The campaigns

`campaign/run_campaign.sh` runs one design. `JOBS` is the number of runs in
parallel; set it to your number of physical cores. Times below are for four
cores, as measured here.

| Design | Command | Runs | Time on 4 cores |
|---|---|---|---|
| Validation (Chapter 4 pose, thesis conventions) | `JOBS=4 campaign/run_campaign.sh validate` | 6 | 5 min |
| Pilot (development poses, V1–V5) | `JOBS=4 campaign/run_campaign.sh pilot` | 160 | 1.5 h |
| Confirmatory (V1–V4) | `JOBS=4 campaign/run_campaign.sh confirmatory` | 1 024 | 10 h |
| Replication (second texture) | `JOBS=4 campaign/run_campaign.sh replication` | 256 | 2.5 h |
| Occlusion sweep | `JOBS=4 campaign/run_campaign.sh sweep` | 512 | 5 h |

Logs go to `runs/<design>/logs/` and the console output of each run to
`runs/<design>/console/`.

**Interrupting and resuming.** A campaign can be stopped (Ctrl-C) and
restarted with the same command; completed runs are skipped.

**Long campaigns.** Run them detached, and keep the computer from sleeping:

```bash
nohup env JOBS=4 campaign/run_campaign.sh confirmatory > confirmatory.log 2>&1 &
ls runs/confirmatory/logs | wc -l     # progress: number of completed runs
```

## 8. Analyses

The commands are those fixed in `PREREGISTRATION.md` and
`PREREGISTRATION_ADDENDUM.md`. They write to `results/`, so the committed
outputs in `confirmatory/`, `replication/` and `sweep/` are not overwritten.

**Confirmatory.**

```bash
python3 analysis/analyze.py runs/confirmatory/logs --poses campaign/poses.csv \
    --out results/confirmatory
```

**Replication.**

```bash
python3 analysis/analyze.py runs/replication/logs --poses campaign/poses.csv \
    --out results/replication --primary desired:coffee_dark_disc:hermite:tukey \
    --primary-success "" --decision-labels "replicated,not replicated"
```

**Sweep.**

```bash
C=uniform_dark_f150,uniform_dark_f250,uniform_dark_f350,uniform_dark_f450
P=$(for L in current desired; do for s in ${C//,/ }; do printf "%s:%s:hermite:tukey," $L $s; done; done)
python3 analysis/analyze.py runs/sweep/logs --poses campaign/poses.csv --out results/sweep \
    --primary "$P" --primary-success "$P" \
    --decision-labels "gain at every fraction,gain not at every fraction"
python3 analysis/sweep_figures.py --sweep results/sweep --confirmatory results/confirmatory \
    --masks textures --poses campaign/poses.csv
```

**Exploratory: the V3/V2 ratio against the iteration budget** (`RESULTS.md` §4).

```bash
python3 analysis/budget_ratio.py runs/confirmatory/logs --out results/confirmatory/budget_ratio.csv
```

**Comparing with the reported results.**

```bash
python3 analysis/compare_results.py results/confirmatory confirmatory
```

Each `results/<design>/` directory contains:
- `report.md`: the summary;
- `primary.csv` and `decision.txt`: the pre-registered tests and the decision;
- `summary.csv`, `comparisons.csv`, `mcnemar.csv`: the per-cell statistics;
- `figures/`.

## 9. Figures and article

```bash
python3 paper/make_figures.py                # writes paper/figures/
sudo apt install -y texlive-latex-extra texlive-publishers texlive-science \
    texlive-fonts-recommended lmodern latexmk
cd paper && latexmk -pdf main.tex            # writes paper/main.pdf
```

Most figures are drawn from the committed outputs. Two need local inputs:
- Fig. 1 needs `textures/`.
- The convergence-curve figure needs `runs/confirmatory/logs/`.

Either is skipped, with a message, if its input is missing.

## 10. Problems

| Symptom | Remedy |
|---|---|
| CMake: `Could not find a package configuration file provided by "VISP"` | install `libvisp-dev`, or pass `-DVISP_DIR=` to your ViSP build directory |
| `program not found: .../build/herpvs_campaign` | build first (§4), or set `BIN=/path/to/herpvs_campaign` |
| `reference texture not found` or `missing texture for scenario ...` | run `tools/make_textures.sh` (§5) |
| `pose N not found` | give the pose file with `-P campaign/poses.csv` |
| `ModuleNotFoundError` in Python | `source .venv/bin/activate`, then `pip install -r requirements.txt` |
| runs much slower than 2.5 min | Release build; `JOBS` no larger than the number of physical cores; under WSL, keep the files in `~/` |
