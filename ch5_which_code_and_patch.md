# Which file, and the edits it needs

## The answer: `tutorial-viewer15.cpp`

`viewer15` is Variant 3 and `viewer14` is Variant 1. Four reasons it has to be
viewer15, in order of importance:

1. **It is Chapter 4's contribution.** Chapter 5 adds the gain layer on top of
   the weighting layer, with **D** held fixed. Building on viewer14 would mean
   abandoning your own Chapter 4 in the chapter that follows it.
2. **It is the code that produced Chapter 4's numbers**, so Arm A can be
   validated against them: same pose, same texture, `-g 0` must reproduce
   0.999 mm / 0.385° exactly.
3. **It has the ε floor** (`muFloor = 1e-6*meanDiag`), which §4.2.2 and §4.5
   both rely on. viewer14 does not — its line is plain
   `((mu*diagHsd) + Hsd).inverseByLU()`.
4. **Its texture wiring is correct and viewer14's is not.** See below.

## The texture wiring — viewer15 is right, viewer14 is wrong

This resolves something I raised repeatedly and then retracted. The two files
genuinely differ:

**viewer15 — correct.** `sim.init(Itexture, X)` at line 266, before `cdMo`, so
the *desired* image comes from the clean `peppers.jpg`; then
`sim.init(Itextured, X)` at line 357, before `cMo`, so the images acquired
during servoing come from the occluded `peppers_o_b.jpg`. Reference clean,
servoing occluded. That is the right way round, and it is why your screenshots
showed a clean s\*.

**viewer14 — inverted.** `sim.init(Itextured, X)` at line 245 before `cdMo`, so
the *desired* image is the occluded one, and `sim.init(Itexture, X)` at line 333
before `cMo`, so the current image is clean. If you ever run viewer14 for an
occlusion result, swap those two.

## A correction: λ and µ are **not** infinite at the first iterations

I told you `Ne` is captured too late and that the gains evaluate to +∞ for the
first two iterations. **That was wrong**, and the reason is the post-increment
in the print:

```cpp
int iter = 1;
do {
    std::cout << "----" << iter++ << std::endl;   // prints 1, iter becomes 2
    ...
    if (iter == 2) { Ne = normeErrorI; }          // TRUE on the first pass
```

`iter` is already 2 by the time the guard is reached, so `Ne` is captured on the
**first** pass, before λ is ever computed. Nothing is infinite and there is
nothing to fix. It also confirms §4.7.1's figure: on that first pass
`normeErrorI == Ne`, so λ = N_ei^0.2 = 52.69, which is the "λ ≃ 53 at the first
iteration" the thesis quotes.

---

# The edits

## 1. The command line is being thrown away — fix this first

`init()` takes no arguments and re-declares all three options as locals:

```cpp
void init() {
    bool opt_click_allowed = true;   // line 248  <-- shadows main's parsed value
    bool opt_display      = true;    // line 249  <-- ditto
    int  opt_niter        = 1000;    // line 250  <-- ditto
```

So `main` parses `-d` and `-c` and then discards them. **Display is always on
and the program always waits for a mouse click.** For a 90-run campaign both are
fatal: the display is what makes an iteration take 8–11 ms instead of ~0.4 ms,
and the click blocks an unattended batch on the first run.

Also worth knowing: the effective iteration cap is **1000**, not 600 — §4.6.3
should say results are read at iteration 600 under a cap of 1000.

Replace the three lines with parameters:

```cpp
void init(bool opt_click_allowed, bool opt_display, int opt_niter,
          GainMode gainMode, int poseIdx, int scenario);
```

and in `main`, after the option parsing:

```cpp
init(opt_click_allowed, opt_display, opt_niter, gainMode, poseIdx, scenario);
```

## 2. Add the three new flags

```cpp
#define GETOPTARGS  "cdi:n:g:p:s:h"
```

and in the `switch`:

```cpp
case 'g': gainMode = (GainMode)atoi(optarg_); break;   // 0 fixed, 1 floored, 2 fuzzy
case 'p': poseIdx  = atoi(optarg_);           break;   // 0..10
case 's': scenario = atoi(optarg_);           break;   // 0 nominal, 1 occluded, 2 illumination
```

declared in `main` alongside the others:

```cpp
GainMode gainMode = GAIN_FIXED;
int poseIdx = 0;
int scenario = 0;
```

## 3. The poses, as an array instead of eleven commented lines

Your ten poses are already in the file as comments, and Chapter 4's is the
active one. Put all eleven in a table so the campaign is a shell loop rather
than eleven recompiles. **Pose 0 is Chapter 4's**, which makes it the
reproducibility check:

```cpp
// x, y, z in metres; rx, ry, rz in degrees
static const double POSES[11][6] = {
    { 0.04, -0.04, 1.10,  12, -12,  6 },   // 0 : Chapter 4's pose
    { 0.27, -0.25, 3.20,  31, -28,  6 },   // 1
    {-0.24,  0.26, 3.18, -29,  30, -7 },   // 2
    { 0.25,  0.28, 3.22,  34, -30,  5 },   // 3
    {-0.26, -0.22, 3.15,  36,  27, -6 },   // 4
    { 0.29, -0.23, 3.19, -33, -29,  9 },   // 5
    {-0.28,  0.25, 3.20,  32,  30, -4 },   // 6
    { 0.30,  0.22, 3.21,  35, -32,  8 },   // 7
    {-0.25, -0.28, 3.16, -34,  31, -9 },   // 8
    { 0.23, -0.30, 3.17,  33, -31,  6 },   // 9
    {-0.29,  0.27, 3.18,  36,  30, -7 }    // 10
};
```

replacing the block at lines 326–346:

```cpp
if (poseIdx < 0 || poseIdx > 10) poseIdx = 0;
const double *P = POSES[poseIdx];
cMo.buildFrom(P[0], P[1], P[2],
              vpMath::rad(P[3]), vpMath::rad(P[4]), vpMath::rad(P[5]));
```

Note that pose 0 sits at Z = 1.10 m and the other ten at Z ≈ 3.2 m — a
displacement roughly ten times larger. **Arm A's baseline must be re-measured
at poses 1–10**; Chapter 4's 0.999 mm does not transfer to them. Either treat
pose 0 as the validation case and report the campaign over 1–10, or say in
§5.6.2 that the eleven span two displacement magnitudes deliberately.

## 4. The three scenarios

The clean choice for illumination is **not** a third JPEG. Generate it in code
from the same texture: then the scenario is fully specified in the thesis by two
numbers, and it is exactly reproducible.

Keep the reference image clean in all three scenarios, and change only what is
acquired during servoing:

```cpp
// --- reference: always the clean texture (as viewer15 already does) ---
sim.init(Itexture, X);
sim.setCameraPosition(cdMo);
sim.getImage(I, cam);
Id = I;
Idhermite();

// --- servoing: the texture depends on the scenario ---
if (scenario == 1) sim.init(Itextured, X);   // occlusion
else               sim.init(Itexture,  X);   // nominal and illumination
```

and for illumination, apply the photometric change to the acquired image, in
both places `sim.getImage(I, cam)` appears for the current view:

```cpp
static const double ILLUM_A = 0.75;   // contrast
static const double ILLUM_B = 30.0;   // brightness offset, grey levels

inline void applyIllumination(vpImage<unsigned char> &Im) {
    for (unsigned int i = 0; i < Im.getHeight(); i++)
        for (unsigned int j = 0; j < Im.getWidth(); j++) {
            double v = ILLUM_A * (double)Im[i][j] + ILLUM_B;
            Im[i][j] = (unsigned char)(v < 0 ? 0 : (v > 255 ? 255 : v));
        }
}
```

called right after each `sim.getImage(I, cam)` when `scenario == 2`. State
*a* = 0.75 and *b* = 30 in §5.6.3 and the scenario is reproducible by anyone.

**Run this once before designing §5.7.2**, and print both errors at
convergence:

```cpp
std::cout << "### normeError = " << normeError
          << "   normeErrorI = " << normeErrorI << std::endl;
```

If `normeErrorI` floors high, λ is pinned high and the illumination scenario
tests overshoot — which is the case where a floor cannot help and only the
scheduler can. If it floors low, illumination just re-tests the stall. That one
run decides what §5.7.2 is about.

## 5. The gains — the actual change

At the top:

```cpp
#include "fuzzy_gain_scheduler.h"
```

Before the loop, beside `mu` and `lambda`:

```cpp
FuzzyGainScheduler fuzzy;
```

And replace lines 613–614:

```cpp
// lambda = pow(10, (log10(normeErrorI) - 0.8*log10(Ne)));
// mu     = pow(10, (5 * log10(normeErrorI) - 5.2*log10(Ne)));
computeGains(gainMode, normeErrorI, Ne, fuzzy, lambda, mu);
```

That is the whole of it. Everything else — `Ihermite()`, the six first-order
kernels, `computeHermiteNormalizedResidual`, the Tukey call, `Lp`, `error_p`,
the ε floor — is untouched, which is what makes `gainMode` the only variable.

Add the scheduler's state to the log line you already have, for §5.7:

```cpp
std::cout << "lambda = " << lambda << "  mu = " << mu
          << "  ebar = " << (Ne > 0 ? normeErrorI/Ne : 1.0)
          << "  trend = " << fuzzy.trend()
          << "  |Tc| = " << sqrt(v.sumSquare()) << std::endl;
```

## 6. Turn off the map dumping for the campaign

```cpp
if (iter == 2 || iter == 601) { saveMaps(w, structG, iter - 1, "v3"); }
```

Ninety runs would write 180 PGM files, most of them identical in purpose. Gate
it on a flag, or on `poseIdx == 0 && gainMode == GAIN_FIXED`, so you keep
Chapter 4's maps and nothing else.

---

# Running the campaign

Once the flags are in, the whole thing is a loop:

```bash
for s in 0 1 2; do          # nominal, occluded, illumination
  for g in 0 1 2; do        # fixed, floored, fuzzy
    for p in 1 2 3 4 5 6 7 8 9 10; do
      ./viewer15 -d -c -n 600 -s $s -g $g -p $p > log_s${s}_g${g}_p${p}.txt
    done
  done
done
```

90 runs, and with the display genuinely off it should be hours rather than days.

## The order

1. **Fix the shadowed options** (§1). Without this, nothing else is runnable
   unattended.
2. **`-g 0 -p 0 -s 1`** and check it reproduces Chapter 4's Variant 3 occluded
   numbers — 3.156 µm, 0.015187°, λ = 2.19209, µ = 2.36709e-9. This proves the
   refactor changed nothing.
3. **The illumination probe** (§4), one run, to see where `normeErrorI` floors.
4. **`-g 1 -p 0 -s 0`** — Arm A′ nominal. The experiment that decides the shape
   of the chapter.
5. **The λ_min sweep**, six runs.
6. **The 90-run campaign.**

Steps 2 and 4 are the ones worth doing tonight. Step 2 protects you against a
refactoring error propagating through ninety runs; step 4 tells you what the
chapter is about before you write any of it.
