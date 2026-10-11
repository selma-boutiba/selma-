# Article plan — simulation study with strong statistics

**Target.** *Robotics and Autonomous Systems* (Elsevier), *Journal of
Intelligent & Robotic Systems* (Springer) or *Machine Vision and Applications*
(Springer). Check each journal's current author guidelines for length and
supplementary material before writing; plan for about 8 000 words, 6–8
figures and 3 tables.

**One campaign, two possible papers.** The same experiments serve a *method*
paper and an *analysis* paper; which one is written is decided by a
pre-registered test, not by inspecting the results.

| If the primary test (§3, H3/H4) … | the paper is |
|---|---|
| succeeds: a structure-informed weighting (V3 or V5) beats Tukey (V2) with Holm-adjusted $p<0.05$ and a median ratio $\le0.8$ | **Method paper.** *Structure-informed robust weighting for direct visual servoing: when image structure helps, and why* |
| fails | **Analysis paper.** *Robust weighting in photometric visual servoing: inertness, convergence under reweighting, and the stall of error-proportional gains* |

---

## 1. Contributions

Common to both versions (theory in `ch4_improvement_proposal.md`, Parts A–B):

1. **Inertness.** With the interaction matrix evaluated on the current image,
   weights on measurements whose support is uniform in the current image do not
   enter the control law (Proposition 1). Structure-based weighting can
   therefore act on an untextured occluder only when the interaction matrix is
   taken from another image, e.g. the desired one.
2. **Convergence under arbitrary reweighting.** Local convergence holds for any
   weight sequence provided $\delta_{\mathbf W}<1$, a condition linked to the
   floor on the damped Hessian and checkable online (Proposition 2).
3. **Rate under error-proportional gains.** $\lambda\propto\lVert\mathbf e_I\rVert^2$
   gives $\lVert\delta\mathbf r_k\rVert=O(k^{-1/2})$ in the nominal case and
   geometric convergence to a biased equilibrium under occlusion
   (Proposition 3).
4. **A pre-registered statistical evaluation** over 32 initial poses ×
   4 scenarios × 2 interaction matrices × 5 weightings, with a breakdown sweep
   and a replication on a second texture.

Method version adds: the adaptive-floor weighting V5, derived from a
heteroscedastic residual model as a Schweppe-type GM-estimator, shown superior
in the regime predicted by contribution 1.

Novelty check before writing contribution 3: establish whether
$\lambda\propto\lVert\mathbf e_I\rVert^2$ appears in published photometric
servoing work. If it appears only in your own implementation, present
Proposition 3 as an analysis of that law and its fix, not as a general finding.

## 2. Weightings compared

All share the control law (4.4) with the floor (4.5); only $\mathbf W$ differs.
$\bar G(p)=G(p)/\overline G$ is the DC-free structure measure (4.11)–(4.12) of
the current image; $w^{\mathrm T}$ is the Tukey weight with the median-centred
MAD scale of its argument.

| | Name | Weight $w_j$ | Role |
|---|---|---|---|
| V1 | none | $1$ | baseline (Chapter 3) |
| V2 | Tukey | $w^{\mathrm T}(e_j)$ | generic robust reference |
| V3 | Hermite-informed | $w^{\mathrm T}\bigl(e_j/\max(\bar G_j,0.1)\bigr)$ | Chapter 4 |
| V4 | structure-penalised | $w^{\mathrm T}(e_j)\,(1+\bar G_j^{2})^{-1/2}$ | DSO-style control: design weighting in the opposite direction |
| V5 | adaptive floor | $w^{\mathrm T}\!\left(e_j\sqrt{1+\gamma_k^2}\big/\sqrt{\bar G_j^{2}+\gamma_k^{2}}\right)$, $\gamma_k=0.1\,\hat s_0/\hat s_k$ | heteroscedastic model: V3 early, V2 at convergence |

$\hat s_k$ is the MAD scale of the raw residual at iteration $k$. The factor
$\sqrt{1+\gamma_k^2}$ is a uniform rescaling, hence inert; it keeps the
argument on the scale of $e_j$ so that ViSP's absolute MAD floor (0.0017)
never acts.

## 3. Hypotheses

Fix the primary family after the pilot and before the confirmatory runs. Commit
`PREREGISTRATION.md` (template in §9), so the git timestamp records the order.

| | Prediction | Source | Test | Family |
|---|---|---|---|---|
| H1 | Robust weighting removes occlusion bias: V2/V1 < 1 | known | Wilcoxon, HL ratio | secondary |
| H2 | Current-image L, uniform occluder: V3 ≈ V2 | Prop. 1 | equivalence, ±25 %, 95 % CI | secondary (confirms the theory) |
| **H3** | **Desired-image L, uniform occluder: V3 < V2** | Prop. 1 | Wilcoxon, Holm | **primary** |
| **H4** | **Desired-image L, uniform occluder: V5 < V2** | heteroscedastic model | Wilcoxon, Holm | **primary** |
| H5 | V4 ≥ V2 wherever H3 holds | direction of design weighting | Wilcoxon | secondary |
| H6 | Textured occluder: V3 not better than V2 | §4.4.4 limitation | Wilcoxon | secondary |
| H7 | Nominal tail slope of $\log\lVert\Delta\mathbf t\rVert$ vs $\log k$ is $-1/2$; occluded weighted runs reach a stationary pose | Prop. 3 | per-run fit; CI of the median | secondary |
| H8 | Under occlusion the final cost does not rank accuracy: Kendall $\tau\le0$ within pose | §4.6.4 | sign of $\tau$ | secondary |
| H9 | Success falls with occluded fraction; the fraction at which it does depends on the weighting | §4.3.2 | success curves, McNemar | secondary |
| H10 | $\lambda_{\min}(\mathbf H_{\mathbf W})/(\operatorname{tr}\mathbf H_{\mathbf W}/6)>10^{-6}$ in every converged run; failures coincide with violations | Prop. 2 | logged margin | secondary |

Primary family = H3, H4 × {translation, rotation} = 4 tests, Holm-adjusted.
Everything else is reported with Holm within the secondary family and labelled
as such.

## 4. Design

**Poses** (`tools/make_poses.py`, coverage of the interior required, bins by
mean image displacement; the definition reproduces §4.6.3's 40/65 px for the
Chapter 4 pose):

| Set | File | Seed | Poses | Use |
|---|---|---|---|---|
| validation | — | — | pose 0 (Chapter 4) | reproduction of Tables 4.1–4.2 |
| pilot | `campaign/poses_pilot.csv` | 20261012 | 101–104, one per bin | development: tuning, sizing, pre-registration |
| confirmatory | `campaign/poses.csv` | 20261011 | 1–32, 8 per bin: 20–32, 32–44, 44–56, 56–68 px | all reported statistics |

The Chapter 5 poses 1–10 (`campaign/poses_ch5.csv`) do not keep the target over
the whole image: background pixels enter the feature vector. They are not used
here. Note this for Chapter 5 as well.

**Scenarios** (`tools/make_occluders.py`, coverage at the desired pose measured
from the mask):

```bash
for k in uniform_dark textured low_contrast; do
  python3 tools/make_occluders.py generate --texture peppers.jpg --kind $k --fraction 0.07 \
      --out-dir campaign/textures --poses campaign/poses.csv
done
for f in 0.15 0.25 0.35 0.45; do
  python3 tools/make_occluders.py generate --texture peppers.jpg --kind uniform_dark \
      --fraction $f --out-dir campaign/textures
done
# Chapter 4's own occluder, measured from its mask (item A6):
python3 tools/make_occluders.py measure --clean peppers.jpg --occluded peppers_o_b.jpg \
    --poses campaign/poses.csv
```

**Runs** (`campaign/run_campaign.sh`):

| Design | Poses | Weightings | L | Scenarios | Runs |
|---|---|---|---|---|---|
| validate | 0 | V1–V3 | current, thesis pixel order | nominal, Chapter 4 occluder | 6 |
| pilot | 101–104 | V1–V5 | current, desired | nominal, dark, textured, low contrast | 160 |
| confirmatory | 1–32 | V1–V5 | current, desired | as pilot | 1 280 |
| sweep | 1–32 | V2, V3, V5 | current, desired | dark at 15, 25, 35, 45 % | 768 |
| replication | 1–32 | V2, V3, V5 | desired | dark 7 % on a second texture | 96 |

Wall time = runs × 600 × $t_{\text{iter}}$ / JOBS. The pilot logs
$t_{\text{iter}}$; at 0.3 s and three parallel jobs the confirmatory design
takes about 21 h, at 0.1 s about 7 h. If too long, drop weightings from the
confirmatory design after the pilot (`CONF_WEIGHTINGS`), never poses.

**Second texture.** A single texture is a threat to external validity that a
reviewer will raise. Choose a texture of different statistics (e.g. a
man-made scene with large flat regions) and replicate the primary comparison
on it.

## 5. Metrics (per run, `analysis/analyze.py`)

- final translation and rotation error: median over the last 50 iterations of
  $\lVert\Delta\mathbf t\rVert$ (mm) and $\lVert\Delta\boldsymbol\theta\mathbf u\rVert$ (deg) of the relative pose ${}^{c^*}\mathbf M_c$;
- success: final errors below 1 mm and 0.5° (the accuracy class quoted in
  §4.7.1), with ECDFs reported so that no conclusion depends on the threshold;
- iterations until $\lVert\mathbf v\rVert<10^{-3}$ m/s; terminal jitter (s.d. of
  $\lVert\Delta\mathbf t\rVert$ over the last 100 iterations);
- tail slope of $\log\lVert\Delta\mathbf t\rVert$ against $\log k$ over $k\ge200$;
- rank margin $\min_k\lambda_{\min}(\mathbf H_{\mathbf W})/(\operatorname{tr}\mathbf H_{\mathbf W}/6)$;
- weight change $\lVert\mathbf w_k-\mathbf w_{k-1}\rVert/\lVert\mathbf w_k\rVert$; fractions
  $w<0.25$ in total and per scale channel; time spent computing the weights.

## 6. Statistical analysis

- **Unit:** the initial pose; every comparison is paired over the poses.
- **Effect:** Hodges–Lehmann estimate of the median log-ratio A/B,
  back-transformed, with its exact distribution-free 95 % CI.
- **Test:** Wilcoxon signed-rank, two-sided, exact when $n\le50$.
- **Multiplicity:** Holm within the primary family (4 tests) and separately
  within the secondary family.
- **Equivalence:** 95 % CI of the ratio inside $[0.8, 1.25]$, which is TOST at
  5 % per outcome with Bonferroni over the two outcomes.
- **Success:** exact McNemar on paired indicators.
- **Sample size.** Monte-Carlo power of the Wilcoxon test under Holm
  (computed for this plan):

| $n$ | family of 20 tests, $d_z=0.75$ / $1.0$ | family of 6 tests, $d_z=0.75$ / $1.0$ |
|---|---|---|
| 10 | 0.00 / 0.00 | 0.20 / 0.36 (exact test) |
| 24 | 0.52 / 0.87 | 0.73 / 0.96 |
| 32 | 0.76 / 0.98 | **0.89 / 1.00** |

  $d_z$ is the mean of the per-pose log-ratios divided by their standard
  deviation. Hence 32 poses and a small primary family. With ten poses, as
  planned for Chapter 5, the smallest attainable two-sided $p$ is 0.002, so a
  20-test family can never reach significance and a 6-test family has power
  of at most 0.36 for a large effect.

The pipeline is verified end to end on synthetic logs with planted effects
(`analysis/selftest.py`): it recovers a 0.6 ratio as significant after Holm,
declares a null effect equivalent, recovers the $-1/2$ tail slope and a
400-fold bias.

## 7. Prerequisites in the code, in order

1. Fix the shadowed options in `init()` and guard all display and `vpPlot`
   calls (`ch5_which_code_and_patch.md` §1, `cpp/INTEGRATION.md` Edit 1).
2. Integrate `cpp/campaign.h` (Edits 2–7).
3. `run_campaign.sh validate` with the thesis pixel order: Tables 4.1–4.2
   must be reproduced to the printed digits. Stop if not.
4. Switch to the corrected principal point (`-x 1`) and record its effect at
   pose 0; this is an erratum for Chapters 3–4.
5. Commit the program that produces the results, tagged.

## 8. Paper outline and word budget

| § | Content | Words | Figures/tables |
|---|---|---|---|
| 1 | Introduction: occlusion in direct servoing; why weighting and what is not known | 900 | — |
| 2 | Related work: robust servoing (Comport 2006), photometric servoing (Collewet 2011; Dame 2011; Crombez 2019), robust direct alignment (LSD-SLAM, DSO), GM-estimation | 900 | — |
| 3 | Setting: Hermite photometric servoing, the weighted control law | 800 | — |
| 4 | Analysis: Lemma 1, Propositions 1–3, predictions H2–H10 | 1 800 | Fig. 1 influence maps |
| 5 | Weightings V1–V5 as GM-estimators; heteroscedastic model | 600 | Table 1 |
| 6 | Protocol: poses, scenarios, metrics, statistics, pre-registration, code | 900 | Fig. 2 scenarios; Table 2 design |
| 7 | Results by hypothesis | 1 600 | Fig. 3 ECDFs; Fig. 4 convergence; Fig. 5 breakdown; Table 3 primary tests |
| 8 | Discussion and limitations: simulation, planar target, textures, constant depth | 500 | — |
| 9 | Conclusion | 300 | — |

The thesis chapter's prose cannot be reused as it stands: the article states
each result once, in neutral terms, and moves the full per-cell tables to
supplementary material.

## 9. Pre-registration template (commit before the confirmatory runs)

```markdown
# Pre-registration — confirmatory campaign
Date / commit:
Program and tag:            Pixel order: corrected
Poses: campaign/poses.csv (seed 20261011, ids 1-32), unchanged after this commit
Primary family (Holm):      e.g. desired:uniform_dark_f070:hermite:tukey,
                                 desired:uniform_dark_f070:adaptive:tukey
Outcomes: final_t_mm, final_r_deg (median of the last 50 iterations)
Decision rule: method paper iff any primary test has p_Holm < 0.05 and ratio <= 0.8
Equivalence margin: 0.8-1.25, 95 % CI
Success thresholds: 1 mm, 0.5 deg
Changes from the pilot, with reasons:
```

## 10. Timeline

| Week | Work |
|---|---|
| 1 | Integration, validation, principal-point erratum |
| 2 | Textures, pilot, pre-registration |
| 3–4 | Confirmatory, sweep and replication runs (compute); write §§2–6 meanwhile |
| 5 | Analysis, figures, §7 |
| 6 | §§1, 8, 9; internal review; submission package (code and data archive) |

## 11. Risks

| Risk | Mitigation |
|---|---|
| Neither V3 nor V5 beats V2 | Analysis paper; H2, H7, H8 and H10 carry it |
| Validation does not reproduce the thesis | Stop and locate the difference before any campaign run |
| Run time too long | Fewer weightings after the pilot; more cores; never fewer poses |
| Principal-point correction changes thesis numbers | Report as an erratum in the thesis; the article uses the corrected matrix |
| Reviewer asks for real-robot evidence | State the simulation scope in the title and abstract; offer the code; target journals that accept simulation studies |

## 12. Files

| Path | Purpose | Status |
|---|---|---|
| `tools/make_poses.py` | stratified initial poses with coverage check | tested |
| `tools/make_occluders.py` | occluded textures; coverage from the mask | tested on a synthetic texture |
| `cpp/campaign.h` | weightings V1–V5, pose loading, pixel convention, logging, maps | 23 checks pass against ViSP 3.6.0 |
| `cpp/INTEGRATION.md` | edits to `tutorial-viewer15.cpp` | needs viewer15 for an exact patch |
| `campaign/run_campaign.sh` | resumable, parallel runner | dry-run tested |
| `campaign/poses*.csv` | confirmatory, pilot and Chapter 5 pose sets | generated |
| `analysis/analyze.py` | metrics, paired statistics, report, LaTeX, figures | verified by `selftest.py` |
