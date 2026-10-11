# Campaign results

160 runs, 4 poses.

## Primary family (Holm over 6 tests)

**Decision:** analysis paper: the P1 condition is not met

| test | interaction | scenario | comparison | outcome | n | effect | 95 % CI / counts | p | p (Holm) |
|---|---|---|---|---|---|---|---|---|---|
| P1 | desired | uniform_dark_disc | hermite / tukey | final_t_mm | 4 | nan | [nan, nan] | nan | nan |
| P1 | desired | uniform_dark_disc | hermite / tukey | final_r_deg | 4 | nan | [nan, nan] | nan | nan |
| P2 | current | low_contrast_disc | hermite / tukey | success | 2 | -2 | hermite-only 0, tukey-only 2 | 0.5 | 1 |
| P2 | current | nominal | hermite / tukey | success | 2 | -2 | hermite-only 0, tukey-only 2 | 0.5 | 1 |
| P2 | current | textured_disc | hermite / tukey | success | 2 | -2 | hermite-only 0, tukey-only 2 | 0.5 | 1 |
| P2 | current | uniform_dark_disc | hermite / tukey | success | 2 | -2 | hermite-only 0, tukey-only 2 | 0.5 | 1 |

## Final pose error per cell (median [IQR])

| interaction | scenario | weighting | n | translation (mm) | rotation (deg) | success | tail slope |
|---|---|---|---|---|---|---|---|
| current | low_contrast_disc | none | 4 | 15.7 [15.7, 142] | 0.678 [0.678, 6.62] | 75 % | 0.00 |
| current | low_contrast_disc | tukey | 4 | 0.698 [0.39, 1.06] | 0.0306 [0.0172, 0.0466] | 100 % | -2.95 |
| current | low_contrast_disc | hermite | 4 | 222 [0.538, 456] | 10.1 [0.0237, 21] | 50 % | -0.39 |
| current | low_contrast_disc | penalised | 4 | 0.72 [0.591, 0.886] | 0.0315 [0.0259, 0.0388] | 100 % | -1.77 |
| current | low_contrast_disc | adaptive | 4 | 227 [1.2, 485] | 10.2 [0.0526, 21.8] | 50 % | -0.62 |
| current | nominal | none | 4 | 4.96 [4.15, 123] | 0.219 [0.183, 5.54] | 75 % | -0.67 |
| current | nominal | tukey | 4 | 14.9 [10.6, 16.2] | 0.656 [0.472, 0.71] | 100 % | -1.13 |
| current | nominal | hermite | 4 | 228 [9.53, 475] | 10.1 [0.42, 21.1] | 50 % | -0.20 |
| current | nominal | penalised | 4 | 10.9 [10.1, 11.5] | 0.476 [0.443, 0.502] | 100 % | -0.83 |
| current | nominal | adaptive | 4 | 228 [10.8, 474] | 10.1 [0.476, 21] | 50 % | -0.23 |
| current | textured_disc | none | 4 | 34.8 [34.8, 176] | 1.51 [1.51, 8.32] | 0 % | 0.00 |
| current | textured_disc | tukey | 4 | 0.412 [0.266, 0.55] | 0.0181 [0.0117, 0.0242] | 100 % | -2.21 |
| current | textured_disc | hermite | 4 | 282 [0.38, 574] | 13.6 [0.0167, 27.7] | 50 % | -0.02 |
| current | textured_disc | penalised | 4 | 0.399 [0.335, 0.472] | 0.0175 [0.0147, 0.0207] | 100 % | -0.92 |
| current | textured_disc | adaptive | 4 | 276 [0.578, 560] | 13.2 [0.0254, 26.8] | 50 % | -0.27 |
| current | uniform_dark_disc | none | 4 | 35.5 [35.5, 176] | 1.53 [1.53, 8.31] | 0 % | 0.00 |
| current | uniform_dark_disc | tukey | 4 | 0.405 [0.263, 0.539] | 0.0178 [0.0116, 0.0237] | 100 % | -2.19 |
| current | uniform_dark_disc | hermite | 4 | 282 [0.415, 575] | 13.5 [0.0182, 27.7] | 50 % | -0.01 |
| current | uniform_dark_disc | penalised | 4 | 0.391 [0.328, 0.462] | 0.0171 [0.0143, 0.0203] | 100 % | -0.88 |
| current | uniform_dark_disc | adaptive | 4 | 278 [0.593, 565] | 13 [0.0261, 26.6] | 50 % | -0.25 |
| desired | low_contrast_disc | none | 4 | 15 [15, 15] | 0.649 [0.649, 0.649] | 100 % | -0.00 |
| desired | low_contrast_disc | tukey | 4 | 0.751 [0.621, 1.08] | 0.0329 [0.0272, 0.0475] | 100 % | -2.14 |
| desired | low_contrast_disc | hermite | 4 | 0.235 [0.221, 0.341] | 0.0103 [0.00964, 0.015] | 100 % | -1.29 |
| desired | low_contrast_disc | penalised | 4 | 0.576 [0.527, 0.762] | 0.0251 [0.023, 0.0333] | 100 % | -1.89 |
| desired | low_contrast_disc | adaptive | 4 | 0.487 [0.4, 0.879] | 0.0213 [0.0175, 0.0385] | 100 % | -1.73 |
| desired | nominal | none | 4 | 4.39 [4.27, 5.27] | 0.192 [0.184, 0.234] | 100 % | -0.62 |
| desired | nominal | tukey | 4 | 12.1 [11.3, 13.3] | 0.527 [0.493, 0.585] | 100 % | -0.89 |
| desired | nominal | hermite | 4 | 5.86 [4.95, 7.94] | 0.258 [0.219, 0.347] | 100 % | -0.62 |
| desired | nominal | penalised | 4 | 12 [10.5, 12.8] | 0.525 [0.461, 0.558] | 100 % | -0.92 |
| desired | nominal | adaptive | 4 | 10.8 [10.3, 12.3] | 0.473 [0.454, 0.539] | 100 % | -0.71 |
| desired | textured_disc | none | 4 | 31.3 [31.3, 31.3] | 1.35 [1.35, 1.35] | 0 % | 0.00 |
| desired | textured_disc | tukey | 4 | 0.316 [0.294, 0.438] | 0.0138 [0.0129, 0.0192] | 100 % | -1.04 |
| desired | textured_disc | hermite | 4 | 0.221 [0.185, 0.306] | 0.0097 [0.00806, 0.0135] | 100 % | -0.31 |
| desired | textured_disc | penalised | 4 | 0.347 [0.333, 0.405] | 0.0151 [0.0146, 0.0177] | 100 % | -0.93 |
| desired | textured_disc | adaptive | 4 | 0.282 [0.245, 0.404] | 0.0123 [0.0108, 0.0177] | 100 % | -0.91 |
| desired | uniform_dark_disc | none | 4 | 31.6 [31.6, 31.6] | 1.37 [1.37, 1.37] | 0 % | -0.00 |
| desired | uniform_dark_disc | tukey | 4 | 0.311 [0.287, 0.431] | 0.0136 [0.0126, 0.0189] | 100 % | -1.00 |
| desired | uniform_dark_disc | hermite | 4 | 0.221 [0.186, 0.305] | 0.0097 [0.0081, 0.0134] | 100 % | -0.28 |
| desired | uniform_dark_disc | penalised | 4 | 0.342 [0.329, 0.398] | 0.0149 [0.0144, 0.0174] | 100 % | -0.90 |
| desired | uniform_dark_disc | adaptive | 4 | 0.28 [0.243, 0.399] | 0.0122 [0.0107, 0.0175] | 100 % | -0.88 |

## Paired comparisons (ratio A/B, HL estimate [95 % CI]; Holm-adjusted p within family)

| family | interaction | scenario | A / B | outcome | n | ratio [95 % CI] | p (Holm) | equivalent (±25 %) |
|---|---|---|---|---|---|---|---|---|
| primary | desired | uniform_dark_disc | hermite / tukey | final_t_mm | 4 | n/a | nan | no |
| primary | desired | uniform_dark_disc | hermite / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | current | low_contrast_disc | tukey / none | final_t_mm | 4 | n/a | nan | no |
| secondary | current | low_contrast_disc | tukey / none | final_r_deg | 4 | n/a | nan | no |
| secondary | current | low_contrast_disc | hermite / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | current | low_contrast_disc | hermite / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | current | low_contrast_disc | penalised / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | current | low_contrast_disc | penalised / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | current | low_contrast_disc | adaptive / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | current | low_contrast_disc | adaptive / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | current | low_contrast_disc | hermite / penalised | final_t_mm | 4 | n/a | nan | no |
| secondary | current | low_contrast_disc | hermite / penalised | final_r_deg | 4 | n/a | nan | no |
| secondary | current | nominal | tukey / none | final_t_mm | 4 | n/a | nan | no |
| secondary | current | nominal | tukey / none | final_r_deg | 4 | n/a | nan | no |
| secondary | current | nominal | hermite / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | current | nominal | hermite / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | current | nominal | penalised / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | current | nominal | penalised / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | current | nominal | adaptive / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | current | nominal | adaptive / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | current | nominal | hermite / penalised | final_t_mm | 4 | n/a | nan | no |
| secondary | current | nominal | hermite / penalised | final_r_deg | 4 | n/a | nan | no |
| secondary | current | textured_disc | tukey / none | final_t_mm | 4 | n/a | nan | no |
| secondary | current | textured_disc | tukey / none | final_r_deg | 4 | n/a | nan | no |
| secondary | current | textured_disc | hermite / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | current | textured_disc | hermite / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | current | textured_disc | penalised / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | current | textured_disc | penalised / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | current | textured_disc | adaptive / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | current | textured_disc | adaptive / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | current | textured_disc | hermite / penalised | final_t_mm | 4 | n/a | nan | no |
| secondary | current | textured_disc | hermite / penalised | final_r_deg | 4 | n/a | nan | no |
| secondary | current | uniform_dark_disc | tukey / none | final_t_mm | 4 | n/a | nan | no |
| secondary | current | uniform_dark_disc | tukey / none | final_r_deg | 4 | n/a | nan | no |
| secondary | current | uniform_dark_disc | hermite / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | current | uniform_dark_disc | hermite / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | current | uniform_dark_disc | penalised / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | current | uniform_dark_disc | penalised / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | current | uniform_dark_disc | adaptive / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | current | uniform_dark_disc | adaptive / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | current | uniform_dark_disc | hermite / penalised | final_t_mm | 4 | n/a | nan | no |
| secondary | current | uniform_dark_disc | hermite / penalised | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | low_contrast_disc | tukey / none | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | low_contrast_disc | tukey / none | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | low_contrast_disc | hermite / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | low_contrast_disc | hermite / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | low_contrast_disc | penalised / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | low_contrast_disc | penalised / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | low_contrast_disc | adaptive / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | low_contrast_disc | adaptive / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | low_contrast_disc | hermite / penalised | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | low_contrast_disc | hermite / penalised | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | nominal | tukey / none | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | nominal | tukey / none | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | nominal | hermite / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | nominal | hermite / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | nominal | penalised / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | nominal | penalised / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | nominal | adaptive / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | nominal | adaptive / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | nominal | hermite / penalised | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | nominal | hermite / penalised | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | textured_disc | tukey / none | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | textured_disc | tukey / none | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | textured_disc | hermite / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | textured_disc | hermite / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | textured_disc | penalised / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | textured_disc | penalised / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | textured_disc | adaptive / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | textured_disc | adaptive / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | textured_disc | hermite / penalised | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | textured_disc | hermite / penalised | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | uniform_dark_disc | tukey / none | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | uniform_dark_disc | tukey / none | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | uniform_dark_disc | penalised / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | uniform_dark_disc | penalised / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | uniform_dark_disc | adaptive / tukey | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | uniform_dark_disc | adaptive / tukey | final_r_deg | 4 | n/a | nan | no |
| secondary | desired | uniform_dark_disc | hermite / penalised | final_t_mm | 4 | n/a | nan | no |
| secondary | desired | uniform_dark_disc | hermite / penalised | final_r_deg | 4 | n/a | nan | no |

## Does the final cost rank the accuracy? (Kendall tau within pose)

- current, low_contrast_disc: median tau -0.10, negative in 50 % of 4 poses
- current, nominal: median tau 0.70, negative in 0 % of 4 poses
- current, textured_disc: median tau 0.00, negative in 50 % of 4 poses
- current, uniform_dark_disc: median tau 0.20, negative in 25 % of 4 poses
- desired, low_contrast_disc: median tau -0.20, negative in 100 % of 4 poses
- desired, nominal: median tau 0.70, negative in 0 % of 4 poses
- desired, textured_disc: median tau -0.40, negative in 100 % of 4 poses
- desired, uniform_dark_disc: median tau -0.40, negative in 100 % of 4 poses
