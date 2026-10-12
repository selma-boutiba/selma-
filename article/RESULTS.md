# Confirmatory campaign — results

Pre-registration: `PREREGISTRATION.md`, committed at `07c1743`
(2026-10-11 18:16 UTC) before the first run. Runs: 1 024 = 32 poses × V1–V4 ×
2 interaction matrices × 4 scenes, all complete (600 passes each), all with the
pre-registered settings (corrected principal point, first-order structure
measure, floor on). Analysis: the pre-registered command, unchanged. Outputs in
`confirmatory/`.

## 1. Primary family (Holm over 6 tests)

| Test | Comparison | Result | p (Holm) | Verdict |
|---|---|---|---|---|
| P1a | V3/V2 camera-centre error, desired-image L, dark disc | **0.658** [0.600, 0.732], n = 29 | $4.2\times10^{-7}$ | supported |
| P1b | V3/V2 rotation error, same | **0.657** [0.600, 0.732], n = 29 | $4.2\times10^{-7}$ | supported |
| P2a | success V3 vs V2, current-image L, clean | V3-only 2, V2-only 5 | 1 | not supported |
| P2b | same, dark disc | V3-only 2, V2-only 3 | 1 | not supported |
| P2c | same, textured disc | V3-only 2, V2-only 3 | 1 | not supported |
| P2d | same, low-contrast disc | V3-only 1, V2-only 6 | 0.50 | not supported |

**Decision (pre-registered rule): method paper.** Both P1 tests are significant
with a median ratio of 0.66, below the 0.8 threshold. The convergence
hypothesis P2 is not supported.

## 2. Accuracy — secondary

Ratio V3/V2 (Hodges–Lehmann, exact 95 % CI) over the poses where both runs
succeed; camera-centre error (rotation error agrees to two decimals):

| L from | Clean | Dark disc | Textured disc | Low-contrast disc |
|---|---|---|---|---|
| current image | 0.74 [0.66, 0.81], n = 24 | 0.63 [0.54, 0.73], n = 25 | 0.62 [0.53, 0.72], n = 25 | 0.42 [0.33, 0.53], n = 25 |
| desired image | 0.69 [0.61, 0.79], n = 29 | 0.66 [0.60, 0.73], n = 29 (P1a) | 0.64 [0.58, 0.72], n = 29 | 0.40 [0.34, 0.46], n = 29 |

Median final errors of successful runs (camera centre / rotation):

| L from | Scene | V1 | V2 | V3 | V4 |
|---|---|---|---|---|---|
| current | clean | 5.36 mm / 0.238° | 12.4 mm / 0.542° | 7.99 mm / 0.351° | 10.3 mm / 0.451° |
| current | dark disc | biased (≈ 35 mm) | 0.447 mm / 0.0197° | **0.327 mm / 0.0144°** | 0.387 mm / 0.0170° |
| desired | clean | 5.22 mm / 0.230° | 13.4 mm / 0.590° | 8.81 mm / 0.389° | 12.0 mm / 0.527° |
| desired | dark disc | biased (≈ 31 mm) | 0.489 mm / 0.0215° | **0.321 mm / 0.0141°** | 0.443 mm / 0.0194° |

- **Robust weighting under occlusion (H1):** V2/V1 = 0.015 (dark and textured
  discs) and 0.06 (low contrast). The unweighted scheme converges to a biased
  pose 31–35 mm from the target.
- **Cost of robustness on the clean scene:** V2/V1 = 2.4–2.5. V3 recovers about
  a third of that cost; V1 remains the most accurate when nothing is occluded.
- **V4 against V2:** 0.90–0.93, equivalent within ±25 %. V3 against V4:
  0.40–0.74.

## 3. Convergence — secondary and exploratory

Basin failures (final rotation > 5°) out of 32 poses:

| L from | V1 | V2 | V3 | V4 |
|---|---|---|---|---|
| current image, per scene | 0–2 | 1–4 | 5–6 | 6–9 |
| desired image, per scene | 0–1 | 0 | 3 (poses 13, 24, 27) | 0 |

By displacement bin, with the current-image matrix (32 runs per bin, all
scenes pooled), failures of V1 / V2 / V3 / V4 are:

| Bin | V1 | V2 | V3 | V4 |
|---|---|---|---|---|
| 0 (20–32 px) | 0 | 0 | 0 | 0 |
| 1 (32–44 px) | 0 | 0 | 0 | 12 |
| 2 (44–56 px) | 0 | 4 | 3 | 15 |
| 3 (56–68 px) | 2 | 6 | **19** | 1 |

V3's failures concentrate at the largest displacements, V4's at intermediate
ones.

## 4. Other measurements (exploratory)

- **Stability margin (Proposition 2):** $\min\lambda_{\min}(\mathbf H_{\mathbf W})/(\operatorname{tr}\mathbf H_{\mathbf W}/6)=5.8\times10^{-6}$
  over 1 024 runs, above $10^{-6}$. No failure is a loss of rank.
- **Clean-scene decay:** median tail slopes −0.65 (V1, V3) and −0.87 to −0.92
  (V2, V4). The decay is algebraic, not geometric.
- **Cost against accuracy:** on the clean scene the final cost ranks accuracy
  (Kendall τ 0.83–1.0). Under occlusion with the desired-image matrix it ranks
  it inversely (τ −0.33 to −0.67, negative at 72–84 % of poses).
- **Weight computation per pass:** V2 10.5 ms, V3 12.7 ms, V4 11.7 ms.

## 5. What these results correct

1. **Item A2 of `ch4_improvement_proposal.md` overreached.** Its algebra
   stands: measurements in a region that is uniform in the current image have
   zero interaction rows and their weights do not enter the control law. Its
   conclusion, that V3 therefore cannot outperform V2 on a uniform occluder with
   the current-image matrix, is refuted: V3/V2 = 0.63 [0.54, 0.73] there. V3's
   gain comes through the textured measurements, which is why it also appears
   on the clean scene (0.69–0.74). Do not adopt A2's "the tie is structurally
   predicted" or the drop-in text F4 into the thesis.
2. **The pilot overstated the convergence effect.** Two failures in four poses
   for V3, and none for V4, became 5–6 of 32 for V3 and 6–9 of 32 for V4. Four
   development poses were too few to estimate failure rates, which is why the
   pilot was not used for inference.
3. **Chapter 4's tie between V2 and V3 (§4.7.2) is a single-pose result.**
   Over 32 poses, in the chapter's own configuration (current-image matrix,
   dark disc), V3 reduces the camera-centre and rotation error by about 37 % when
   both converge, at the price of more failures at large displacement.

## 6. Replication on a second texture

**Pre-registration.** `PREREGISTRATION_ADDENDUM.md` §R, committed at `418f24c`
before the first run.

**Runs.** 256 runs, all complete: 32 poses × V2, V3 × two interaction matrices
× two scenes (clean coffee, and coffee with the Chapter 4 disc filled dark).

**Analysis.** The pre-registered command; outputs in `replication/`.

### 6.1 Pre-registered result

| Test | Comparison | Result | Verdict |
|---|---|---|---|
| R1a | V3/V2 camera-centre error, desired-image $\mathbf L$, coffee disc | no pose where both succeed (V2: 0 of 32) | not computable |
| R1b | V3/V2 rotation error, same cell | same | not computable |

**Pre-registered verdict: not replicated.** The criterion needs both tests to be
significant. Neither can be computed, because V2 fails at every pose. V2 is the
scheme that fails, not V3.

### 6.2 Secondary (pre-registered, Holm within the family)

Success out of 32 poses:

| $\mathbf L$ from | Scene | V2 | V3 | V3-only / V2-only | p (Holm) |
|---|---|---|---|---|---|
| current image | clean | 32 | 30 | 0 / 2 | 0.50 |
| current image | disc | **0** | **24** | 24 / 0 | $3.6\times10^{-7}$ |
| desired image | clean | 31 | 24 | 1 / 8 | 0.078 |
| desired image | disc | **0** | **28** | 28 / 0 | $3.0\times10^{-8}$ |

Final errors, as camera-centre error / rotation error. V2 is the median over
all 32 poses; V3 is the median over its successful runs:

| $\mathbf L$ from | Scene | V2 | V3 |
|---|---|---|---|
| current image | clean | 10.0 mm / 0.440° | 8.57 mm / 0.377° |
| current image | disc | 128.1 mm / 5.62° (every pose) | 6.63 mm / 0.298° |
| desired image | clean | 9.97 mm / 0.438° | 8.71 mm / 0.383° |
| desired image | disc | 174.8 mm / 7.69° (every pose) | 8.36 mm / 0.363° |

**Clean-scene accuracy where both succeed (V3/V2).**

| $\mathbf L$ from | Ratio [95 % CI] | n | p (Holm) |
|---|---|---|---|
| current image | 0.79 [0.64, 0.94] | 30 | 0.30 |
| desired image | 0.94 [0.79, 1.09] | 23 | 0.30 |

On the thesis texture the same ratios were 0.69–0.74.

**V3's failures.** All have a rotation error of 4–54°, except one near miss
at 23 mm / 1.01°.
- Current-image $\mathbf L$: 8 with the disc and 2 on the clean scene, all in
  bins 2–3.
- Desired-image $\mathbf L$: 4 with the disc and 8 on the clean scene, in
  bins 1–3.

The rank margin stays above $5.9\times10^{-6}$ in every run.

### 6.3 Why V2 fails (exploratory, not pre-registered)

Seven additional runs were made on coffee with the disc
(`replication/diagnostics/`). The maps come from `analysis/diag_maps.py`.

1. **It is an equilibrium, not a basin failure.**
   - V2 ends at the same pose from every initial pose. The interquartile range
     of the camera-centre error is 127.7–128.3 mm with the current-image
     matrix and 174.7–174.9 mm with the desired-image matrix.
   - From pose 1, V2 comes within 40–42 mm of the target at pass 145 and then
     moves away.
   - V3's successful runs also share a single final pose, with an
     interquartile range below 0.01 mm. That pose is 6.6 mm (current) or
     8.4 mm (desired) from the target.
2. **The offset lies along the weakly observable direction.**
   - With the desired-image matrix, V2's offset is
     $\Delta\mathbf t=(-19.6,\,171.3,\,28.5)$ mm and
     $\theta\mathbf u=(7.63,\,0.88,\,-0.10)$°. With the current-image matrix
     it is $(-6.4,\,126.6,\,17.8)$ mm and $(5.61,\,0.29,\,-0.12)$°.
   - In both cases this is a translation along $y$ coupled with a rotation
     about $x$, with $\Delta t_y\approx Z^{*}\theta_x$
     ($1.3\ \text{m}\times0.133\ \text{rad}=173$ mm).
   - Along this direction the image of a fronto-parallel plane changes only
     at second order.
   - The thesis's target-frame metric reports 16.9 mm for this pose (F4),
     against a camera-centre error of 175 mm.
3. **The desired pose is not an equilibrium under this occlusion.**
   - Started at the desired pose with the desired-image matrix, all schemes
     drift along the same direction.
   - After 600 passes, V1 is 35 mm away and still moving.
   - V2 is 8.5 mm away, and its drift is still speeding up
     (0.49 mm per 50 passes at the end).
   - V3 is 6.0 mm away and slowing down, towards its 8.4 mm equilibrium.
4. **Tukey weighting does worse than no weighting here.** From pose 1 with the
   desired-image matrix:

   | Weighting | Camera-centre error | Rotation error | Final cost |
   |---|---|---|---|
   | V1 | 78 mm | 3.4° | $4.02\times10^{10}$ |
   | V2 | 175 mm | 7.7° | $4.62\times10^{10}$ |
   | V3 | 8.4 mm | 0.36° | $3.98\times10^{10}$ |

   V2's equilibrium is not a lower-cost pose. It is produced by the weights.
5. **Mechanism** (`replication/figures/diag_maps_pose1.pdf`, desired-image
   matrix, final pass).
   - At V2's final pose, Tukey rejects most of the disc but keeps a curved band
     inside it. In that band the hidden texture, the shadowed cup handle, is
     itself dark, so the residual stays inside Tukey's acceptance region.
   - With the desired-image matrix these measurements have non-zero
     interaction rows. They carry the influence that remains inside the
     occluder.
   - V3 rejects the whole disc. Its structure measure is computed from the
     current image, which is uniform inside the disc, so any non-zero residual
     there is divided by the floor $G_{\min}$ and rejected.
   - V3 therefore detects the occluder by the absence of structure, whether or
     not its grey level happens to match the hidden texture.

**Not established.**
- With the current-image matrix, the interior of the uniform disc has zero
  interaction rows (Proposition 1). V2's equilibrium in that case (128 mm, same
  direction) must therefore come from the occluder's boundary band. No maps
  were recorded for that case.
- Why the thesis texture produces no comparable bias is also open. There the
  low-contrast disc leaves small residuals inside the occluder too, yet V2
  ends within 1 mm. A plausible factor, untested, is how the retained
  measurements project onto the weakly observable direction, whose stiffness
  is the smallest eigenvalue of $\mathbf H_{\mathbf W}$ (rank margin about
  $10^{-5}$ in every run).

### 6.4 What the replication changes

- **The accuracy result (V3 at 0.66 × V2's error when both converge) is
  established on the thesis texture only.**
  - On coffee with the disc it cannot be tested, because V2 never converges.
  - On the clean coffee scene the gain is smaller (0.79 and 0.94) and not
    significant.
- **How the occluder affects V2 depends on the texture.**
  - Thesis texture: V2 converged at least as often as V3 (P2 not supported).
  - Coffee with the disc: V2 converges at no pose, against 24–28 of 32 for V3.
- **V3's residual bias under occlusion also depends on the texture:** 0.33 mm
  on the thesis texture, 6.6–8.4 mm on coffee.
  - The 20 mm / 1° threshold counts V3's coffee equilibrium as a success.
  - That equilibrium is of the same size as the clean-scene error at pass 600,
    but unlike that error it does not keep decreasing.
- **Two failure modes now have evidence.** With the current-image matrix, V3
  narrows the convergence domain at large displacements on both textures. On
  the clean coffee scene with the desired-image matrix, it also fails at more
  poses (8 against 1; not significant after Holm).

## 7. Occlusion-fraction sweep

Running. Pre-registered in `PREREGISTRATION_ADDENDUM.md` §S; to be reported
here.

## 8. What the article can claim

1. Structure-informed Tukey weighting (V3) reduces the final positioning error
   of robust photometric servoing against Tukey weighting (V2) when both
   converge.
   - On the thesis texture, the reduction is about a third with the
     desired-image interaction matrix (pre-registered, p_Holm = 4e-7) and with
     the current-image matrix (secondary), under three occluder appearances
     and on the clean scene.
   - On the second texture with the dark disc, V2 converges at no pose: it
     reaches a biased equilibrium 128–175 mm from the target, along the weakly
     observable direction. V3 converges at 24–28 of 32 poses, to 6.6–8.4 mm
     (pre-registered secondary, p_Holm ≤ 4e-7; the pre-registered accuracy
     test could not be computed).
   - V3 rejects an occluder by the absence of current-image structure, not by
     residual magnitude (§6.3).
2. With the current-image matrix this gain costs convergence at large initial
   displacements, on both textures. With the desired-image matrix the cost is
   3 of 32 poses on the thesis texture and 4–8 of 32 on the second texture.
3. Under occlusion the final cost ranks accuracy inversely, so it cannot serve
   as a convergence criterion. The stability margin of the reweighted law holds
   throughout.
4. Tukey weighting on residual magnitude can converge farther from the target
   than the unweighted law. This happens when part of the occluder matches the
   grey level of the hidden texture (exploratory; one texture, §6.3).

The occlusion-fraction sweep (§7) is still to be added.
