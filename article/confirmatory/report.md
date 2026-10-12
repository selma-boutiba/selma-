# Campaign results

1024 runs, 32 poses.

## Primary family (Holm over 6 tests)

**Decision:** method paper: every P1 test is significant after Holm with ratio <= 0.80

| test | interaction | scenario | comparison | outcome | n | effect | 95 % CI / counts | p | p (Holm) |
|---|---|---|---|---|---|---|---|---|---|
| P1 | desired | uniform_dark_disc | hermite / tukey | final_t_mm | 29 | 0.658 | [0.6, 0.732] | 7.08e-08 | 4.25e-07 |
| P1 | desired | uniform_dark_disc | hermite / tukey | final_r_deg | 29 | 0.657 | [0.6, 0.732] | 7.08e-08 | 4.25e-07 |
| P2 | current | low_contrast_disc | hermite / tukey | success | 7 | -5 | hermite-only 1, tukey-only 6 | 0.125 | 0.5 |
| P2 | current | nominal | hermite / tukey | success | 7 | -3 | hermite-only 2, tukey-only 5 | 0.453 | 1 |
| P2 | current | textured_disc | hermite / tukey | success | 5 | -1 | hermite-only 2, tukey-only 3 | 1 | 1 |
| P2 | current | uniform_dark_disc | hermite / tukey | success | 5 | -1 | hermite-only 2, tukey-only 3 | 1 | 1 |

## Final pose error per cell (median [IQR])

| interaction | scenario | weighting | n | translation (mm) | rotation (deg) | success | tail slope |
|---|---|---|---|---|---|---|---|
| current | low_contrast_disc | none | 32 | 15.7 [15.7, 15.7] | 0.678 [0.678, 0.678] | 100 % | -0.00 |
| current | low_contrast_disc | tukey | 32 | 1 [0.559, 1.68] | 0.044 [0.0245, 0.0741] | 97 % | -2.28 |
| current | low_contrast_disc | hermite | 32 | 0.491 [0.285, 0.65] | 0.0216 [0.0125, 0.0286] | 81 % | -1.01 |
| current | low_contrast_disc | penalised | 32 | 0.808 [0.611, 2.47] | 0.0355 [0.0267, 0.109] | 81 % | -1.69 |
| current | nominal | none | 32 | 5.55 [4.67, 6.44] | 0.246 [0.207, 0.284] | 94 % | -0.64 |
| current | nominal | tukey | 32 | 13 [10.4, 15.6] | 0.569 [0.455, 0.686] | 91 % | -0.89 |
| current | nominal | hermite | 32 | 9.87 [7.37, 14.5] | 0.435 [0.323, 0.64] | 81 % | -0.63 |
| current | nominal | penalised | 32 | 11.1 [8.99, 13.8] | 0.487 [0.394, 0.609] | 81 % | -0.73 |
| current | textured_disc | none | 32 | 34.8 [34.8, 34.8] | 1.51 [1.51, 1.51] | 0 % | -0.00 |
| current | textured_disc | tukey | 32 | 0.527 [0.361, 0.741] | 0.0231 [0.0159, 0.0325] | 88 % | -1.07 |
| current | textured_disc | hermite | 32 | 0.373 [0.251, 0.456] | 0.0164 [0.011, 0.02] | 84 % | -0.27 |
| current | textured_disc | penalised | 32 | 0.486 [0.366, 2.93] | 0.0213 [0.0161, 0.129] | 78 % | -0.77 |
| current | uniform_dark_disc | none | 32 | 35.5 [35.5, 35.5] | 1.53 [1.53, 1.53] | 0 % | 0.00 |
| current | uniform_dark_disc | tukey | 32 | 0.519 [0.358, 0.727] | 0.0228 [0.0157, 0.0319] | 88 % | -1.04 |
| current | uniform_dark_disc | hermite | 32 | 0.37 [0.257, 0.454] | 0.0163 [0.0112, 0.02] | 84 % | -0.25 |
| current | uniform_dark_disc | penalised | 32 | 0.473 [0.368, 281] | 0.0208 [0.0162, 18] | 72 % | -0.71 |
| desired | low_contrast_disc | none | 32 | 15 [15, 15] | 0.649 [0.649, 0.649] | 97 % | -0.00 |
| desired | low_contrast_disc | tukey | 32 | 0.939 [0.666, 1.3] | 0.0413 [0.0293, 0.0572] | 100 % | -2.27 |
| desired | low_contrast_disc | hermite | 32 | 0.372 [0.274, 0.6] | 0.0164 [0.012, 0.0264] | 91 % | -1.32 |
| desired | low_contrast_disc | penalised | 32 | 0.894 [0.687, 1.08] | 0.0392 [0.0302, 0.0472] | 100 % | -2.20 |
| desired | nominal | none | 32 | 5.26 [4.39, 6.09] | 0.232 [0.195, 0.268] | 97 % | -0.66 |
| desired | nominal | tukey | 32 | 13.5 [11.9, 14.6] | 0.592 [0.525, 0.642] | 97 % | -0.93 |
| desired | nominal | hermite | 32 | 9.13 [7.01, 11.7] | 0.402 [0.308, 0.514] | 91 % | -0.66 |
| desired | nominal | penalised | 32 | 11.9 [10.4, 14.3] | 0.527 [0.458, 0.628] | 100 % | -0.90 |
| desired | textured_disc | none | 32 | 31.3 [31.3, 31.3] | 1.35 [1.35, 1.35] | 0 % | -0.00 |
| desired | textured_disc | tukey | 32 | 0.493 [0.42, 0.594] | 0.0217 [0.0184, 0.0261] | 100 % | -1.31 |
| desired | textured_disc | hermite | 32 | 0.324 [0.243, 0.434] | 0.0142 [0.0106, 0.0191] | 91 % | -0.33 |
| desired | textured_disc | penalised | 32 | 0.447 [0.395, 0.518] | 0.0196 [0.0173, 0.0227] | 100 % | -1.23 |
| desired | uniform_dark_disc | none | 32 | 31.6 [31.6, 31.6] | 1.37 [1.37, 1.37] | 0 % | -0.00 |
| desired | uniform_dark_disc | tukey | 32 | 0.489 [0.415, 0.585] | 0.0215 [0.0182, 0.0258] | 100 % | -1.27 |
| desired | uniform_dark_disc | hermite | 32 | 0.348 [0.242, 0.431] | 0.0153 [0.0106, 0.019] | 91 % | -0.33 |
| desired | uniform_dark_disc | penalised | 32 | 0.443 [0.391, 0.51] | 0.0194 [0.0171, 0.0224] | 100 % | -1.18 |

## Paired comparisons (ratio A/B, HL estimate [95 % CI]; Holm-adjusted p within family)

| family | interaction | scenario | A / B | outcome | n | ratio [95 % CI] | p (Holm) | equivalent (±25 %) |
|---|---|---|---|---|---|---|---|---|
| primary | desired | uniform_dark_disc | hermite / tukey | final_t_mm | 29 | 0.658 [0.6, 0.732] | 4.25e-07 | no |
| primary | desired | uniform_dark_disc | hermite / tukey | final_r_deg | 29 | 0.657 [0.6, 0.732] | 4.25e-07 | no |
| secondary | current | low_contrast_disc | tukey / none | final_t_mm | 32 | 0.0622 [0.0466, 0.089] | 5.75e-05 | no |
| secondary | current | low_contrast_disc | tukey / none | final_r_deg | 32 | 0.0632 [0.0472, 0.0904] | 6.57e-05 | no |
| secondary | current | low_contrast_disc | hermite / tukey | final_t_mm | 32 | 0.526 [0.39, 6.97] | 1 | no |
| secondary | current | low_contrast_disc | hermite / tukey | final_r_deg | 32 | 0.525 [0.389, 7.52] | 1 | no |
| secondary | current | low_contrast_disc | penalised / tukey | final_t_mm | 32 | 1.1 [0.834, 2.77] | 1 | no |
| secondary | current | low_contrast_disc | penalised / tukey | final_r_deg | 32 | 1.1 [0.833, 2.79] | 1 | no |
| secondary | current | low_contrast_disc | hermite / penalised | final_t_mm | 32 | 0.44 [0.21, 0.928] | 0.895 | no |
| secondary | current | low_contrast_disc | hermite / penalised | final_r_deg | 32 | 0.441 [0.209, 1.14] | 1 | no |
| secondary | current | nominal | tukey / none | final_t_mm | 32 | 2.37 [2, 2.75] | 0.00235 | no |
| secondary | current | nominal | tukey / none | final_r_deg | 32 | 2.35 [1.99, 2.74] | 0.00235 | no |
| secondary | current | nominal | hermite / tukey | final_t_mm | 32 | 0.802 [0.695, 3.69] | 1 | no |
| secondary | current | nominal | hermite / tukey | final_r_deg | 32 | 0.802 [0.695, 3.87] | 1 | no |
| secondary | current | nominal | penalised / tukey | final_t_mm | 32 | 0.936 [0.807, 1.22] | 1 | yes |
| secondary | current | nominal | penalised / tukey | final_r_deg | 32 | 0.972 [0.833, 1.28] | 1 | no |
| secondary | current | nominal | hermite / penalised | final_t_mm | 32 | 0.867 [0.584, 1.34] | 1 | no |
| secondary | current | nominal | hermite / penalised | final_r_deg | 32 | 0.851 [0.581, 1.33] | 1 | no |
| secondary | current | textured_disc | tukey / none | final_t_mm | 32 | 0.0151 [0.0123, 0.0202] | 1.83e-06 | no |
| secondary | current | textured_disc | tukey / none | final_r_deg | 32 | 0.0153 [0.0125, 0.0205] | 4.17e-06 | no |
| secondary | current | textured_disc | hermite / tukey | final_t_mm | 32 | 0.676 [0.546, 0.948] | 0.895 | no |
| secondary | current | textured_disc | hermite / tukey | final_r_deg | 32 | 0.676 [0.547, 0.952] | 0.895 | no |
| secondary | current | textured_disc | penalised / tukey | final_t_mm | 32 | 1.02 [0.821, 5.14] | 1 | no |
| secondary | current | textured_disc | penalised / tukey | final_r_deg | 32 | 1.03 [0.82, 5.24] | 1 | no |
| secondary | current | textured_disc | hermite / penalised | final_t_mm | 32 | 0.628 [0.0359, 0.932] | 0.895 | no |
| secondary | current | textured_disc | hermite / penalised | final_r_deg | 32 | 0.628 [0.0294, 0.931] | 0.895 | no |
| secondary | current | uniform_dark_disc | tukey / none | final_t_mm | 32 | 0.0146 [0.0121, 0.0194] | 1.83e-06 | no |
| secondary | current | uniform_dark_disc | tukey / none | final_r_deg | 32 | 0.0148 [0.0123, 0.0197] | 2.77e-06 | no |
| secondary | current | uniform_dark_disc | hermite / tukey | final_t_mm | 32 | 0.67 [0.556, 0.957] | 0.895 | no |
| secondary | current | uniform_dark_disc | hermite / tukey | final_r_deg | 32 | 0.671 [0.557, 0.969] | 0.895 | no |
| secondary | current | uniform_dark_disc | penalised / tukey | final_t_mm | 32 | 1.03 [0.82, 23.5] | 1 | no |
| secondary | current | uniform_dark_disc | penalised / tukey | final_r_deg | 32 | 1.03 [0.818, 25.4] | 1 | no |
| secondary | current | uniform_dark_disc | hermite / penalised | final_t_mm | 32 | 0.629 [0.0259, 1.01] | 0.99 | no |
| secondary | current | uniform_dark_disc | hermite / penalised | final_r_deg | 32 | 0.628 [0.0237, 1.01] | 0.99 | no |
| secondary | desired | low_contrast_disc | tukey / none | final_t_mm | 32 | 0.0611 [0.0504, 0.074] | 2.89e-08 | no |
| secondary | desired | low_contrast_disc | tukey / none | final_r_deg | 32 | 0.062 [0.0512, 0.075] | 2.89e-08 | no |
| secondary | desired | low_contrast_disc | hermite / tukey | final_t_mm | 32 | 0.426 [0.358, 0.53] | 0.0417 | no |
| secondary | desired | low_contrast_disc | hermite / tukey | final_r_deg | 32 | 0.426 [0.358, 0.53] | 0.0417 | no |
| secondary | desired | low_contrast_disc | penalised / tukey | final_t_mm | 32 | 0.915 [0.856, 0.984] | 0.878 | yes |
| secondary | desired | low_contrast_disc | penalised / tukey | final_r_deg | 32 | 0.914 [0.855, 0.982] | 0.816 | yes |
| secondary | desired | low_contrast_disc | hermite / penalised | final_t_mm | 32 | 0.466 [0.394, 0.581] | 0.0431 | no |
| secondary | desired | low_contrast_disc | hermite / penalised | final_r_deg | 32 | 0.468 [0.395, 0.583] | 0.0431 | no |
| secondary | desired | nominal | tukey / none | final_t_mm | 32 | 2.48 [2.1, 2.78] | 6.57e-05 | no |
| secondary | desired | nominal | tukey / none | final_r_deg | 32 | 2.45 [2.1, 2.75] | 6.57e-05 | no |
| secondary | desired | nominal | hermite / tukey | final_t_mm | 32 | 0.73 [0.633, 0.86] | 0.149 | no |
| secondary | desired | nominal | hermite / tukey | final_r_deg | 32 | 0.733 [0.636, 0.864] | 0.165 | no |
| secondary | desired | nominal | penalised / tukey | final_t_mm | 32 | 0.928 [0.857, 0.993] | 0.895 | yes |
| secondary | desired | nominal | penalised / tukey | final_r_deg | 32 | 0.926 [0.855, 0.992] | 0.895 | yes |
| secondary | desired | nominal | hermite / penalised | final_t_mm | 32 | 0.786 [0.679, 1.03] | 1 | no |
| secondary | desired | nominal | hermite / penalised | final_r_deg | 32 | 0.789 [0.682, 1.03] | 1 | no |
| secondary | desired | textured_disc | tukey / none | final_t_mm | 32 | 0.0158 [0.0141, 0.0175] | 2.89e-08 | no |
| secondary | desired | textured_disc | tukey / none | final_r_deg | 32 | 0.016 [0.0143, 0.0178] | 2.89e-08 | no |
| secondary | desired | textured_disc | hermite / tukey | final_t_mm | 32 | 0.669 [0.602, 0.791] | 0.065 | no |
| secondary | desired | textured_disc | hermite / tukey | final_r_deg | 32 | 0.668 [0.6, 0.791] | 0.065 | no |
| secondary | desired | textured_disc | penalised / tukey | final_t_mm | 32 | 0.927 [0.87, 0.989] | 0.895 | yes |
| secondary | desired | textured_disc | penalised / tukey | final_r_deg | 32 | 0.925 [0.869, 0.986] | 0.895 | yes |
| secondary | desired | textured_disc | hermite / penalised | final_t_mm | 32 | 0.724 [0.633, 0.845] | 0.0772 | no |
| secondary | desired | textured_disc | hermite / penalised | final_r_deg | 32 | 0.724 [0.633, 0.848] | 0.0772 | no |
| secondary | desired | uniform_dark_disc | tukey / none | final_t_mm | 32 | 0.0149 [0.0133, 0.0167] | 2.89e-08 | no |
| secondary | desired | uniform_dark_disc | tukey / none | final_r_deg | 32 | 0.0152 [0.0135, 0.017] | 2.89e-08 | no |
| secondary | desired | uniform_dark_disc | penalised / tukey | final_t_mm | 32 | 0.927 [0.87, 0.991] | 0.895 | yes |
| secondary | desired | uniform_dark_disc | penalised / tukey | final_r_deg | 32 | 0.926 [0.869, 0.988] | 0.895 | yes |
| secondary | desired | uniform_dark_disc | hermite / penalised | final_t_mm | 32 | 0.748 [0.643, 0.862] | 0.0908 | no |
| secondary | desired | uniform_dark_disc | hermite / penalised | final_r_deg | 32 | 0.749 [0.644, 0.864] | 0.0908 | no |

## Does the final cost rank the accuracy? (Kendall tau within pose)

- current, low_contrast_disc: median tau -0.17, negative in 50 % of 32 poses
- current, nominal: median tau 1.00, negative in 0 % of 32 poses
- current, textured_disc: median tau -0.17, negative in 50 % of 32 poses
- current, uniform_dark_disc: median tau 0.00, negative in 47 % of 32 poses
- desired, low_contrast_disc: median tau -0.67, negative in 84 % of 32 poses
- desired, nominal: median tau 0.83, negative in 0 % of 32 poses
- desired, textured_disc: median tau -0.67, negative in 78 % of 32 poses
- desired, uniform_dark_disc: median tau -0.33, negative in 72 % of 32 poses
