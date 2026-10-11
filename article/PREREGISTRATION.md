# Pre-registration — confirmatory campaign

**Status: DRAFT, awaiting the author's approval.** Once approved, commit this
file unchanged before the first confirmatory run. The commit timestamp records
that the hypotheses preceded the data.

| Item | Value |
|---|---|
| Program | `cpp/herpvs_campaign.cpp` at the commit carrying this file |
| Conventions | corrected principal point (`-x 1`), first-order structure measure (`-G 1`), floor on (`-F 1`), 600 passes |
| Poses | `campaign/poses.csv`, seed 20261011, ids 1–32, unchanged after this commit |
| Weightings | V1 none, V2 Tukey, V3 Hermite-informed, V4 structure-penalised |
| Interaction matrix | current image, desired image |
| Scenes | clean; Chapter 4 disc refilled dark, textured, low-contrast (`make_occluders.py refill`) |
| Runs | 32 × 4 × 2 × 4 = 1 024 |
| Outcomes | final camera-centre error (mm) and rotation error (deg): median of the last 50 passes of ${}^{c^*}\mathbf M_c$ |
| Success | camera-centre error < 20 mm and rotation error < 1° |

## Primary family (Holm, α = 0.05, 3 tests)

| | Comparison | Test | Prediction |
|---|---|---|---|
| P1a | desired-image L, dark disc, V3 vs V2, camera-centre error | Wilcoxon signed-rank, two-sided, poses where both succeed | V3 < V2 |
| P1b | same, rotation error | same | V3 < V2 |
| P2 | current-image L, clean scene, success V3 vs V2 | exact McNemar, two-sided | V3 succeeds less often |

Effect sizes: Hodges–Lehmann median ratio with exact 95 % CI (P1); discordant
counts and the difference in success rate (P2).

## Decision rule

- **Method paper** if P1a and P1b are significant after Holm with median ratio
  ≤ 0.8. Contribution: structure-informed weighting improves robust
  photometric servoing when the interaction matrix is taken from the desired
  image, as Proposition 1 predicts, and it narrows the convergence domain
  when the matrix is taken from the current image (P2).
- **Analysis paper** otherwise.

## Secondary, reported with Holm within the family

V2 vs V1 under occlusion; V3 vs V2 in every other cell; V4 vs V2; success
comparisons in every cell; equivalence of V3 and V2 (95 % CI within
0.8–1.25) where both converge with the current-image matrix.

## Exploratory, no test

Tail decay (algebraic against geometric fits); Kendall τ between final cost
and accuracy; rank margin; weight change; per-channel rejection; cost of the
weights; success against displacement bin.

## Changes from the plan, with reasons (from `PILOT.md`)

- V5 dropped: no consistent effect in the pilot.
- Success thresholds fixed at 20 mm and 1°: they separate converged runs
  (< 17 mm, < 0.71°) from basin failures (> 440 mm, > 19°) in the pilot.
- The primary family moved from "V3 and V5 vs V2, desired-image L" to P1–P2
  above. P2 is added because the pilot's largest effect is on convergence.
- H7 (tail slope −1/2) becomes exploratory: measured slopes were −0.6 to −1.1.
