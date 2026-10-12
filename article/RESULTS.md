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

## 6. What the article can claim

1. Structure-informed Tukey weighting (V3) reduces the final positioning error
   of robust photometric servoing by about a third against Tukey weighting,
   with the desired-image interaction matrix (pre-registered, p_Holm = 4e-7)
   and with the current-image matrix (secondary), under three occluder
   appearances and on the clean scene.
2. With the current-image matrix this gain costs convergence at large initial
   displacements; with the desired-image matrix the cost falls to 3 of 32 poses.
3. Under occlusion the final cost ranks accuracy inversely, so it cannot serve
   as a convergence criterion. The stability margin of the reweighted law holds
   throughout.

Still needed for the article: the replication on a second texture and the
occlusion-fraction sweep (`PLAN.md`), both secondary.
