# Running the study on Windows with Visual Studio 2015 and ViSP

This guide is for a Windows computer on which ViSP is already built with
Visual Studio 2015, as for the thesis programs. It uses CMake GUI, Visual
Studio and the Windows Command Prompt (`cmd`); no Linux tools are needed. The
general guide is `RUN_ON_YOUR_PC.md`.

**What was tested.** The Python scripts were tested on Linux against the
`bash` scripts used for the study, and they give identical results. The C++
code compiles as C++14, the most Visual Studio 2015 supports. It does not use
ViSP's `vpRobust`, whose interface changed between ViSP versions: the Tukey
estimator is implemented in `cpp/campaign.h`, operation for operation as in
ViSP. The Windows build itself could not be tested here. If a step fails,
send the exact error message.

## 1. What you need

| Item | Note |
|---|---|
| Visual Studio 2015 with C++ | already installed |
| ViSP 3.x built with Visual Studio 2015 | its build directory contains `VISPConfig.cmake` |
| CMake 3.10 or later | the CMake you used to build ViSP |
| Python 3.10, 3.11 or 3.12, 64-bit | [python.org](https://www.python.org/downloads/windows/); tick **Add python.exe to PATH** during installation |
| Git for Windows (optional) | [git-scm.com](https://git-scm.com/download/win); otherwise download a ZIP (§2) |
| `peppers.jpg`, `peppers_o_b.jpg` | your two Chapter 4 textures |

## 2. Get the code

**With Git**, in a Command Prompt:

```bat
cd %USERPROFILE%\Documents
git clone -b claude/chapter-4-improvements-fqjzce https://github.com/selma-boutiba/selma-.git
```

**Without Git:**
1. Open the repository on github.com and choose the branch
   `claude/chapter-4-improvements-fqjzce`.
2. Click **Code → Download ZIP** and unzip it.

The rest of this guide calls the `article` folder of the repository
`ARTICLE`, for example `C:\Users\<you>\Documents\selma-\article`. Avoid
folder names with accents.

## 3. Build the program with CMake GUI and Visual Studio 2015

1. Open **CMake GUI** and set the two folders:
   - **Where is the source code:** `ARTICLE/cpp`
   - **Where to build the binaries:** `ARTICLE/build`
2. Click **Configure**. Choose the generator **Visual Studio 14 2015** with the
   same architecture as your ViSP build:
   - 64-bit: platform `x64`, or the generator named `Visual Studio 14 2015 Win64`;
   - 32-bit: no platform.
3. If CMake reports that it cannot find `VISP`, set the entry `VISP_DIR` to
   your ViSP build directory, the folder that contains `VISPConfig.cmake`
   (for example `C:/visp-ws/visp-build-vc14`). Then click **Configure**
   again.
4. Click **Generate**, then **Open Project**.
5. In Visual Studio, choose **Release** and **x64** (or **Win32**) in the
   toolbar, then **Build → Build Solution**. Do not use Debug: it is several
   times slower.

The programs are now `ARTICLE\build\Release\herpvs_campaign.exe` and
`ARTICLE\build\Release\test_campaign.exe`.

**DLLs.** The programs need the ViSP DLLs and those of its third-party
libraries, exactly as your thesis programs do. If Windows reports that a file
such as `visp_core3xx.dll` cannot be found, add the folder that contains it
to the `PATH` environment variable. That folder is typically
`...\visp-build-vc14\install\x64\vc14\bin` or `...\visp-build-vc14\bin\Release`.

## 4. Python

Open a **Command Prompt** (`cmd`, not PowerShell) and run:

```bat
cd /d ARTICLE
py -3 -m venv .venv
.venv\Scripts\activate.bat
python -m pip install --upgrade pip
pip install -r requirements.txt
```

In every new Command Prompt, run `cd /d ARTICLE` and
`.venv\Scripts\activate.bat` again before using the scripts.

## 5. Check

```bat
build\Release\test_campaign.exe campaign\poses.csv
python analysis\selftest.py
```

- **`test_campaign.exe`** must end with `PASSED (0 failures)`. With a ViSP
  older than 3.3, two comparisons with ViSP's own estimator are skipped.
- **`selftest.py`** must print nine lines starting with `ok`.

## 6. Textures

1. Create the folder `ARTICLE\textures`.
2. Copy `peppers.jpg` and `peppers_o_b.jpg` into it.
3. Run:

   ```bat
   python tools\make_textures.py
   ```

This writes every scene of the study into `textures\`:
- the four appearances of the Chapter 4 disc;
- the sweep ellipses;
- the second texture and its disc.

## 7. One run

```bat
build\Release\herpvs_campaign.exe -d -c -n 600 -w 3 -L 1 -x 1 -r textures\peppers.jpg -t textures\uniform_dark_disc.png -P campaign\poses.csv -p 1 -o one_run.csv
python analysis\plot_run.py one_run.csv
```

The run takes a few minutes and prints one line at the end. `plot_run.py`
then prints the final errors and writes `one_run.png`. The study, built with
GCC on Linux, gave a camera-centre error of **0.4307 mm** and a rotation
error of **0.01894°** for this run.

Visual Studio rounds floating-point operations slightly differently, so
expect agreement to three or four significant digits, not to the last digit.

| Option | Meaning |
|---|---|
| `-w 1` … `-w 4` | V1 none, V2 Tukey, V3 structure-informed, V4 structure-penalised |
| `-L 0` / `-L 1` | interaction matrix from the current image (thesis) / the desired image |
| `-p 1` … `-p 32` | initial pose from `campaign\poses.csv`; `-p 0` is the Chapter 4 pose |
| `-t FILE` | the texture seen during the servo, e.g. `textures\textured_disc.png` |
| `-n N` | number of iterations |
| `-m PREFIX` | also write weight and influence maps |

The program has no display window. Your thesis program remains the way to
watch a run.

## 8. The campaigns

```bat
python campaign\run_campaign.py validate --jobs 4
python campaign\run_campaign.py confirmatory --jobs 4
python campaign\run_campaign.py replication --jobs 4
python campaign\run_campaign.py sweep --jobs 4
```

| Design | Runs | Time with 4 parallel runs |
|---|---|---|
| validate | 6 | about 5 min |
| confirmatory | 1 024 | about 10 h |
| replication | 256 | about 2.5 h |
| sweep | 512 | about 5 h |

- **Parallel runs:** set `--jobs` to your number of physical CPU cores (Task
  Manager → Performance → CPU → Cores).
- **Progress:** each completed run prints one line.
- **Interrupting:** a campaign can be stopped with Ctrl-C and restarted with
  the same command; finished runs are skipped.
- **Long campaigns:** in Windows power settings, prevent the computer from
  sleeping while a campaign runs.

Logs go to `runs\<design>\logs\`.

## 9. Analyses and comparison with the reported results

```bat
python analysis\run_analyses.py confirmatory replication sweep
python analysis\compare_results.py results\confirmatory confirmatory
```

`run_analyses.py` applies the pre-registered analyses of
`PREREGISTRATION.md` and `PREREGISTRATION_ADDENDUM.md`. Its outputs go to
`results\<design>\`:
- `report.md`: the summary;
- `primary.csv` and `decision.txt`: the pre-registered tests and decision;
- `figures\`.

`compare_results.py` compares every run with the committed results, and
lists the runs whose success differs. Because Visual Studio rounds slightly
differently, a few runs that start close to the edge of the convergence
domain may change outcome. The statistics should agree to the precision
reported.

## 10. Figures and the article

```bat
python paper\make_figures.py
```

To compile the article, either:
- **Overleaf:** upload the folder `paper` (`main.tex`, `refs.bib`,
  `figures\`) to a new project; Overleaf has the Elsevier class; or
- **MiKTeX:** in `paper\`, run `pdflatex main`, `bibtex main`, `pdflatex main`
  and `pdflatex main`. MiKTeX installs missing packages on first use.

## 11. Problems

| Message | Remedy |
|---|---|
| CMake: `Could not find a package configuration file provided by "VISP"` | set `VISP_DIR` to the folder containing `VISPConfig.cmake` (§3) |
| CMake: generator or platform does not match | delete `ARTICLE\build`, configure again with the architecture of your ViSP build |
| Visual Studio: `unresolved external symbol` from ViSP | ViSP built for another architecture or configuration; use the same x64/Win32 and Release |
| `... .dll was not found` when running | add the ViSP and third-party `bin` folders to `PATH` (§3) |
| `program not found: ...herpvs_campaign` | build in Release (§3), or give the path: `--bin C:\path\to\herpvs_campaign.exe` |
| `missing texture for scenario ...` | run `python tools\make_textures.py` (§6) |
| `'python' is not recognized` | reinstall Python with **Add to PATH**, or use `py -3` instead of `python` |
| a compiler error in `campaign.h` or `herpvs_campaign.cpp` | send the first error message and your ViSP version (`VISP_VERSION` in `VISPConfig.cmake`) |
