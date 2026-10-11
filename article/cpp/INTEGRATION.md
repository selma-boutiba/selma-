# Integrating `campaign.h` into `tutorial-viewer15.cpp`

`viewer15` is not in the repository, so the insertion points below are given
against the structure shared by every program on this branch (`Ihermite()`,
`Idhermite()`, `init()`, the `do { … } while` servo loop) and against the
Chapter 5 edits in `ch5_which_code_and_patch.md`. Upload `viewer15` and the
customised `vpFeatureLuminance` to get an exact patch instead.

`campaign.h` is header-only and depends on ViSP core. It was compiled and
tested against ViSP 3.6.0 (`test_campaign.cpp`: 23 checks).

## Order of work — do not reorder

| Step | Change | Check before continuing |
|---|---|---|
| 0 | none | Record the console of `viewer15` at pose 0, occluded, Variant 3: 3.156 µm, 0.015187°, λ = 2.19209, µ = 2.36709e-9 (Table 4.2) |
| 1 | Edits 1–7 below with `-x 0` (thesis pixel order) | `run_campaign.sh validate` reproduces Tables 4.1 and 4.2 to the printed digits for V1–V3 |
| 2 | Run pose 0 again with `-x 1` (corrected principal point) | Record the change; it is an erratum for Chapters 3–4 |
| 3 | `run_campaign.sh pilot` | All 160 runs complete; read `t_iter_ms` to size the campaign |

If step 1 does not reproduce the tables, stop: the campaign would measure a
different program from the one the thesis describes.

## Edit 1 — options reach `init()`

As in `ch5_which_code_and_patch.md` §1: delete the three locals in `init()`
that shadow `opt_click_allowed`, `opt_display` and `opt_niter`, and pass them
as arguments. Add one more argument, `const campaign::Options &opts`.

Guard every display **and every `vpPlot` call** with `opt_display`.
`init_visp_plot()` opens a window unconditionally; on a machine without a
display the batch would fail at the first run.

## Edit 2 — flags

```cpp
#include "campaign.h"
campaign::Options g_opts;                 // global, beside the other globals

#define GETOPTARGS "cdi:n:g:p:s:w:L:r:t:P:o:m:x:h"
```

In the option switch, before `default:`:

```cpp
case 'p': g_opts.poseId = atoi(optarg_); break;   // replaces Chapter 5's -p
default:
  if (campaign::parseOption(c, optarg_, g_opts)) break;
  usage(argv[0], optarg_, ipath, opt_niter); return false;
```

## Edit 3 — textures and pose

Replace the two hard-coded `vpImageIo::read` calls with
`g_opts.refTexture` (reference image) and `g_opts.servoTexture` (images
acquired during servoing), keeping viewer15's wiring: `sim.init(Iref, X)`
before acquiring `Id` at `cdMo`, `sim.init(Iservo, X)` before the loop.

Replace the pose block with

```cpp
if (!campaign::loadPose(g_opts.poseFile, g_opts.poseId, cMo)) {
  std::cerr << "pose " << g_opts.poseId << " not found" << std::endl; exit(EXIT_FAILURE);
}
```

`-p 0` without `-P` is the Chapter 4 pose.

## Edit 4 — principal point (in `Ihermite()` **and** `Idhermite()`)

```cpp
// was: vpPixelMeterConversion::convertPoint(cam, i, j, y, x);
campaign::pixelToMeter(cam, i, j, g_opts.pixelOrder, x, y);
```

ViSP's `convertPoint` takes (column, row); the thesis call passes (row,
column), so the interaction matrix is evaluated with the principal point at
(120, 160) instead of (160, 120), i.e. normalised coordinates offset by
±40/870 = ±0.046. `-x 0` keeps the thesis behaviour for the reproduction
check; `-x 1` (default) is correct.

## Edit 5 — interaction matrix

Before the loop, after `sId` has been built from the desired image:

```cpp
vpMatrix Ldes;
sId.interaction(Ldes);                        // L at the desired image, fixed
```

In the loop, replace `sI.interaction(Lsd);` with

```cpp
if (g_opts.interaction == campaign::L_DESIRED) Lsd = Ldes;
else sI.interaction(Lsd);
```

## Edit 6 — weights

Before the loop:

```cpp
campaign::Weigher weigher(g_opts);
campaign::Logger logger;
logger.open(g_opts.logPath, g_opts);
const unsigned int hIn = 240 - 2 * bord, wIn = 320 - 2 * bord;   // 220 x 300
```

The structure measure must be available as the N = 66 000 interior values,
in the same row-major order as the feature vector. Split viewer15's
`computeHermiteNormalizedResidual()` so that the part computing `structG`
from the six first-order maps (`g10_2 … g01_4`, the DC-free gate) is a
function of its own:

```cpp
void computeStructG(vpColVector &structG) {
  structG.resize(hIn * wIn);
  unsigned int k = 0;
  for (unsigned int i = bord; i < 240 - bord; i++)
    for (unsigned int j = bord; j < 320 - bord; j++, k++)
      structG[k] = std::sqrt(g10_2[i][j]*g10_2[i][j] + g01_2[i][j]*g01_2[i][j]
                           + g10_3[i][j]*g10_3[i][j] + g01_3[i][j]*g01_3[i][j]
                           + g10_4[i][j]*g10_4[i][j] + g01_4[i][j]*g01_4[i][j]);
}
```

and replace the normalisation and the `robust.MEstimator` call with

```cpp
campaign::Timer tw;
if (g_opts.weighting >= campaign::W_HERMITE) computeStructG(structG);
weigher.compute(error, structG, w);            // V1-V5 from -w
double tWeight = tw.ms();
```

`Lp`, `error_p`, `Hsd`, the floor, λ, µ and the velocity are unchanged.
With `-w 3` the weigher computes exactly what the chapter's Variant 3 does —
G over the N positions, mean over N, floor 0.1, one factor per position
shared by its four measurements, Tukey with the scale of the normalised
residual — which is what step 1 verifies.

## Edit 7 — log and maps

At the top of the loop body: `campaign::Timer ti;`. After `cMo` has been
updated:

```cpp
campaign::IterationRecord rec;
rec.iter = iter - 1;
rec.poseErr = campaign::poseError(cdMo, cMo);
rec.v = v;
rec.e2 = normeError;  rec.eI2 = normeErrorI;
rec.lambda = lambda;  rec.mu = mu;
rec.scale = weigher.scale();  rec.gamma = weigher.gamma();
logger.weightStats(w, Hsd, 4, rec);           // Hsd = (W L)^T (W L) = H_W
rec.tWeightMs = tWeight;  rec.tIterMs = ti.ms();
logger.write(rec);

if (!g_opts.mapPrefix.empty() && (iter - 1 == 1 || iter - 1 == opt_niter - 1))
  campaign::saveMaps(g_opts.mapPrefix, iter - 1, w, Lp, error_p, hIn, wIn);
```

`poseError` returns the relative pose ${}^{c^*}\mathbf{M}_c$; the thesis
tables use the component-wise difference of two pose vectors, which agrees
with it to first order (with the opposite sign) and is not itself a pose.

Disable `saveMaps()` of Chapter 4 and the console printing of every
iteration for the campaign; the log carries everything.

## Build

```cmake
find_package(VISP REQUIRED)
include_directories(${VISP_INCLUDE_DIRS} path/to/article/cpp)
add_executable(viewer15 tutorial-viewer15.cpp)
target_link_libraries(viewer15 ${VISP_LIBRARIES})
```

Build in Release (`-DCMAKE_BUILD_TYPE=Release`): the convolutions dominate
the iteration time.
