# Chapter 4 — proposed improvements

**Basis.** The compiled chapter (`boutiba_V1_removed.pdf`, 37 pp., printed pp. 66–102),
the section sources and variant programs on this branch, and ViSP's `vpRobust.cpp`.
Every numerical statement below was recomputed; the computation is indicated.

Items are ordered by consequence for the defence. Part A lists errors of
substance; Part B adds theory that replaces assertions by statements with proof;
Part C ranks the experiments by value per hour; Parts D and E cover presentation
and reproducibility. Drop-in LaTeX for the principal corrections is in Part F.

---

## Summary

| # | Item | Sections | Kind | Cost |
|---|---|---|---|---|
| A1 | The damping "independent check" is an identity of the gain law | 4.7.2, 4.8 | error | text |
| A2 | Structure mechanism acts on measurements whose interaction rows vanish | 4.4.3, 4.4.4, 4.7.2–4.8 | error | text |
| A3 | Implemented estimator has 86.8 % Gaussian efficiency, not 95 % | 4.2.2, 4.3.1 | error | text |
| A4 | Tukey argument is median-centred in the implementation | 4.3.1, 4.3.2, 4.4.3 | error | text |
| A5 | $N_e$ undefined; four derived numbers wrong | 4.7.1, 4.7.2 | error | text |
| A6 | The "unexplained" 8-px band is an erosion artefact of the occluder measurement | 4.3.2, 4.7.3 | error | 1 measurement |
| A7 | Numerical and consistency slips | 4.7.2, 4.7.3, 4.8 | error | text |
| B1 | Lemma replacing "the row norm factorises" | 4.4.1 | theory | text |
| B2 | Stability proposition without the slow-variation assumption | 4.5 | theory | text |
| B3 | Convergence-rate corollary explaining the stall | 4.5, 4.7.1, 4.7.2 | theory | text |
| B4 | Positioning as a Schweppe-type GM-estimator; relation to direct SLAM | 4.1, 4.4 | scholarship | text |
| C1–C7 | Experiments, ranked | 4.6, 4.7 | experiment | 10 min – 10 h |
| D | Figures, length, register, notation | all | presentation | text |
| E | Source consolidation, committed V3 is functionally V2 | — | reproducibility | 1 h |

---

## A. Errors of substance

### A1. The damping check in 4.7.2 (p. 95) and 4.8 (p. 102) is circular

The implementation (`herpvs_v3_hermite.cpp`, loop body) computes

```cpp
lambda = pow(10, (      log10(normeErrorI) - 0.8*log10(Ne)));   // S / Ne^0.8
mu     = pow(10, (5.0 * log10(normeErrorI) - 5.2*log10(Ne)));   // S^5 / Ne^5.2
```

with $S=\lVert\mathbf{e}_I\rVert^2$. Eliminating $S$ gives, **identically**,

$$\mu=\lambda^{5}\,N_e^{-1.2}.$$

Hence the damping ratio between two runs is the fifth power of the gain ratio
multiplied by $(N_e^{\mathrm{occ}}/N_e^{\mathrm{nom}})^{-1.2}$, whatever the
closed-loop behaviour. Verification:

| quantity | value |
|---|---|
| $N_e^{\mathrm{nom}}$, $N_e^{\mathrm{occ}}$ (inverted from the printed $5.23\log_{10}N_e$ = 45.0095, 45.0219) | $4.0367\times10^{8}$, $4.0588\times10^{8}$ |
| nominal $\mu$ predicted from $\lambda=0.0407037$ | $5.2595\times10^{-18}$ (logged $5.2593\times10^{-18}$) |
| occluded V1 $\mu$ predicted from $\lambda=2.33425$ | $3.241\times10^{-9}$ (logged $3.24\times10^{-9}$) |
| $(N_e^{\mathrm{occ}}/N_e^{\mathrm{nom}})^{-1.2}-1$ | $-0.65\,\%$ |
| resulting "damping ratios" | $6.162, 4.510, 4.500\times10^{8}$ (text: 6.16, 4.51, 4.50) |
| "recovered exponent" | 9.9968, 9.9967, 9.9967 (text: 9.997) |

The 0.62 % residual and the exponent 9.997 are therefore the small difference in
the initial intensity error between the nominal and occluded runs, and the
agreement "across three runs, over nine orders of magnitude" verifies that the
program implements (3.28)–(3.29), not that the stall mechanism is correct. The
sentences "the account above is not an interpretation placed on the data but a
relation the data satisfy" (p. 95) and "The relation is not an interpretation
placed on the data … 9.997 against the 10 of (3.29)" (p. 102) must be removed.
The same argument is relied upon in `ch5_plan_assessment.md` §1 and must be
removed there too.

The mechanism itself is sound; B3 supplies a derivation and a genuinely
independent check. Replacement text: F1.

A related overstatement: "holding $\lambda$ up — by whatever means — improved
the final accuracy by three orders of magnitude" (p. 95) compares a nominal with
an occluded scene, so the gain is not the only variable changed. State it as
consistent with the analysis and defer the intervention (fixed gain on the
nominal scene) to Chapter 5, whose Arm A performs it.

### A2. The flat-region mechanism of 4.4.3 is inert with the current-image interaction matrix

The interaction matrix is recomputed at every iteration **from the current
image**: `Ihermite()` builds `vvx … vwz` from `getGradX(I)`, `getGradY(I)`, filters
them with $D_k$, and the loop calls `sI.buildFrom(I, …)` then
`sI.interaction(Lsd)`. Consequently, for every measurement $j$ whose support lies
inside a region where the current image is uniform, $\boldsymbol{\ell}_j=\mathbf{0}$
exactly, and its contributions to the control law,
$w_j^{2}e_j\boldsymbol{\ell}_j^{\top}$ to $\mathbf{L}_R^{\top}\mathbf{W}^{2}\mathbf{e}$
and $w_j^{2}\boldsymbol{\ell}_j^{\top}\boldsymbol{\ell}_j$ to $\mathbf{H}_{\mathbf{W}}$,
vanish **for every** $w_j\in[0,1]$. Their weight influences $\mathbf{V}_c$ only
through the robust scale, to which they contribute a fraction of the sample.

Implications:

1. The interior of the uniform occluder — the solid black region of
   Fig. 4.10(d) — is inert in all three variants. Its rejection is visible but
   has no effect on the commanded velocity. The mechanism stated in 4.4.3
   ("$\bar G$ meets the floor, the residual is inflated tenfold, and the
   measurement is rejected. … This is the mechanism by which the weighting acts
   on occlusion") therefore describes an effect without consequence for the
   control.
2. The occluder biases the control only through the band within the
   gradient-operator plus Hermite-kernel half-width of its boundary, where the
   current image contains the occluder's own contour. There $G$ is **large**,
   $\bar G>1$, and Variant 3 is **more lenient** than Variant 2.
3. The observed tie between Variants 2 and 3 under occlusion is therefore
   predicted structurally, not merely unresolved statistically. This converts the
   chapter's weakest point into an explained result.
4. The defence in 4.4.4 (p. 80: "discards little information in the case where
   the flatness is genuine, while removing a substantial source of bias in the
   case where it is not") is internally inconsistent: the bound of B1 that limits
   the information carried by a flat region limits its bias by the same factor,
   whether the flatness is genuine or due to occlusion.
5. Where the structure normalisation *can* act: (i) the reallocation of the
   outlier budget among textured measurements, which affects the early
   over-rejection of 4.3.3 (the Variant 2 weight map, C1, decides this);
   (ii) configurations in which the interaction matrix is not taken from the
   current image — the desired-image matrix $\mathbf{L}^{*}$ or the mean
   $\tfrac12(\mathbf{L}+\mathbf{L}^{*})$, both standard in photometric servoing —
   where the occluder interior carries non-zero rows (C4).

If Chapter 3 states that the interaction matrix is evaluated at the desired
image, the text and the implementation disagree and one must be changed. The
variable name `Lsd` and its comment ("matrice d'interaction a la position
desiree") suggest that this was once the case.

Replacement text: F4. Supporting figure: the influence map of C2.

### A3. The efficiency justification does not apply to the implemented estimator

Section 4.2.2 applies $w_j$ to both $\mathbf{L}_R$ and $\mathbf{e}$, so the normal
equations carry $w_j^{2}$, and states that "nothing depends on" this choice. The
fixed point of the implemented iteration is the M-estimator with influence
function

$$\tilde\psi(u)=w^{\mathrm T}(u)^{2}\,u=u\left(1-\frac{u^{2}}{c^{2}}\right)^{4}\mathbb{1}_{\lvert u\rvert\le c}.$$

The rejection point is unchanged, but the Gaussian efficiency is not
(numerical quadrature, $\mathrm{eff}=\mathbb{E}[\tilde\psi']^{2}/\mathbb{E}[\tilde\psi^{2}]$):

| influence function | $c=4.6851$ | $c$ for 95 % |
|---|---|---|
| $u(1-u^2/c^2)^2$ (the biweight, as justified in 4.3.1) | 0.950 | 4.685 |
| $u(1-u^2/c^2)^4$ (as implemented) | **0.868** | 6.291 |

Replace "rather than because anything depends on it" (p. 71) with the statement
that the implemented estimator is the redescending M-estimator
$\tilde\psi$, whose Gaussian efficiency at $c=4.6851$ is 86.8 %, and amend the
95 % sentence of 4.3.1 accordingly. Since 4.3.1 already concedes that the
Gaussian model does not hold, the conclusion is unaffected; the claim is.

### A4. The Tukey argument is median-centred

`vpRobust::MEstimator` (ViSP, `modules/core/src/math/robust/vpRobust.cpp`,
lines 145–167 on `master`) computes `m_normres[i] = fabs(residues[i] - med)` and
passes it to `psiTukey`. The implemented argument is therefore

$$u_j=\frac{e_j-\operatorname{med}_k e_k}{\hat s},$$

not $e_j/\hat s$ as in (4.7), (4.9) and (4.15). Correct the three equations
(for Variant 3, $\tilde e_j-\operatorname{med}_k\tilde e_k$). The inertness of a
uniform rescaling argued in 4.4.3 still holds. Confirm the line numbers against
the ViSP release used, since the API changed within 3.x.

### A5. $N_e$ is undefined, and four derived numbers are wrong

$N_e$ is the intensity error $\lVert\mathbf{e}_I\rVert^2$ captured at the first
iteration — a normalising constant, not a count. With
$\bar e=\lVert\mathbf{e}_I\rVert^2/N_e$ the law is exactly

$$\lambda=N_e^{0.2}\,\bar e,\qquad \mu=N_e^{-0.2}\,\bar e^{\,5}.$$

| p. | printed | correct | reason |
|---|---|---|---|
| 88 | "$\lambda\simeq53$ at the first iteration" (as an observation) | $\lambda=N_e^{0.2}=52.6$ by construction | $\bar e=1$ |
| 88 | "from 72.7 … grey levels" | 72.5 | $\sqrt{4.0367\times10^{8}/76\,800}$ |
| 88 | "to 2.5 grey levels" | **2.0** | $72.5/\sqrt{52.63/0.0407}$ |
| 95 | printed "Ne = 45.02" (console) | $N_e=4.06\times10^{8}$ | the program prints $5.23\log_{10}N_e$ |

The console line `std::cout << "Ne       =    " << (5.23*log10(Ne))` should print $N_e$ itself; it
caused the earlier false diagnosis of a "missing constant".

### A6. The 8-pixel band of 4.7.3 is an artefact of how the occluder was measured

The 5.80 % coverage was measured as "the connected region where $G$ vanishes"
(3 829 positions, equivalent radius 34.9 px); the largest connected rejected
region has 5 886 positions (43.3 px). But $G(p)=0$ requires the whole
$9\times9$ support of the first-order kernels to lie inside the uniform region,
so that region is the occluder **eroded** by the 4-px half-width. Correcting for
the erosion gives an occluder radius of about 38.9 px and a rejection band of
about 4.4 px — the filter half-width predicted in 4.3.2. Consequently:

- 4.7.3 "The measured band is wider than that argument accounts for, and the
  remainder is not explained here" becomes a confirmed prediction;
- 4.3.2 ("two filter half-widths", "factor of about 1.5") and 4.7.3 ("factor of
  1.6", "eight pixels") currently disagree with each other; both are replaced;
- the occluded fraction at the desired pose is then about 7.2 %, not 5.80 %,
  and the 7.4 % at the initial pose may be underestimated likewise.

Action: count the occluder directly from the mask (render the occluded and clean
textures at the same pose and take the support of their difference), on the same
basis (positions or measurements) as the rejected fraction. Until then the
erosion account is a hypothesis.

### A7. Numerical and consistency slips

| p. | printed | correct |
|---|---|---|
| 92 | "a factor of between four and five hundred in translation" | 410–523 ($1293.1/3.156$, $1293.1/2.474$) |
| 101 | "against **a third** of the clean scene's valid measurements carried below a quarter weight" | 13.6 %, i.e. about **one in seven** (as 4.3.3 says) |
| 98 | Table 4.3: "Tukey weights of Variant 3 over the 66000 measurements"; "disc covers 7.4 %" | $w^{\mathrm H}$; state the basis (A8); both coverages |
| 99 | Fig. 4.10: "Tukey weights", "mid-grey denotes the filter border" | $w^{\mathrm H}$; border excluded (the corrected caption exists in `ch4_sec7_3_weightmaps_corrected.tex` but was not applied) |
| 84 vs 98 | 4.6.2: percentages are fractions of $P$; Table 4.3: of 66 000 | `saveMaps()` stores the **mean of the four channel weights** per position, $\bar w(p)=\tfrac14\sum_k w_{(k,p)}$; define it and use one basis |
| 68, 71 | "the scheme of Chapter 3 **exactly**" (4.1, Remark) | "up to the floor $\varepsilon$" (as 4.8 already says) |
| 73, 97 | "disc" / "elliptical region" | one term |

---

## B. Theory to add (text only)

### B1. Lemma for 4.4.1

"The norm of that row factorises into the local gradient magnitude and a purely
geometric term" is false as an equality: the row is
$\nabla I^{\top}\mathbf{M}(x,y)$ with $\mathbf{M}\in\mathbb{R}^{2\times6}$, and its norm
depends on the orientation of $\nabla I$. What holds is a two-sided bound, which
suffices for the argument and is tight here:

- $\sigma_{\min}(\mathbf{M})\lVert\nabla I\rVert\le\lVert\mathbf{L}_I\rVert\le\sigma_{\max}(\mathbf{M})\lVert\nabla I\rVert$;
- at the principal point $\mathbf{M}\mathbf{M}^{\top}=(1+Z^{-2})\mathbf{I}_2$, so equality holds there;
- over the $300\times220$ interior with $f=870$ px, $(u_0,v_0)=(160,120)$ and
  $Z=1.3$ m (the implementation's constants), $\max\sigma_{\max}/\sigma_{\min}=1.023$ (computed).

Two further precisions: the rows of $\mathbf{L}_R$ are $D_k\ast\mathbf{L}_I$, so
the image-dependent factor is the gradient magnitude of the $k$-th **feature
map**; $G$ reproduces it for the zeroth-order component of the summed kernel and
is a proxy for the rest. And the row norm mixes translational and rotational
units, so the metric must be stated. The row norms themselves are already
computed at no cost; using them (or the leverages) would make the argument exact
(C7). Drop-in: F2.

### B2. Stability proposition for 4.5

The present 4.5 asserts local stability "with the weights held fixed" and rests
the time-varying case on an unverified slow-variation assumption. Near the
desired pose a stronger statement is available. With
$\mathbf{K}_{\mathbf{W}}=\mathbf{H}_{\mathbf{W}}+\mu\operatorname{diag}\mathbf{H}_{\mathbf{W}}+\varepsilon\mathbf{I}_6$,

$$\mathbf{K}_{\mathbf{W}}^{-1}\mathbf{H}_{\mathbf{W}}=\mathbf{I}_6-\boldsymbol{\Delta}_{\mathbf{W}},\qquad
\lVert\boldsymbol{\Delta}_{\mathbf{W}}\rVert\le\delta_{\mathbf{W}}:=\frac{\mu\max_i(\mathbf{H}_{\mathbf{W}})_{ii}+\varepsilon}{\lambda_{\min}(\mathbf{H}_{\mathbf{W}})}.$$

In the nominal case $\mathbf{e}(\mathbf{r}^{*})=\mathbf{0}$, so to first order
$\delta\dot{\mathbf{r}}=-\lambda(\mathbf{I}-\boldsymbol{\Delta}_{\mathbf{W}})\delta\mathbf{r}$
and $V=\tfrac12\lVert\delta\mathbf{r}\rVert^{2}$ satisfies
$\dot V\le-\lambda(1-\delta_{\mathbf{W}})\lVert\delta\mathbf{r}\rVert^{2}$ **for any
weighting sequence**, provided $\sup_t\delta_{\mathbf{W}(t)}<1$. The
weights enter only through the rank condition. With $\mu\to10^{-18}$ the
condition reduces to $(\operatorname{tr}\mathbf{H}_{\mathbf{W}}/6)/\lambda_{\min}(\mathbf{H}_{\mathbf{W}})<10^{6}$,
which ties the floor (4.5) directly to stability and is checkable by logging
$\lambda_{\min}(\mathbf{H}_{\mathbf{W}})$ (C5). It uses that the controller's matrix
equals the Jacobian near $\mathbf{r}^{*}$, which holds in this simulation (planar
fronto-parallel target at the constant $Z=1.3$ m of the code). Under occlusion
the same argument applies about the equilibrium $\bar{\mathbf{r}}$ of the weighted
law, and the accuracy reported in Table 4.2 is the bias
$\lVert\bar{\mathbf{r}}-\mathbf{r}^{*}\rVert$ of each estimator. Drop-in: F3.

### B3. Convergence-rate corollary: the stall, derived and checked independently

Near $\mathbf{r}^{*}$ in the nominal case $\lVert\mathbf{e}_I\rVert^{2}\approx\delta\mathbf{r}^{\top}\mathbf{Q}\,\delta\mathbf{r}$,
hence $\lambda\approx N_e^{-0.8}\,\delta\mathbf{r}^{\top}\mathbf{Q}\,\delta\mathbf{r}$ and, with
the sampling period $T=0.04$ s,
$\delta\mathbf{r}_{k+1}\approx\bigl(1-TN_e^{-0.8}\rho_k\bigr)\delta\mathbf{r}_k$,
$\rho_k=\delta\mathbf{r}_k^{\top}\mathbf{Q}\,\delta\mathbf{r}_k$. Then
$1/\rho_{k+1}\approx1/\rho_k+2TN_e^{-0.8}$, so

$$\lVert\delta\mathbf{r}_k\rVert=O(k^{-1/2}),\qquad \lVert\mathbf{V}_{c,k}\rVert=O(k^{-3/2}):$$

asymptotic but not exponential convergence. Under occlusion $\bar e\to\bar e_\infty\simeq4.17\times10^{-2}$,
$\lambda\to\lambda_\infty\simeq2.19$, and the contraction is geometric at
$1-T\lambda_\infty=0.912$ per iteration.

Independent checks from data already recorded:

- **Nominal, Variant 1.** The pose error at iteration 284 (commit `cc3ba1a`:
  0.561 mm, 0.332°) and at 600 (0.337 mm, 0.219°) fall by 1.66 and 1.52, against
  $\sqrt{600/284}=1.45$ predicted by the $O(k^{-1/2})$ law. Any constant gain in
  the observed range would predict a reduction by many orders of magnitude.
  The simulation is deterministic, but confirm that the two console captures
  come from the same binary before quoting them; a per-iteration log (C5) makes
  the check a slope of $-1/2$ on a log–log plot.
- **Occluded, Variant 1.** Pose unchanged to six significant figures between
  iterations 400 and 600, as a contraction of 0.912 per iteration requires
  ($0.912^{200}\approx10^{-8}$). The final errors of Table 4.2 are therefore
  equilibrium biases, not truncated convergence; "until it meets the limits of
  the simulation" (p. 94) should say so. Variant 2's 15 % change over the same
  interval is then drift of the equilibrium with the weights, i.e. the same
  phenomenon as Variant 3's terminal jitter.

This replaces the circular check of A1 with a derivation plus two independent
confirmations, and gives Chapter 5 a precise motivation. Drop-in: F1.

### B4. Positioning in the literature

- **Generalised M-estimation.** Normalising the residual by a function of the
  design row before applying $\psi$ is a Schweppe-type GM-estimator (Handschin,
  Schweppe, Kohlas, Fiechter, IEEE Trans. PAS, 1975; Hampel, Ronchetti,
  Rousseeuw, Stahel, *Robust Statistics*, 1986; Maronna, Martin, Yohai,
  *Robust Statistics*, 2006/2019). Variant 3 is of that type with design
  function $\bar G$. The direction is the reverse of classical bounded-influence
  estimation, which down-weights high-leverage rows; the chapter must justify the
  reversal. The justification is a heteroscedastic residual model: with
  $e_j\approx\boldsymbol{\ell}_j\,\delta\mathbf{r}+n_j$,
  $\operatorname{Var}e_j\approx\sigma_n^{2}+s^{2}\lVert\boldsymbol{\ell}_j\rVert^{2}$.
  Normalising by $\lVert\boldsymbol{\ell}_j\rVert\propto G$ is correct while the
  pose error dominates ($s\gg\sigma_n$); normalising uniformly (Variant 2) is
  correct at convergence. The floor $\max(\bar G,\bar G_{\min})\approx\sqrt{\bar G^{2}+\bar G_{\min}^{2}}$
  is a crude, fixed version of the $\sigma_n$ term. This framing predicts
  that Variant 3 should help early and not at convergence — consistent with its
  larger terminal jitter and translational error — and motivates an adaptive
  floor (C7).
- **Direct visual odometry and SLAM.** LSD-SLAM (Engel, Schöps, Cremers,
  ECCV 2014) normalises photometric residuals by a propagated variance with a
  gradient-dependent term; DSO (Engel, Koltun, Cremers, IEEE TPAMI 2018) uses
  the gradient-dependent weight $c^{2}/(c^{2}+\lVert\nabla I\rVert^{2})$, which
  **down-weights** high-gradient pixels. Both address the same
  heteroscedasticity as Variant 3; the second does so in the opposite
  direction. An examiner from vision will raise them.
- **Visual servoing.** Comport, Marchand, Chaumette, "Statistically robust 2-D
  visual servoing", IEEE T-RO 2006 (the $\mathbf{D}$-weighted law and its
  stability discussion); Collewet, Marchand, "Photometric visual servoing",
  IEEE T-RO 2011; Dame, Marchand, mutual-information servoing, IEEE T-RO 2011
  (an alternative route to occlusion and illumination robustness).

Verify each reference against the bibliography before use.

---

## C. Experiments, ranked by value per hour

| # | Experiment | Tests | Cost |
|---|---|---|---|
| C1 | Weight maps for Variant 2, nominal and occluded, iterations 1 and 600; per-channel fractions $w<0.25$ | the only comparative evidence possible for the mechanism (A2.5-i); 4.3.3; 4.4.4 | 2 runs, ~20 min |
| C2 | Influence maps $\lVert w_j^{2}e_j\boldsymbol{\ell}_j\rVert$ (sum over the four channels), V1–V3, occluded, iteration 600 | A2: where the bias enters (expected: the boundary band, not the interior) | same runs; `Lp[i][·]*error_p[i]` is already in the loop |
| C3 | Multi-pose campaign: 10 poses (reuse Chapter 5's table) × 3 variants × 2 configurations = 60 runs | ranking; replaces the single-run caveat (5 occurrences); median, IQR, worst case, number converged; paired Wilcoxon signed-rank V2 vs V3 (minimum two-sided $p=0.002$ at $n=10$) | 4–10 h unattended |
| C4 | Occluder design: (i) textured occluder; (ii) uniform occluder with $\mathbf{L}^{*}$ or $\tfrac12(\mathbf{L}+\mathbf{L}^{*})$; (iii) fraction sweep 5–40 %; (iv) bright uniform occluder | (i) the limitation stated in 4.4.4 but untested; (ii) the only configuration where the flat-region mechanism is active; (iii) the breakdown argument of 4.3.2; (iv) the claim "suppressed identically" of 4.4.3 | 2–4 runs each, 1 pose; more with C3 |
| C5 | Per-iteration CSV: pose error, $\lVert\mathbf{V}_c\rVert$, $\lVert\mathbf{e}\rVert^{2}$, $\lVert\mathbf{e}_I\rVert^{2}$, $\lambda$, $\mu$, $\lambda_{\min}(\mathbf{H}_{\mathbf{W}})$, $\operatorname{tr}\mathbf{H}_{\mathbf{W}}$, $\lVert\mathbf{W}_k-\mathbf{W}_{k-1}\rVert_F/\lVert\mathbf{W}_k\rVert_F$, rejection fractions | B2 condition; B3 slope; 4.5 slow-variation; vector figures (D) | code only |
| C6 | Timing of the weight computation with `std::chrono`, display off, idle machine, 3 repetitions | the "a fifth of the filtering" claim of 4.4.1 (currently an operation count) | 30 min; requires the `init()` shadowing fix |
| C7 | Optional method variants: adaptive floor $\bar G_{\min}(t)=\hat\sigma_n/\hat s_{\text{pose}}(t)$; row-norm or leverage $h_j=\boldsymbol{\ell}_j\mathbf{H}^{-1}\boldsymbol{\ell}_j^{\top}$ in place of $G$ | B4 prediction; exact form of B1 at no extra convolution | 1 variant, rerun C3 |

The gain intervention (fixed or floored $\lambda$ on the nominal scene) is the
causal test of B3; it is Arm A/A′ of Chapter 5 and should be forward-referenced
rather than duplicated.

---

## D. Presentation

1. **Figures.** Nine three-panel ViSP screenshots (Figs. 4.3–4.5, 4.7–4.9)
   become two vector figures (nominal, occluded), each with three panels —
   $\lVert\mathbf{V}_c\rVert$, pose-error norms in mm and deg, $\lVert\mathbf{e}\rVert^{2}$ on
   a log axis — with the three variants overlaid. The present captions state
   "(m and rad)" and "$\lVert\mathbf{e}\rVert=\lVert\mathbf{R}-\mathbf{R}^{*}\rVert$"
   while the text reports $\lVert\mathbf{e}\rVert^{2}$ and the tables mm, µm and
   degrees. Figs. 4.2(d) and 4.6(e) show one panel for several variants.
2. **Length.** 14 260 words over 37 pages for six runs. The argument that the
   feature error is not an accuracy criterion appears seven times (4.2.1, 4.6.4,
   4.7.1 ×2, 4.7.2 ×2, 4.8): state it once in 4.6.4 and cite it. The conclusion
   (3.5 pp.) should be 1–1.5 pp. Target: 25–28 pp.
3. **Register.** Remove evaluative intensifiers: "is not in doubt" (×2), "the
   clearest demonstration in this thesis" (×2), "decisive" (×4), "not merely"
   (×5), "It should be said" (×2), "It is worth noting" (×3).
4. **Contributions.** End 4.1 with an enumerated list of contributions, as IEEE
   papers do, and let 4.8 answer it item by item.
5. **Notation.** The weight matrix $\mathbf{D}$ collides with the kernels
   $\mathbf{D}^{\sigma}_{n,m}$, and $\mathbf{D}^{\mathrm T}$ reads as a transpose.
   Rename to $\mathbf{W}$, $\mathbf{W}^{\mathrm T}\to\mathbf{W}_{\mathrm T}$,
   $\mathbf{W}_{\mathrm H}$.
6. **Cross-references and typesetting** (printed page numbers):

| p. | defect |
|---|---|
| 68 ff. | "Chapter 2.11" (13×): `\label{ch:herpvs}` binds to a section counter; place it immediately after `\chapter{…}` |
| 69 | "Chapters and 2.11": empty `\ref{chap:Fundamentals-of-Visual-Servoing}`; "Section ?? reports the results; and Section ?? concludes" |
| 70 | $\varepsilon$ detached from its sentence ("and is a fixed floor …") and placed after it as (4.5) |
| 71, 75, 83, 86 | remarks typeset as "[title]"; both resolve to "Remark 4.2.2"; define `\newtheorem{remark}{Remark}[section]` under `\theoremstyle{remark}` |
| 73, 74, 76 (×2), 77, 78, 82 (×2), 83 | "??" (11 in total) |
| 76 | "Section ?? records what happens when it is used" — no full stop |
| 77 | "Their responses, are accordingly" |
| 78 | "quantified in Section ??." — sentence truncated; the scale-energy table (`ch4_scale_energy.tex`) is in neither chapter |
| 81 | "favours the rejection.ld defeat any scheme …" — corrupted paste; the paragraph is then repeated |
| 83 | "definition ofnthe" |
| 88 | "[? ]" |
| 92 | "((a)" |
| 99 | Fig. 4.10 floats into 4.8 |
| 101 | "Section 4.7.3show" |

---

## E. Source hygiene and reproducibility

1. **Consolidate.** The chapter exists as 27 fragments with superseded versions
   side by side (4.7.1 ×3, 4.4.2 ×2, 4.7.2 ×2, 4.7.3 ×2, tables ×2), each carrying
   long `%%%` review notes. The PDF shows that several applied corrections were
   lost in transit (Table 4.3 and Fig. 4.10 captions, 4.4.2 fragments, the p. 81
   corruption). Produce one `chapter4.tex` from the canonical files —
   `sec1_introduction`, `sec2_1`, `sec2_2`, `sec3_1`–`sec3_3`, `sec4_1`,
   `sec4_2_structure_measure_v2`, `sec4_3`, `sec4_4`, `sec5`, `sec6_1`, `sec6_2`,
   `sec6_3_4`, `sec7_1_nominal_corrected`, `sec7_2_occlusion_written`,
   `sec7_3_weightmaps_corrected`, `sec8` — strip the review notes, and compile
   until the log reports no undefined references.
2. **The committed Variant 3 is functionally Variant 2.** In
   `herpvs_v3_hermite.cpp`, `computeHermiteNormalizedResidual()` builds $G$ from
   the summed kernels `w_h, w_v, w_d`, and returns the unnormalised residual when
   `error.getRows() != nbPixels`, with `nbPixels` $=76\,800$ against $264\,000$
   rows. The chapter's numbers come from the patched local program
   (`tutorial-viewer15.cpp` per `ch5_which_code_and_patch.md`). Commit that file
   and tag the revision that produced Tables 4.1–4.3.
3. **Command-line options are discarded.** `init()` re-declares the display,
   click and iteration-count options as locals, so `-d` has no effect
   (`ch5_which_code_and_patch.md` §1). Required for C3 and C6.

---

## F. Drop-in LaTeX

### F1. Replaces "The measurements confirm this, and admit an independent check …" (4.7.2, p. 95) and the corresponding sentences of 4.8

```latex
The gain law makes the mechanism explicit. Writing
$\bar{e}=\norm{\mathbf{e}_I}^{2}/N_e$ for the intensity error normalised by its
value $N_e$ at the first iteration, \eqref{eq:lambda}--\eqref{eq:mu} read
\begin{equation}
    \lambda=N_e^{0.2}\,\bar{e},
    \qquad
    \mu=N_e^{-0.2}\,\bar{e}^{\,5}.
    \label{eq:gain-normalised}
\end{equation}
In the nominal case $\bar{e}\to0$ and the gain vanishes with the error. Under
occlusion $\bar{e}$ tends to a positive limit, $\bar{e}_\infty\simeq4.2\times10^{-2}$,
fixed by the residual that the occluder leaves wherever the reference has no
counterpart, and the gain tends to $\lambda_\infty\simeq2.2$, against
$\lambda=0.0407$ at the end of the nominal run.
Proposition~\ref{prop:rate} states the consequence: the pose error decays as
$O(k^{-1/2})$ in the first case and geometrically, with ratio
$1-T\lambda_\infty\simeq0.91$ per iteration, in the second. Two observations
not used in deriving it agree with it. Between iterations~$284$ and~$600$ the
nominal Variant~1 run reduces its pose error by factors of $1.52$ in rotation
and $1.66$ in translation, against $\sqrt{600/284}=1.45$ for the
$O(k^{-1/2})$ law and many orders of magnitude for a geometric decay at any
gain in the observed range. Under occlusion the pose of Variant~1 is unchanged
to six significant figures between iterations~$400$ and~$600$, as a
contraction of $0.91$ per iteration requires; the errors of
Table~\ref{tab:Occlusion_pose} are therefore the equilibrium biases of the
three estimators rather than truncated convergence. The damping carries no
independent information: by~\eqref{eq:gain-normalised},
$\mu=\lambda^{5}N_e^{-1.2}$ identically. That a gain which does not vanish with
the error removes the stall is consistent with this analysis; the direct test,
in which the gain is held up on the nominal scene, is the subject of
Chapter~\ref{ch:gain-scheduling}.   % label of Chapter 5, to be set
```

### F2. Lemma for 4.4.1 (replaces "The norm of that row therefore factorises …")

```latex
\begin{lemma}[Image-dependent factor of an interaction row]
\label{lem:row-norm}
At a pixel of normalised coordinates $(x,y)$ and depth $Z$, the luminance
interaction row is $\mathbf{L}_I=\nabla I^{\top}\mathbf{M}(x,y)$ with
\begin{equation*}
    \mathbf{M}(x,y)=
    \begin{bmatrix}
        1/Z & 0 & -x/Z & -xy & 1+x^{2} & -y\\
        0 & 1/Z & -y/Z & -(1+y^{2}) & xy & x
    \end{bmatrix},
\end{equation*}
and
\begin{equation}
    \sigma_{\min}\bigl(\mathbf{M}\bigr)\,\norm{\nabla I}
    \;\le\;\norm{\mathbf{L}_I}\;\le\;
    \sigma_{\max}\bigl(\mathbf{M}\bigr)\,\norm{\nabla I}.
    \label{eq:row-bound}
\end{equation}
\end{lemma}
\begin{proof}
$\norm{\mathbf{M}^{\top}\mathbf{g}}^{2}=\mathbf{g}^{\top}\mathbf{M}\mathbf{M}^{\top}\mathbf{g}$,
and the eigenvalues of $\mathbf{M}\mathbf{M}^{\top}$ are
$\sigma_{\min}^{2}$ and $\sigma_{\max}^{2}$.
\end{proof}
At the principal point $\mathbf{M}\mathbf{M}^{\top}=(1+Z^{-2})\mathbf{I}_2$ and
\eqref{eq:row-bound} holds with equality. For the camera and depth of
Section~\ref{sec:ablation-conditions} the ratio
$\sigma_{\max}/\sigma_{\min}$ does not exceed $1.023$ over the interior region,
so the row norm equals the gradient magnitude times a geometric factor to within
$2.3\%$. Since the rows of $\mathbf{L}_R$ are the Hermite-filtered rows
$\mathbf{D}_k\ast\mathbf{L}_I$ and $\mathbf{M}$ varies negligibly over the
$9\times9$ support, the same holds at each scale with $\nabla I$ replaced by the
gradient of the corresponding feature map, of which the measure $G$
of~\eqref{eq:structG} is a scale-aggregated approximation. The Euclidean norm on
$(\mathbf{v},\boldsymbol{\omega})$ in SI units is used throughout.
```

### F3. Proposition for 4.5 (replaces the paragraph on time variation)

```latex
\begin{proposition}[Local convergence under arbitrary reweighting]
\label{prop:reweighting}
Let $\mathbf{K}_{\mathbf{W}}=\mathbf{H}_{\mathbf{W}}+\mu\,\mathrm{diag}(\mathbf{H}_{\mathbf{W}})+\varepsilon\mathbf{I}_{6}$
and
\begin{equation}
    \delta_{\mathbf{W}}=\frac{\mu\max_{i}(\mathbf{H}_{\mathbf{W}})_{ii}+\varepsilon}
                            {\lambda_{\min}(\mathbf{H}_{\mathbf{W}})}.
\end{equation}
Then $\mathbf{K}_{\mathbf{W}}^{-1}\mathbf{H}_{\mathbf{W}}=\mathbf{I}_{6}-\boldsymbol{\Delta}_{\mathbf{W}}$
with $\norm{\boldsymbol{\Delta}_{\mathbf{W}}}\le\delta_{\mathbf{W}}$. If
$\mathbf{e}(\mathbf{r}^{*})=\mathbf{0}$, the interaction matrix used equals the
Jacobian of $\mathbf{e}$ in a neighbourhood of $\mathbf{r}^{*}$, and
$\delta_{\mathbf{W}(t)}\le\bar{\delta}<1$ along the trajectory, then
$V=\tfrac12\norm{\delta\mathbf{r}}^{2}$ satisfies, to first order,
\begin{equation}
    \dot V\le-\lambda\,(1-\bar{\delta})\,\norm{\delta\mathbf{r}}^{2}
\end{equation}
for every sequence of weights.
\end{proposition}
\begin{proof}
$\mathbf{K}_{\mathbf{W}}^{-1}\mathbf{H}_{\mathbf{W}}
=\mathbf{I}_{6}-\mathbf{K}_{\mathbf{W}}^{-1}\bigl(\mu\,\mathrm{diag}(\mathbf{H}_{\mathbf{W}})+\varepsilon\mathbf{I}_{6}\bigr)$
and $\lambda_{\min}(\mathbf{K}_{\mathbf{W}})\ge\lambda_{\min}(\mathbf{H}_{\mathbf{W}})$.
With $\mathbf{e}=\mathbf{L}_R\,\delta\mathbf{r}+O(\norm{\delta\mathbf{r}}^{2})$,
\eqref{eq:robust-lm-law} gives
$\delta\dot{\mathbf{r}}=-\lambda(\mathbf{I}_{6}-\boldsymbol{\Delta}_{\mathbf{W}})\delta\mathbf{r}+O(\norm{\delta\mathbf{r}}^{2})$,
and $\delta\mathbf{r}^{\top}(\mathbf{I}_{6}-\boldsymbol{\Delta}_{\mathbf{W}})\delta\mathbf{r}
\ge(1-\norm{\boldsymbol{\Delta}_{\mathbf{W}}})\norm{\delta\mathbf{r}}^{2}$.
\end{proof}
The weights enter only through $\lambda_{\min}(\mathbf{H}_{\mathbf{W}})$: no
assumption on their rate of variation is needed. Once $\mu$ is negligible the
condition reads
$(\operatorname{tr}\mathbf{H}_{\mathbf{W}}/6)/\lambda_{\min}(\mathbf{H}_{\mathbf{W}})<10^{6}$
by~\eqref{eq:hessian-floor}, which the floor thereby links directly to
stability. Under occlusion $\mathbf{e}(\mathbf{r}^{*})\neq\mathbf{0}$ and the
statement applies about the equilibrium $\bar{\mathbf{r}}$ of the weighted law;
$\norm{\bar{\mathbf{r}}-\mathbf{r}^{*}}$ is the bias that
Table~\ref{tab:Occlusion_pose} reports.

\begin{proposition}[Convergence rate under the adaptive gain]
\label{prop:rate}
Under the assumptions of Proposition~\ref{prop:reweighting} with
$\bar{\delta}\to0$, the gain~\eqref{eq:gain-normalised} and sampling period $T$,
the nominal closed loop satisfies
$\norm{\delta\mathbf{r}_{k}}=O(k^{-1/2})$ and
$\norm{\mathbf{V}_{c,k}}=O(k^{-3/2})$. If instead
$\bar{e}\to\bar{e}_\infty>0$, the convergence to $\bar{\mathbf{r}}$ is
geometric with ratio $1-TN_e^{0.2}\bar{e}_\infty$.
\end{proposition}
\begin{proof}
With $\norm{\mathbf{e}_I}^{2}=\rho+O(\norm{\delta\mathbf{r}}^{3})$,
$\rho=\delta\mathbf{r}^{\top}\mathbf{Q}\,\delta\mathbf{r}$, the update
$\delta\mathbf{r}_{k+1}=(1-TN_e^{-0.8}\rho_k)\,\delta\mathbf{r}_k$ preserves the
direction of $\delta\mathbf{r}$ and gives
$\rho_{k+1}^{-1}=\rho_k^{-1}+2TN_e^{-0.8}+O(\rho_k)$, hence
$\rho_k\sim(2TN_e^{-0.8}k)^{-1}$. The second statement is immediate.
\end{proof}
```

### F4. Replaces the mechanism paragraph of 4.4.3 and the defence in 4.4.4

```latex
In the configurations considered here the occluded region is a uniform area of
the target surface, so $G$ vanishes on it whatever its intensity, $\bar{G}$ meets
the floor~\eqref{eq:Gfloor}, and the residuals there are inflated tenfold. It is
important to be precise about what this achieves. The interaction matrix is
evaluated on the current image, and by Lemma~\ref{lem:row-norm} its rows vanish
wherever the current image is uniform over the filter support. A measurement in
the interior of the occluder therefore contributes
$w_{j}^{2}e_{j}\boldsymbol{\ell}_{j}^{\top}=\mathbf{0}$ to the descent direction and
$w_{j}^{2}\boldsymbol{\ell}_{j}^{\top}\boldsymbol{\ell}_{j}=\mathbf{0}$ to the curvature
\emph{whatever its weight}, and influences the control law only through the
robust scale. The occluder biases the control through the band, of the order of
the filter support, around its boundary, where the current image contains the
occluder's own contour; there $G$ is large, the normalisation reduces the
residual, and the Hermite-informed weighting is the more lenient of the two.
On a uniform occluder the structure normalisation cannot, therefore, confer an
advantage over residual-based weighting through the occluded region itself;
its effect is confined to the reallocation of the outlier budget among
textured measurements, which bears on the early rejection of valid measurements
discussed in Section~\ref{sec:tukey-limitation}. The same bound implies that
down-weighting a genuinely untextured region discards little information, and,
for the same reason, that down-weighting an occluded one removes little bias.
```

Section 4.7.2 then reports the tie as predicted (cite this paragraph), and 4.8
states the contribution accordingly.
