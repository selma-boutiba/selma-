# Pre-registration addendum — replication and occlusion-fraction sweep

**Status:** written on 2026-10-12 and committed before the first run of either
design. The two designs were requested by the author ("choose the second
texture and run both"), and they are the two designs that `RESULTS.md` §6
still lists as needed. `PREREGISTRATION.md` is unchanged. Nothing in this
addendum alters the confirmatory family or its decision.

Common settings are those of `PREREGISTRATION.md`:

- `cpp/herpvs_campaign.cpp` at the commit carrying this file;
- corrected principal point (`-x 1`), first-order structure measure (`-G 1`),
  floor on (`-F 1`), 600 passes;
- `campaign/poses.csv`, ids 1–32;
- outcomes: the median over the last 50 passes of the camera-centre error (mm)
  and the rotation error (deg) of ${}^{c^*}\mathbf M_c$;
- success: camera-centre error below 20 mm and rotation error below 1°.

Only V2 (Tukey) and V3 (Hermite-informed) are run. These are the two
weightings of the confirmatory primary family.

## R. Replication on a second texture

### R.1 Texture

The texture is `skimage.data.coffee()`, a photograph by Rachel Michetti
released under CC0. It is cropped to the target aspect ratio
$0.60:0.43$ and resized to $386\times277$ (Lanczos).

It differs from the thesis texture in content and statistics:

| | Thesis texture | Coffee |
|---|---|---|
| Content | faces, dark near-uniform background | man-made object, wood grain |
| Mean grey level | 60 | 102 |
| Pixels with $\lVert\nabla I\rVert<1$ grey level/px | 27 % | 19 % |
| Median $\lVert\nabla I\rVert$ | 3.5 | 3.6 |

The CC0 licence allows the texture to be archived with the code. The texture
is still not committed here, because `tools/` regenerates it.

### R.2 Occluder

The occluder is the Chapter 4 disc, transferred onto the coffee texture
through target-plane coordinates (`make_occluders.py refill --onto`) and
filled with grey level 0. The geometry on the target is identical to the
geometry in the confirmatory design. The image coverage is therefore
identical too:

| Pose | Coverage |
|---|---|
| desired | 7.54 % |
| Chapter 4 initial | 9.29 % |
| over the 32 poses | 6.0–12.3 % (median 9.3 %) |

### R.3 Runs

- **Scenes:** clean coffee (`nominal`) and `coffee_dark_disc`.
- **Design:** 32 poses × {V2, V3} × {current-image, desired-image $\mathbf L$}
  × 2 scenes = **256 runs**.
- **Command:** `REF=coffee.png TEX=<dir> run_campaign.sh replication`.

### R.4 Primary family R (Holm, α = 0.05, 2 tests)

Both tests are run with the desired-image $\mathbf L$ on `coffee_dark_disc`,
over the poses where both V2 and V3 succeed:

| | Comparison | Test | Prediction |
|---|---|---|---|
| R1a | camera-centre error, V3 vs V2 | Wilcoxon signed-rank, two-sided | V3 < V2 |
| R1b | rotation error, V3 vs V2 | same | V3 < V2 |

**Replication criterion:** R1a and R1b are both significant after Holm, with
a Hodges–Lehmann median ratio V3/V2 ≤ 0.8. This is the criterion that
decided the confirmatory P1. If it is not met, the article reports the
primary result as "not replicated on the second texture", with the ratio and
its CI.

### R.5 Secondary, Holm within the family

- V3 vs V2 accuracy in the other three cells:
  - current-image $\mathbf L$, clean;
  - current-image $\mathbf L$, disc;
  - desired-image $\mathbf L$, clean.
- Success of V3 vs V2 (exact McNemar test) in all four cells.

### R.6 Analysis command

```bash
python3 analysis/analyze.py <logs>/replication/logs --poses campaign/poses.csv \
    --out replication --primary desired:coffee_dark_disc:hermite:tukey \
    --primary-success "" --decision-labels "replicated,not replicated"
```

## S. Occlusion-fraction sweep

### S.1 Occluders

- **Texture:** the thesis texture.
- **Occluder:** a uniform dark ellipse (grey level 0) centred at
  $(0.04,-0.03)$ m on the target, with aspect ratio 1.4 and orientation 20°.
- **Size:** the semi-axes are fitted so that the coverage at the desired pose
  is $f\in\{0.15, 0.25, 0.35, 0.45\}$
  (`make_occluders.py generate --kind uniform_dark --fraction f`).

Coverage at the 32 initial poses:

| $f$ | Semi-axes (m) | Chapter 4 pose | 32 poses: min / median / max |
|---|---|---|---|
| 0.15 | 0.099 × 0.071 | 19.8 % | 16.7 / 19.6 / 24.4 % |
| 0.25 | 0.128 × 0.091 | 30.2 % | 25.7 / 31.8 / 40.8 % |
| 0.35 | 0.152 × 0.108 | 37.7 % | 33.8 / 42.4 / 57.2 % |
| 0.45 | 0.172 × 0.123 | 43.9 % | 41.2 / 51.7 / 72.9 % |

The confirmatory dark disc (7.5 %) is the fifth point of the curve and is not
re-run.

### S.2 Runs

- **Design:** 32 poses × {V2, V3} × {current-image, desired-image $\mathbf L$}
  × 4 fractions = **512 runs**.
- **Command:** `run_campaign.sh sweep`.

### S.3 Family S (Holm, α = 0.05, 24 tests)

| | Comparison, in each of the 8 cells (fraction × $\mathbf L$) | Test | Prediction |
|---|---|---|---|
| S1 | camera-centre and rotation error, V3 vs V2, poses where both succeed (16 tests) | Wilcoxon signed-rank, two-sided | V3 < V2 |
| S2 | success, V3 vs V2 (8 tests) | exact McNemar, two-sided | none (P2 was not supported) |

A test needs at least six pairs. A cell with fewer pairs where both runs
succeed has no p-value, and it counts as "not shown".

**Pre-specified statement:** "the accuracy gain of V3 holds at every
fraction" is true if every S1 test is significant after Holm with ratio
≤ 0.8. If not, the article reports the fractions at which it holds.

### S.4 Effect sizes

- **S1:** Hodges–Lehmann ratio with exact 95 % CI.
- **S2:** discordant counts, and the success rate per cell with a
  Clopper–Pearson 95 % CI.

### S.5 Exploratory, no test

- The breakdown fraction of each weighting × $\mathbf L$: the smallest $f$
  at which the success rate falls below 50 %.
- Success against the per-pose initial coverage.
- The Tukey scale estimate and the rejected fraction at the first pass,
  against coverage.

### S.6 Analysis command

```bash
C=uniform_dark_f150,uniform_dark_f250,uniform_dark_f350,uniform_dark_f450
P=$(for L in current desired; do for s in ${C//,/ }; do printf "%s:%s:hermite:tukey," $L $s; done; done)
python3 analysis/analyze.py <logs>/sweep/logs --poses campaign/poses.csv --out sweep \
    --primary "$P" --primary-success "$P" \
    --decision-labels "gain at every fraction,gain not at every fraction"
```

## Compute

At about 160 s per run with four runs in parallel, the replication takes about
3 h and the sweep about 6 h. The replication runs first.
