# Chapter 4 — corrections, in order of severity

Read from the PDF of 22 pages. Sections 4.4.2, 4.7.3, Table 4.3 and Figure 4.10
are in and consistent. The problems below are what remains.

---

## A. The chapter currently contradicts itself. Fix these first.

### A1. Section 4.7.1 was never replaced

`ch4_sec7_1_revised.tex` was not applied. The old text is still there, and it
now states the opposite of Section 4.7.3, which uses the same runs:

| p. | §4.7.1 says | §4.7.3 says |
|---|---|---|
| 73 | "the purpose of this run is not to distinguish them but to establish that they are **indistinguishable**" | at iteration 1 the clean scene strongly down-weights **13.6%** of measurements, all of them valid |
| 75 | "the ordering between variants **is not to be read as a ranking**" | the weighting is transparent only **at convergence**, 0.3% |
| 76 | "**Neither estimator rejects valid measurements** to a degree that degrades positioning" | "all 13.6% of its strongly down-weighted measurements are valid ones" |
| 76 | "the two weighted variants converge to **the same accuracy** as the unweighted one" | — |

An examiner reading the two sections together will see this. **Replace the body
of 4.7.1 with `ch4_sec7_1_revised.tex`.** Keep Figures 4.2–4.5.

### A2. Table 4.1 reports a different method from the one Section 4.4 defines

Row 3 of Table 4.1 is the **intensity-gate** run:

```
Hermite   9.77e-5  1.36e-4  8.08e-4  -1.24e-3  8.33e-4  3.79e-4
```

which converts to ‖Δω‖ = **0.088°**. That is G built from the *summed* kernels.
But Section 4.4.2 now defines G from the **first-order** kernels, and that run
gives ‖Δω‖ = **0.385°**:

```
Hermite   1.00817e-4  -8.37754e-5  -9.90396e-4   6.68334e-3  -5.30911e-4  3.30153e-4
```

Sections 4.4.2, 4.7.3, Table 4.3 and Figure 4.10 are all DC-free. Table 4.1 and
Figure 4.5 are not. **Replace row 3 of Table 4.1** with the values above (the
corrected table is in `ch4_tables_corrected.tex`), and **regenerate Figure 4.5**
from the DC-free nominal run.

If Figure 4.5 was produced from the intensity run it must be replaced; if you
are unsure which, re-run nominal V3 once with the first-order line in place and
screenshot the three plots.

### A3. Section 4.7.2 has no text

Only Figures 4.6–4.9 and Table 4.2. Draft supplied in
`ch4_sec7_2_occlusion.tex`.

**The result it has to report honestly:** Variants 2 and 3 differ by less than
8% in either direction — V3 is 7.6% better in rotation and 7.2% worse in
translation. Section 4.6.1 calls this "the decisive comparison of the chapter",
so the chapter must say plainly that on this configuration the two are
indistinguishable, and that what the weighting buys is the factor of ~400 over
the unweighted baseline. Claiming more will not survive scrutiny.

---

## B. Units. Both results tables are wrong in the same way.

The values are raw **metres and radians** under **mm** and **degree** headers —
every translation 1000× too small, every rotation 57.3× too small.

And the `initial` / `desired` rows use two further units: initial `(0.4, -0.4,
11)` is the pose `(0.04, -0.04, 1.10)` m written in decimetres, while desired
`1.3` is in metres. Three units in one column.

Corrected tables in `ch4_tables_corrected.tex`. They also drop the pose rows
into the caption, which removes the confusion between *poses* (initial,
desired) and *errors* (the variant rows), and add norm columns so the reader
does not have to compute them.

---

## C. Numbers in the 4.7.1 prose

| p. | printed | correct |
|---|---|---|
| 73 | ‖e‖ ≈ 5.23×10¹¹ at the initial pose | **3.48×10¹¹** — 5.23 is the *axis maximum* of the plot |
| 73 | "at 8.3×10⁻⁵, 2.0×10⁻⁴ and 1.3×10⁻⁴ of its initial value" | all three inherit the wrong initial value; recompute against 3.48×10¹¹ |
| 75 | λ = 0.0231, 0.0572, **0.0365** | third is the intensity run; DC-free gives **0.0407** |
| 75 | μ = 3.10×10⁻¹⁹, 2.88×10⁻¹⁷, **3.04×10⁻¹⁸** | third → **5.26×10⁻¹⁸** |
| 75 | ‖v_c‖ = 1.41×10⁻⁴, 3.68×10⁻⁴, **2.88×10⁻⁵** | third → **2.29×10⁻⁴** |
| 75 | "since λ is a deterministic function of ‖e‖" | λ is a function of the **intensity** error ‖e_I‖², a different quantity from the feature error. The observed ordering is real; the stated reason is not. |

The λ ordering survives the correction: 0.0231 < 0.0407 < 0.0572 still matches
4.35×10⁷ < 6.48×10⁷ < 1.04×10⁸.

Also p. 73–74: `‖e‖` is the **sum of squares**, not the norm. Either relabel it
‖e‖² throughout 4.7 and on the figure axes, or take square roots.

---

## D. Text that no longer matches the method

### D1. Section 4.4.1, p. 67
> "It is **not a generic image-gradient or saliency measure** appended to the
> servoing scheme"

With first-order kernels the operator **is** a Gaussian-derivative gradient
magnitude. Section 4.4.2 already states the defensible version — that what is
contributed is the choice of scales, not the form of the operator. Make 4.4.1
agree.

The clause "inherits the band-pass character … established in Section 3.2.4"
is now **true** of the implementation and should be kept.

### D2. Section 4.4.3, p. 70
> "the occluded region is rendered as a uniform **dark** area of the target
> surface, so its Hermite response energy falls well below the image average"

Darkness is now irrelevant: G has exactly zero DC gain, so a uniform **bright**
occluder is rejected identically. Rewrite so the operative word is *uniform*
and delete *dark*. This strengthens the section — the only limitation left to
acknowledge is the textured occluder of 4.4.4, which is exactly what 4.4.4
says.

### D3. Section 4.4.3, p. 70
"the normalized residual is reduced**,,** and" — double comma.

---

## E. References, empty sections, floats

- **Broken `??`**, in reading order: 4.4.2 ×2 (`Section ??` for the kernel
  definition, `Section ??` for the ablation), 4.4.2 `Figure ??(a)` → 4.2(a),
  4.4.2 ×2 in the paragraph after (4.10), 4.4.2 ×3 in the finest-scale
  paragraph, 4.5 ×2 (`Section ??` / `Chapter ??`), 4.7.1 p. 75 `Remark ??`,
  4.7.1 p. 76 `Section ??`.
- **Section 4.6.3 "Test configurations"** is still an empty heading. Fill it
  with the nominal and occluded configurations, or delete it and fold the
  content into 4.6.2.
- **Section 4.6.4** promises weight maps without qualification. Only Variant 3
  produces them, and Variant 1 has **D** = **I** so its map is uniformly white.
  Amend to Variant 3.
- **Section 4.7.3 cross-references**: p. 76 "Sections 4.7.1 and **4.7**" →
  4.7.2; p. 79 "reported in Section **4.7**" → 4.7.2.
- **Figure 4.10 floats onto p. 80**, after the Section 4.8 heading, while its
  text is on p. 76–79. Force it earlier (`[htbp]`, or place it directly after
  the 4.7.3 opening paragraph).
- **Equation (4.15), p. 71**: the hat spans `DL_R` instead of the whole term.
- **Section 4.2.2, p. 63**: `[The unweighted scheme is the special case D = I]`
  and `[What the weights may depend upon]` appear in square brackets — a broken
  `remark` environment, which is also why `Remark 4.2.2` does not resolve on
  p. 75.

---

## F. One thing I could not check

Table 4.2's Hermite row is very close to the iteration-**300** console you sent
(1.82029e-6, 1.75413e-6, -1.86639e-6, 2.54027e-4, 7.57441e-5, -8.23718e-6),
differing only in the last digits. Confirm it is the iteration-600 value, since
Table 4.1 and Table 4.3 are both at 600 and the three must agree.

The iteration-600 ‖e‖² values for the occluded runs are also needed for 4.7.2 —
at iteration 300 they were 1.551×10¹⁰, 1.573×10¹⁰, 1.573×10¹⁰.
