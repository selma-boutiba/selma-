# Your V3 program, prepared for the article

| File | What it is |
|---|---|
| `tutorial-viewer4.cpp` | your program, unchanged |
| `tutorial-viewer4-article.cpp` | the same program with the changes the article needs, each marked `[ARTICLE-n]` |
| `tutorial-viewer4-article.diff` | every changed line, before and after |
| `CMakeLists.txt` | your CMake file, for the new program |

The article version keeps your code, your display and your plot. It uses the
modified `vpFeatureLuminance` already in your ViSP.

**Check of the results.** Built here against your modified `vpFeatureLuminance`:
- with the thesis settings (`-x 0 -G 0 -L 0 -w 3`), it gives the same final
  pose as the campaign program;
- with the article settings, it reproduces runs of the article to every
  printed digit.

## Why the program must change

| | In your file | What the article needs | Option |
|---|---|---|---|
| 1 | only Variant 3 | four weightings: V1 none, V2 Tukey, V3 Hermite-informed, V4 structure-penalised | `-w 1` … `-w 4` |
| 2 | interaction matrix from the current image (line 572, `sI.interaction(Lsd)`) | also from the desired image, computed once (`sId.interaction`) | `-L 0` / `-L 1` |
| 3 | structure measure from `w_h, w_v, w_d` (line 860, summed kernels) | first-order kernels `g10_k, g01_k`, as Chapter 4 defines `G` (your file computes them but does not use them) | `-G 1` |
| 4 | `convertPoint(cam, i, j, y, x)` (lines 750, 790): the principal point becomes (120, 160) instead of (160, 120) | `convertPoint(cam, j, i, x, y)` | `-x 1` |
| 5 | one initial pose (line 326) | 32 initial poses from `campaign/poses.csv` | `-P`, `-p 1` … `-p 32` |
| 6 | fixed textures `peppers.jpg`, `peppers_o_b.jpg` | the other occluders and the second texture | `-r`, `-t` |
| 7 | the error printed is `t(cMo) − t(cdMo)`, the target seen from the camera | the position error of the camera itself, ${}^{c^*}\mathbf M_c$, saved at every iteration | `-o file.csv` |
| 8 | `init()` ignores `-n`, `-d`, `-c` (it uses its own values) and `-i` is required | the options work; `-i` is not needed | `-n 600`, `-d`, `-c` |

Default settings:
- **Article settings** (`-x 1 -G 1`) are the default.
- **Thesis behaviour:** `-x 0 -G 0` gives exactly what your file computes.

## Build in Visual Studio 2015

1. In CMake GUI, set:
   - **source:** `article/thesis_program`
   - **build:** `article/thesis_program/build`
2. Click **Configure**. Choose Visual Studio 14 2015 with the architecture of
   your ViSP (Win64 or not). If CMake does not find ViSP, set `VISP_DIR` to your
   ViSP build folder. Then click **Generate** and **Open Project**.
3. In Visual Studio, select **Release**, then **Build Solution**.
4. Running from Visual Studio: right-click `tutorial-viewer4-article` →
   **Set as StartUp Project**. Then open **Properties → Debugging** and set:
   - **Working Directory:** the `article` folder;
   - **Command Arguments:** one of the lines below.

## The runs of the article

Run from the `article` folder. The textures come from
`python tools\make_textures.py` (see `RUN_ON_WINDOWS.md` §6).

**One run, with display and plot.** V3, desired-image interaction matrix,
dark disc, pose 1:

```
-w 3 -L 1 -x 1 -G 1 -r textures\peppers.jpg -t textures\uniform_dark_disc.png -P campaign\poses.csv -p 1
```

**The same run without display, writing the log:**

```
-w 3 -L 1 -x 1 -G 1 -r textures\peppers.jpg -t textures\uniform_dark_disc.png -P campaign\poses.csv -p 1 -d -c -o run.csv
```

The last line printed must be close to:

```
### final pass 600  errorpose(m, rad) 4.16788e-07 9.00742e-07 -1.53515e-05 0.000299727 0.000133889 -3.75872e-06  |e|^2 1.59696e+10  lambda 2.5175  mu 5.69713e-09  |Tc| 3.7993e-06
```

`python analysis\plot_run.py run.csv` gives the camera's final error: 0.4307 mm
and 0.01894°.

**What to change for each part of the article:**

| Part of the article | `-w` | `-L` | `-t` (servo texture) | `-r` | poses |
|---|---|---|---|---|---|
| Confirmatory campaign | 1, 2, 3, 4 | 0, 1 | `peppers.jpg` (clean), `uniform_dark_disc.png`, `textured_disc.png`, `low_contrast_disc.png` | `peppers.jpg` | 1–32 |
| Replication | 2, 3 | 0, 1 | `coffee.png` (clean), `coffee_dark_disc.png` | `coffee.png` | 1–32 |
| Occlusion sweep | 2, 3 | 0, 1 | `uniform_dark_f150.png`, `f250`, `f350`, `f450` | `peppers.jpg` | 1–32 |

That is 1 024 + 256 + 512 runs, too many to start by hand. The campaign
script starts them all with your program, without display, and can be
stopped and resumed:

```
python campaign\run_campaign.py confirmatory --jobs 4 --bin thesis_program\build\Release\tutorial-viewer4-article.exe
python campaign\run_campaign.py replication --jobs 4 --bin thesis_program\build\Release\tutorial-viewer4-article.exe
python campaign\run_campaign.py sweep --jobs 4 --bin thesis_program\build\Release\tutorial-viewer4-article.exe
python analysis\run_analyses.py confirmatory replication sweep
```

The script gives your program the options above, `-d -c -n 600` and a log
file per run. The analysis then produces the tables and figures of the
article in `results\`.

## The other program, `cpp/herpvs_campaign.cpp`

It is the same computation as `tutorial-viewer4-article.cpp`, without display
and plot, and without the modified `vpFeatureLuminance`. It was used to run
the 1 792 runs of the article because it builds against any ViSP. You can
use either program; they give the same results.
