# Pilot campaign — development set

**Design.** 160 runs: poses 101–104 (one per displacement bin, 30–57 px;
disjoint from the confirmatory poses) × weightings V1–V5 × interaction matrix
from the current or the desired image × four scenes: clean, and the Chapter 4
disc refilled dark, textured or low-contrast (identical geometry, 7.5 % of the
image at the desired pose). Corrected principal point, first-order structure
measure, 600 passes, 155–175 s per run on four cores. Outputs in `pilot/`.

**Success** = camera-centre error below 20 mm and rotation error below 1°.
The pilot separates outcomes cleanly at these thresholds: converged runs end
below 17 mm and 0.71°; basin failures end above 440 mm and 19°; V1's occlusion
bias falls in between (31–35 mm, 1.35–1.53°).

These are development data on four poses: they shape the confirmatory
hypotheses and support no inference.

## 1. Convergence: the interaction matrix decides

Successful runs out of 4 poses:

| L from | Scene | V1 | V2 | V3 | V4 | V5 |
|---|---|---|---|---|---|---|
| current image | clean | 3 | 4 | **2** | 4 | **2** |
| current image | low-contrast disc | 3 | 4 | **2** | 4 | **2** |
| current image | dark disc | 0 (bias) | 4 | **2** | 4 | **2** |
| current image | textured disc | 0 (bias) | 4 | **2** | 4 | **2** |
| desired image | clean | 4 | 4 | 4 | 4 | 4 |
| desired image | low-contrast disc | 4 | 4 | 4 | 4 | 4 |
| desired image | dark disc | 0 (bias) | 4 | 4 | 4 | 4 |
| desired image | textured disc | 0 (bias) | 4 | 4 | 4 | 4 |

With the current-image matrix, V3 and V5 lose poses 102 and 104 in every scene.
They stall in a high-cost basin (intensity error stuck at 0.6 of its initial
value, then a slow drift to 20–29°). V1 loses pose 102 the same way; V2 and V4
lose none. The failure is not caused by the principal-point correction: pose
102 fails identically with the thesis convention (V3: 27.9°, V1: 26.0°).

Interpretation, to be tested: at large misalignment the measurements with the
strongest gradients are those whose linearisation is least valid. V2 rejects
their large residuals, V4 down-weights them further, and V3 protects them,
behaving there like the unweighted scheme.

## 2. Accuracy: with the desired-image matrix, V3 is best

Ratio V3/V2 per pose (101, 102, 103, 104), desired-image matrix:

| Scene | Rotation | Camera centre |
|---|---|---|
| dark disc | 0.93, 0.64, 0.62, 0.55 | 0.93, 0.64, 0.62, 0.55 |
| textured disc | 0.87, 0.63, 0.61, 0.53 | 0.87, 0.63, 0.61, 0.53 |
| low-contrast disc | 0.29, 0.43, 0.36, 0.31 | 0.29, 0.43, 0.36, 0.31 |
| clean | 0.46, 0.41, 0.78, 0.52 | 0.45, 0.42, 0.78, 0.52 |

V3 beats V2 in all 16 pose–scene pairs. Median final errors (converged runs):

| L from | Scene | V2 | V3 | V4 | V5 |
|---|---|---|---|---|---|
| desired | dark disc | 0.0136°, 0.31 mm | **0.0097°, 0.22 mm** | 0.0149°, 0.34 mm | 0.0122°, 0.28 mm |
| desired | low-contrast disc | 0.0329°, 0.75 mm | **0.0103°, 0.24 mm** | 0.0251°, 0.58 mm | 0.0213°, 0.49 mm |
| desired | clean | 0.527°, 12.1 mm | **0.258°, 5.9 mm** | 0.525°, 12.0 mm | 0.473°, 10.8 mm |
| current | dark disc | 0.0178°, 0.41 mm | 0.0161°, 0.37 mm | 0.0171°, 0.39 mm | 0.0211°, 0.48 mm |

On the clean scene V1 remains the most accurate (0.19°, 4.4 mm), as in the
chapter; V3 with the desired-image matrix halves V2's clean-scene penalty.

This is the regime Proposition 1 singles out. With the desired-image matrix,
the occluder's interior has non-zero interaction rows, and the structure
normalisation acts on measurements that enter the control law. The clean-scene
gain shows the benefit is not only occlusion rejection: the normalisation also
reduces Tukey's rejection of valid textured measurements (Section 4.3.3).

V5 shows no consistent gain over V2 (ratios 0.54–1.07) and V4 none (0.59–1.17).

## 3. Other observations

- **Stability.** $\min\lambda_{\min}(\mathbf H_{\mathbf W})/(\operatorname{tr}\mathbf H_{\mathbf W}/6)=1.2\times10^{-5}$
  over all 160 runs, above the $10^{-6}$ of Proposition 2. The failures are
  wrong basins, not loss of rank.
- **Stall.** Clean-scene tail slopes of $\log\lVert\Delta\mathbf t\rVert$ against
  $\log k$ are −0.62 to −1.13, steeper than the −1/2 of Proposition 3. The decay
  is algebraic, not geometric, but faster than predicted. H7 is reworded as a
  comparison of algebraic and geometric fits and becomes exploratory.
- **Cost does not rank accuracy under occlusion.** With the desired-image
  matrix, Kendall's τ between final ‖e‖² and camera-centre error is negative at
  every pose (median −0.4) in the dark, textured and low-contrast scenes. On the
  clean scene it is +0.7.
- **Cost of the weights,** per pass: V2 10.4 ms, V3 13.2 ms, V4 11.9 ms, V5
  17.8 ms, against about 255 ms for a whole pass.

## 4. Consequences for Chapter 4

- The tie between V2 and V3 in Section 4.7.2 is explained: Chapter 4 computes
  the interaction matrix from the current image, where Proposition 1 predicts
  it. With the desired-image matrix, V3 is consistently better.
- With the current-image matrix, V3 narrows the convergence domain. This is a
  limitation the chapter should state.

## 5. Decisions for the confirmatory campaign (draft, see `PREREGISTRATION.md`)

- Weightings V1–V4. V5 is dropped: no consistent effect, and 20 % of the
  compute.
- Both interaction matrices; the four scenes; 32 poses → 1 024 runs, about 12 h
  on four cores.
- Primary family (Holm, 3 tests):
  **P1a/P1b** desired-image matrix, dark disc: V3 vs V2 on camera-centre and
  rotation error (Wilcoxon);
  **P2** current-image matrix, clean scene: success V3 vs V2 (exact McNemar).
