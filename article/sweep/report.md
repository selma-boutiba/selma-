# Campaign results

512 runs, 32 poses.

## Primary family (Holm over 24 tests)

**Decision:** gain not at every fraction: the P1 condition is not met

| test | interaction | scenario | comparison | outcome | n | effect | 95 % CI / counts | p | p (Holm) |
|---|---|---|---|---|---|---|---|---|---|
| P1 | current | uniform_dark_f150 | hermite / tukey | final_t_mm | 24 | 0.765 | [0.704, 0.826] | 2.01e-05 | 0.000262 |
| P1 | current | uniform_dark_f150 | hermite / tukey | final_r_deg | 24 | 0.765 | [0.703, 0.827] | 2.01e-05 | 0.000262 |
| P1 | current | uniform_dark_f250 | hermite / tukey | final_t_mm | 16 | 0.46 | [0.364, 0.624] | 3.05e-05 | 0.000336 |
| P1 | current | uniform_dark_f250 | hermite / tukey | final_r_deg | 16 | 0.46 | [0.364, 0.623] | 3.05e-05 | 0.000336 |
| P1 | current | uniform_dark_f350 | hermite / tukey | final_t_mm | 12 | 0.361 | [0.255, 0.529] | 0.000488 | 0.00293 |
| P1 | current | uniform_dark_f350 | hermite / tukey | final_r_deg | 12 | 0.361 | [0.255, 0.53] | 0.000488 | 0.00293 |
| P1 | current | uniform_dark_f450 | hermite / tukey | final_t_mm | 0 | nan | [nan, nan] | nan | nan |
| P1 | current | uniform_dark_f450 | hermite / tukey | final_r_deg | 0 | nan | [nan, nan] | nan | nan |
| P1 | desired | uniform_dark_f150 | hermite / tukey | final_t_mm | 23 | 0.768 | [0.722, 0.801] | 4.77e-07 | 7.63e-06 |
| P1 | desired | uniform_dark_f150 | hermite / tukey | final_r_deg | 23 | 0.767 | [0.723, 0.801] | 4.77e-07 | 7.63e-06 |
| P1 | desired | uniform_dark_f250 | hermite / tukey | final_t_mm | 16 | 0.463 | [0.228, 0.688] | 3.05e-05 | 0.000336 |
| P1 | desired | uniform_dark_f250 | hermite / tukey | final_r_deg | 16 | 0.463 | [0.227, 0.688] | 3.05e-05 | 0.000336 |
| P1 | desired | uniform_dark_f350 | hermite / tukey | final_t_mm | 5 | nan | [nan, nan] | nan | nan |
| P1 | desired | uniform_dark_f350 | hermite / tukey | final_r_deg | 5 | nan | [nan, nan] | nan | nan |
| P1 | desired | uniform_dark_f450 | hermite / tukey | final_t_mm | 0 | nan | [nan, nan] | nan | nan |
| P1 | desired | uniform_dark_f450 | hermite / tukey | final_r_deg | 0 | nan | [nan, nan] | nan | nan |
| P2 | current | uniform_dark_f150 | hermite / tukey | success | 3 | -3 | hermite-only 0, tukey-only 3 | 0.25 | 0.5 |
| P2 | current | uniform_dark_f250 | hermite / tukey | success | 12 | -12 | hermite-only 0, tukey-only 12 | 0.000488 | 0.00293 |
| P2 | current | uniform_dark_f350 | hermite / tukey | success | 15 | -11 | hermite-only 2, tukey-only 13 | 0.00739 | 0.0222 |
| P2 | current | uniform_dark_f450 | hermite / tukey | success | 20 | -20 | hermite-only 0, tukey-only 20 | 1.91e-06 | 2.67e-05 |
| P2 | desired | uniform_dark_f150 | hermite / tukey | success | 8 | 0 | hermite-only 4, tukey-only 4 | 1 | 1 |
| P2 | desired | uniform_dark_f250 | hermite / tukey | success | 14 | 14 | hermite-only 14, tukey-only 0 | 0.000122 | 0.000854 |
| P2 | desired | uniform_dark_f350 | hermite / tukey | success | 23 | 23 | hermite-only 23, tukey-only 0 | 2.38e-07 | 4.05e-06 |
| P2 | desired | uniform_dark_f450 | hermite / tukey | success | 27 | 27 | hermite-only 27, tukey-only 0 | 1.49e-08 | 2.68e-07 |

## Final pose error per cell (median [IQR])

| interaction | scenario | weighting | n | translation (mm) | rotation (deg) | success | tail slope |
|---|---|---|---|---|---|---|---|
| current | uniform_dark_f150 | tukey | 32 | 0.451 [0.393, 0.575] | 0.0198 [0.0173, 0.0252] | 84 % | -0.48 |
| current | uniform_dark_f150 | hermite | 32 | 0.353 [0.333, 71.1] | 0.0155 [0.0146, 3.89] | 75 % | -0.10 |
| current | uniform_dark_f250 | tukey | 32 | 0.457 [0.328, 0.911] | 0.0201 [0.0144, 0.04] | 88 % | -1.95 |
| current | uniform_dark_f250 | hermite | 32 | 146 [0.243, 677] | 9.2 [0.0106, 29.4] | 50 % | -0.11 |
| current | uniform_dark_f350 | tukey | 32 | 0.392 [0.226, 1.89] | 0.0172 [0.00993, 0.0829] | 78 % | -3.18 |
| current | uniform_dark_f350 | hermite | 32 | 243 [0.119, 958] | 14.3 [0.00522, 42.6] | 44 % | -0.06 |
| current | uniform_dark_f450 | tukey | 32 | 0.273 [0.103, 310] | 0.012 [0.00452, 16.3] | 62 % | -4.06 |
| current | uniform_dark_f450 | hermite | 32 | 1.1e+03 [999, 1.17e+03] | 48.5 [44.7, 51.5] | 0 % | 0.38 |
| desired | uniform_dark_f150 | tukey | 32 | 0.45 [0.403, 0.577] | 0.0198 [0.0177, 0.0253] | 84 % | -0.47 |
| desired | uniform_dark_f150 | hermite | 32 | 0.34 [0.312, 0.371] | 0.0149 [0.0137, 0.0163] | 84 % | -0.10 |
| desired | uniform_dark_f250 | tukey | 32 | 146 [0.343, 1.26e+03] | 10.5 [0.0151, 57] | 50 % | -0.30 |
| desired | uniform_dark_f250 | hermite | 32 | 0.231 [0.211, 0.244] | 0.0101 [0.00925, 0.0107] | 94 % | -0.28 |
| desired | uniform_dark_f350 | tukey | 32 | 968 [553, 1.26e+03] | 55.7 [31, 97.3] | 16 % | 0.19 |
| desired | uniform_dark_f350 | hermite | 32 | 0.114 [0.107, 0.121] | 0.005 [0.00468, 0.0053] | 88 % | -0.58 |
| desired | uniform_dark_f450 | tukey | 32 | 1.26e+03 [1.21e+03, 3.03e+03] | 58.9 [46.8, 142] | 0 % | 0.33 |
| desired | uniform_dark_f450 | hermite | 32 | 0.0163 [0.0148, 0.0257] | 0.000711 [0.000645, 0.00113] | 84 % | -1.88 |

## Paired comparisons (ratio A/B, HL estimate [95 % CI]; Holm-adjusted p within family)

| family | interaction | scenario | A / B | outcome | n | ratio [95 % CI] | p (Holm) | equivalent (±25 %) |
|---|---|---|---|---|---|---|---|---|
| primary | current | uniform_dark_f150 | hermite / tukey | final_t_mm | 24 | 0.765 [0.704, 0.826] | 0.000262 | no |
| primary | current | uniform_dark_f150 | hermite / tukey | final_r_deg | 24 | 0.765 [0.703, 0.827] | 0.000262 | no |
| primary | current | uniform_dark_f250 | hermite / tukey | final_t_mm | 16 | 0.46 [0.364, 0.624] | 0.000336 | no |
| primary | current | uniform_dark_f250 | hermite / tukey | final_r_deg | 16 | 0.46 [0.364, 0.623] | 0.000336 | no |
| primary | current | uniform_dark_f350 | hermite / tukey | final_t_mm | 12 | 0.361 [0.255, 0.529] | 0.00293 | no |
| primary | current | uniform_dark_f350 | hermite / tukey | final_r_deg | 12 | 0.361 [0.255, 0.53] | 0.00293 | no |
| primary | current | uniform_dark_f450 | hermite / tukey | final_t_mm | 0 | n/a | nan | no |
| primary | current | uniform_dark_f450 | hermite / tukey | final_r_deg | 0 | n/a | nan | no |
| primary | desired | uniform_dark_f150 | hermite / tukey | final_t_mm | 23 | 0.768 [0.722, 0.801] | 7.63e-06 | no |
| primary | desired | uniform_dark_f150 | hermite / tukey | final_r_deg | 23 | 0.767 [0.723, 0.801] | 7.63e-06 | no |
| primary | desired | uniform_dark_f250 | hermite / tukey | final_t_mm | 16 | 0.463 [0.228, 0.688] | 0.000336 | no |
| primary | desired | uniform_dark_f250 | hermite / tukey | final_r_deg | 16 | 0.463 [0.227, 0.688] | 0.000336 | no |
| primary | desired | uniform_dark_f350 | hermite / tukey | final_t_mm | 5 | n/a | nan | no |
| primary | desired | uniform_dark_f350 | hermite / tukey | final_r_deg | 5 | n/a | nan | no |
| primary | desired | uniform_dark_f450 | hermite / tukey | final_t_mm | 0 | n/a | nan | no |
| primary | desired | uniform_dark_f450 | hermite / tukey | final_r_deg | 0 | n/a | nan | no |

## Does the final cost rank the accuracy? (Kendall tau within pose)

- current, uniform_dark_f150: median tau nan, negative in nan % of 0 poses
- current, uniform_dark_f250: median tau nan, negative in nan % of 0 poses
- current, uniform_dark_f350: median tau nan, negative in nan % of 0 poses
- current, uniform_dark_f450: median tau nan, negative in nan % of 0 poses
- desired, uniform_dark_f150: median tau nan, negative in nan % of 0 poses
- desired, uniform_dark_f250: median tau nan, negative in nan % of 0 poses
- desired, uniform_dark_f350: median tau nan, negative in nan % of 0 poses
- desired, uniform_dark_f450: median tau nan, negative in nan % of 0 poses
