# Campaign results

256 runs, 32 poses.

## Primary family (Holm over 2 tests)

**Decision:** not replicated: the P1 condition is not met

| test | interaction | scenario | comparison | outcome | n | effect | 95 % CI / counts | p | p (Holm) |
|---|---|---|---|---|---|---|---|---|---|
| P1 | desired | coffee_dark_disc | hermite / tukey | final_t_mm | 0 | nan | [nan, nan] | nan | nan |
| P1 | desired | coffee_dark_disc | hermite / tukey | final_r_deg | 0 | nan | [nan, nan] | nan | nan |

## Final pose error per cell (median [IQR])

| interaction | scenario | weighting | n | translation (mm) | rotation (deg) | success | tail slope |
|---|---|---|---|---|---|---|---|
| current | coffee_dark_disc | tukey | 32 | 128 [128, 128] | 5.62 [5.61, 5.63] | 0 % | -0.02 |
| current | coffee_dark_disc | hermite | 32 | 6.63 [6.63, 56.2] | 0.298 [0.298, 1.42] | 75 % | 0.00 |
| current | nominal | tukey | 32 | 10 [8.34, 12.1] | 0.44 [0.366, 0.533] | 100 % | -1.19 |
| current | nominal | hermite | 32 | 8.64 [7, 9.78] | 0.38 [0.307, 0.43] | 94 % | -0.65 |
| desired | coffee_dark_disc | tukey | 32 | 175 [175, 175] | 7.69 [7.68, 7.69] | 0 % | 0.06 |
| desired | coffee_dark_disc | hermite | 32 | 8.36 [8.36, 8.36] | 0.363 [0.363, 0.363] | 88 % | 0.00 |
| desired | nominal | tukey | 32 | 9.97 [7.83, 10.9] | 0.438 [0.345, 0.48] | 97 % | -1.02 |
| desired | nominal | hermite | 32 | 10 [7.52, 19.9] | 0.442 [0.331, 0.877] | 75 % | -0.67 |

## Paired comparisons (ratio A/B, HL estimate [95 % CI]; Holm-adjusted p within family)

| family | interaction | scenario | A / B | outcome | n | ratio [95 % CI] | p (Holm) | equivalent (±25 %) |
|---|---|---|---|---|---|---|---|---|
| primary | desired | coffee_dark_disc | hermite / tukey | final_t_mm | 0 | n/a | nan | no |
| primary | desired | coffee_dark_disc | hermite / tukey | final_r_deg | 0 | n/a | nan | no |
| secondary | current | coffee_dark_disc | hermite / tukey | final_t_mm | 32 | 0.0521 [0.0518, 0.511] | 1.2e-05 | no |
| secondary | current | coffee_dark_disc | hermite / tukey | final_r_deg | 32 | 0.0533 [0.053, 0.516] | 1.2e-05 | no |
| secondary | current | nominal | hermite / tukey | final_t_mm | 32 | 0.835 [0.678, 1.04] | 0.296 | no |
| secondary | current | nominal | hermite / tukey | final_r_deg | 32 | 0.836 [0.679, 1.04] | 0.296 | no |
| secondary | desired | nominal | hermite / tukey | final_t_mm | 32 | 1.22 [0.937, 4.65] | 0.298 | no |
| secondary | desired | nominal | hermite / tukey | final_r_deg | 32 | 1.23 [0.94, 3.94] | 0.298 | no |

## Does the final cost rank the accuracy? (Kendall tau within pose)

- current, coffee_dark_disc: median tau nan, negative in nan % of 0 poses
- current, nominal: median tau nan, negative in nan % of 0 poses
- desired, coffee_dark_disc: median tau nan, negative in nan % of 0 poses
- desired, nominal: median tau nan, negative in nan % of 0 poses
