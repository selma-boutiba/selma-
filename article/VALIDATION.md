# Reproduction of Chapter 4 and validation of the campaign program

Inputs received: the servoing program (uploaded as `tutorial-viewer1.cpp`; it
is the file earlier notes call `tutorial-viewer15.cpp`, the line numbers they
quote match it), the modified `vpFeatureLuminance` (`.h`, `.cpp`) and the two
target textures (clean, and with the black disc), 386 × 230 pixels.
Environment: ViSP 3.6.0, GCC, 4 cores. The textures are not committed.

## 1. Building the thesis program against ViSP 3.6

Two compatibility issues, neither touching the computation:

- ViSP 3.6 headers need C++17.
- The program calls `vpImageFilter::filter()` on `vpImage<double>` inputs with
  a single output 24 times per iteration. No ViSP release from 3.0 to 3.6 has
  that overload, so it was added to the ViSP used for the thesis. It was
  reimplemented as a correlation, like ViSP's `unsigned char` overload. The
  reproduction below confirms the choice; a convolution would flip the
  asymmetric summed kernels and change the trajectories.

The program was otherwise run unchanged, with display, click and plotting
switched off and 600 passes.

## 2. The thesis results are reproduced

| Quantity | Chapter 4 | Reproduced | Note |
|---|---|---|---|
| $N_e$, clean / occluded | $4.0367$ / $4.0588\times10^{8}$ | $4.0365$ / $4.0586\times10^{8}$ | textures and renderer agree |
| $\lVert\mathbf e\rVert^2$ at the first pass, clean | $3.48\times10^{11}$ | $3.483\times10^{11}$ | |
| V3, occluded: λ, µ at pass 600 | 2.19209, $2.36709\times10^{-9}$ | 2.19209, $2.36718\times10^{-9}$ | λ to six digits |
| V3, occluded: rotation error | 0.015187° | 0.015247° | 0.4 % |
| V1, occluded: t (µm) | (675.6, 30.8, −1102.2) | (677.3, 30.6, −1106.2) | 0.3 % |
| V1, occluded: λ | 2.33425 | 2.33495 | |
| V2, occluded: λ | 2.19257 | 2.19238 | |
| V3 summed-kernel ablation, clean: rotation error | 0.088301° | 0.088283° | |
| V3 clean, at the pass where λ equals the chapter's | (101, −84, −990) µm, (0.383, −0.030, 0.019)° | pass 582: (101.7, −84.4, −985.6) µm, (0.3815, −0.0336, 0.0191)° | see F2 |

Micrometre-level components of the occluded weighted runs differ by up to a
micrometre. This is the terminal jitter of those runs (about 0.25 µm per pass)
amplified by bit-level differences between compilers and ViSP versions.

## 3. The campaign program is equivalent to the thesis program

`cpp/herpvs_campaign.cpp` builds the features and the interaction matrix
itself, so it needs neither the modified `vpFeatureLuminance` nor a modified
`vpImageFilter`. With `-w 3 -x 0 -L 0` and `-G 0` or `-G 1`, it reproduces the
thesis program **to every printed digit**: six pose components,
$\lVert\mathbf e\rVert^2$, λ, µ and $\lVert\mathbf v_c\rVert$, on the clean and
occluded scenes, with both structure measures.

## 4. Findings that change Chapter 4

**F1. The uploaded program is in the ablation configuration.** Its structure
measure (line 860) uses the summed kernels `w_h, w_v, w_d`. The first-order
maps `g10_2 … g01_4` are computed at every iteration but not used. The
chapter's Variant 3 numbers come from the first-order (DC-free) measure, which
`-G 1` reproduces. Switch line 860 back before any further thesis run.

**F2. The clean-scene results are read about 15–18 passes early.** Every
quantity the chapter reports for the clean scene (λ, ‖e‖², all six pose
components) is matched simultaneously at pass 582–585 of the reproduction, for
V1, V2 and V3 alike, not at pass 600. With the $k^{-1/2}$ decay this changes the
errors by about 1.5 %. The occluded results are stationary and unaffected.

**F3. The occluder covers 7.5 %, not 5.8 %.** Measured from the occluder mask
(convex hull of the region where the textures differ; stable for thresholds
6–20 grey levels):

| Pose | True coverage | Where $G=0$ (eroded by 4 px) | Dilated by 4 px |
|---|---|---|---|
| desired | 7.51 % | 5.64 % | 9.57 % |
| Chapter 4 initial | 9.26 % | 7.15 % | 11.60 % |

The chapter's 5.80 % is the region where $G$ vanishes, i.e. the occluder eroded
by the kernel half-width. The 9.5 % rejected at convergence is the occluder
dilated by exactly that half-width (9.57 %). The prediction of Section 4.3.2 is
therefore confirmed, and the unexplained band of Section 4.7.3 does not exist.

**F4. The reported translation error is not the camera's position error.**
The tables give $\mathbf t-\mathbf t^{*}$ of ${}^{c}\mathbf M_o$, the target
origin seen from the camera. It barely changes when the camera rotates about
the target. The camera centre (${}^{c^*}\mathbf M_c$) is about $Z\theta$ away:

| Run (thesis conventions) | Thesis translation | Camera-centre error | $Z\theta$ |
|---|---|---|---|
| V1 clean | 0.33 mm | 4.83 mm | 4.90 mm |
| V2 clean | 0.79 mm | 13.4 mm | 13.5 mm |
| V3 clean | 0.98 mm | 8.54 mm | 8.57 mm |
| V1 occluded | 1.30 mm | 39.3 mm | 38.7 mm |
| V2 occluded | 2.7 µm | 0.37 mm | 0.37 mm |
| V3 occluded | 3.7 µm | 0.35 mm | 0.35 mm |

"Better than one millimetre" (4.7.1) and "within three micrometres" (4.7.2,
4.8) describe the target-frame quantity. Either define that quantity
explicitly wherever it is quoted, or report the camera-centre error. The
article uses the camera-centre error; both are logged.

**F5. Principal point.** All thesis programs call
`convertPoint(cam, i, j, y, x)`. ViSP's own `vpFeatureLuminance` calls
`convertPoint(cam, j, i, x, y)`. The interaction matrix was therefore built
with the principal point at (120, 160) instead of (160, 120). Correcting it
(`-x 1`), Chapter 4 pose, first-order measure, 600 passes:

| Variant, scene | Rotation error, thesis → corrected | Camera centre, thesis → corrected |
|---|---|---|
| V1 clean | 0.221° → 0.180° | 4.94 → 4.04 mm |
| V1 occluded | 1.704° → 1.505° | 39.3 → 34.8 mm |
| V2 clean | 0.609° → 0.528° | 13.7 → 11.9 mm |
| V2 occluded | 0.0162° → 0.0147° | 0.370 → 0.336 mm |
| V3 clean | 0.386° → 0.305° | 8.72 → 6.98 mm |
| V3 occluded | 0.0152° → 0.0142° | 0.347 → 0.324 mm |

The correction improves every case by 7–21 % and changes no ordering or
conclusion of the chapter. It is an erratum for Chapters 3–4; the campaign
uses the corrected matrix.

## 5. Not yet established

- Resolved: the chapter's V1 came from a program **with** the ε floor. At the
  pass where λ equals the chapter's, the clean-scene rotation error is 0.2192°
  with the floor and 0.2085° without it (chapter: 0.2186°). Under occlusion
  the floor makes no difference.
- The cause of F2 (where the consoles were read, or a small difference in the
  first passes).

## 6. Change after the study: portable build

After all campaigns, `campaign.h` replaced the call to ViSP's `vpRobust` by
`tukeyWeights()`, the same computation written out, so that the program
builds with older ViSP versions and with Visual Studio 2015 (C++14). Rebuilt
on the study's machine, the program reproduces the committed final lines of
V2, V3 and V4 runs and of all six validation runs. The trajectories agree to
ten significant digits. The only visible change is in the last printed digit
of a rotation component of about $10^{-8}$ rad in one V2 run, a last-bit
rounding difference between ViSP's compiled library and the inlined code.
