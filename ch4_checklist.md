# Chapter 4 — edit checklist

Work top to bottom. Step 1 removes about eight separate problems in one
action, so do it first and most of the list disappears.

---

## STEP 1 — Replace section 4.7.1 entirely  ▸ the big one

Delete everything from the heading **4.7.1 Nominal case** (p.71) down to the
end of the paragraph ending *"...attributed to the action of the weighting on
corrupted measurements alone."* (p.75), **including Table 4.1**.

Paste in `ch4_sec7_1_revised.tex`.

Keep Figures 4.1, 4.2, 4.3, 4.4 where they are. Regenerate Figure 4.4 from the
DC-free Variant 3 run.

This single swap deletes all of the following, so you do not have to hunt for
them individually:

- [x] p.71 "establish that they are **indistinguishable** when nothing requires rejection"
- [x] p.72 "the three are **superimposable** at the scale of the figure"
- [x] p.72 "the weights saturate and **the three control laws coincide**"
- [x] p.72 "‖e‖ ≈ 5.23×10¹¹"  (wrong — that is the plot's axis maximum; the value is 3.48×10¹¹)
- [x] p.72 the three fractions 8.3×10⁻⁵, 2.0×10⁻⁴, 1.3×10⁻⁴  (all derived from the wrong initial value)
- [x] p.73 "the **ordering between variants is not to be read as a ranking**"
- [x] p.73 "λ is a deterministic function of ‖e‖"  (it is a function of the *intensity* error)
- [x] p.73 the λ and μ values  (the third variant's are now λ = 0.041, μ = 5.3×10⁻¹⁸)
- [x] p.74 Table 4.1  (units were raw metres and radians under mm and degree headers)
- [x] p.74 "converge to **the same accuracy** ... to the same numerical floor"
- [x] p.75 "Any difference observed in the occluded runs **can consequently be attributed**..."

---

## STEP 2 — Delete Table 4.2 and its paragraph  (p.74)

Delete the paragraph beginning *"What does separate the variants under nominal
conditions is cost."* and the table that follows it.

Reason: the DC-free Variant 3 does **six extra 9×9 convolutions per
iteration** and measured **391 ms** against the earlier version's **1064 ms** —
2.7× faster while doing strictly more work. The table records machine load.

If you want a cost figure later: set `bool opt_display = false;` on the
**local** inside `init()` (the `-d` flag does nothing, because that local
shadows it), run each variant three times, report the mean. Expect ≈ 1.2×, not
3.38×.

---

## STEP 3 — Fix equation (4.9) in section 4.4.2  ▸ most important correction

**Currently says:** G is the Euclidean norm of `w_h`, `w_v`, `w_d` — the
responses to the *summed* kernels Dnm2, Dnm3, Dnm4.

**Must say:** G is built from the **first-order** kernels D_{1,0} and D_{0,1}
at σ₂, σ₃, σ₄:

    G(p) = sqrt( Σ_{k=2,3,4} [ (D_{1,0}^{σk} * I)(p)² + (D_{0,1}^{σk} * I)(p)² ] )

As it stands the text describes code you are no longer shipping. Everything
else in 4.4 then becomes true: the summed kernels have a DC gain equal to their
L₁ norm, so the old G was a smoothed intensity; the first-order kernels are odd
and have exactly zero DC gain, so the new G is genuinely band-pass.

Also in that subsection:

- The sentence excluding the finest scale **stays**, but change its reason.
  Not *"the response remains close to raw intensity"* — the exclusion is simply
  by scale.
- p.67, section 4.4.1: soften *"it is not a generic image-gradient or saliency
  measure"*. With first-order kernels it **is** a Gauss-derivative gradient
  magnitude. The defensible claim is narrower: it is evaluated at the same
  analysis scales that define the features. Keep the band-pass clause — that
  one is now true.

---

## STEP 4 — Add the ablation paragraph to section 4.4.4  (p.68)

New paragraph, roughly:

> An earlier formulation of (4.9) used the summed responses w_h, w_v, w_d
> directly. Those kernels are low-pass dominated — their DC gain equals their
> L₁ norm to three decimal places — so G measured local intensity rather than
> local structure: on the target of Fig. 4.1(a) the mean gradient of G was
> 6.2% of its mean level and 62.8% of pixels lay within a tenth of their own
> 9×9 local mean, against 26.2% and 18.8% for the first-order form adopted
> here. That formulation attained a smaller final error, 0.089° against 0.385°
> in rotation. On this target the bright regions are also the strongly textured
> ones, so an intensity-weighted gate coincides with a structure-weighted one;
> whether this holds on scenes where brightness and texture are uncorrelated is
> left to future work.

**Figure:** put the two structure maps side by side —
`figures/gmap_v3_1.png` (intensity-like) and `figures/gmap_v3_dcfree.png`
(edge-like). One glance makes the whole argument.

---

## STEP 5 — Fix the occlusion mechanism sentence in 4.4.3  (p.68)

**Currently:** *"the occluded region is rendered as a uniform **dark** area of
the target surface, so its Hermite response energy falls well below the image
average"*

With the DC-free G, darkness is irrelevant — **uniformity** is what matters.
Rewrite so the operative word is *uniform*, and drop *dark*. This makes the
paragraph stronger, not weaker: the mechanism now works on a bright occluder
too, so the caveat in 4.4.4 about a *strongly textured* occluder is the only
one you still need.

---

## STEP 6 — Fill section 4.7.3 "Weight maps"  (p.75, currently an empty heading)

You have the figures: `figures/wmap_v3_dcfree.png`.

Two claims it lets you support:

1. **Section 4.3.3** predicts valid measurements are rejected across the frame
   while the misalignment is still gross. The map at iteration 11 shows 3.5%
   fully rejected and coherent dark patches **on a clean scene** — exactly that.
   Produce the iteration-600 map too (change the save condition to
   `iter == 2 || iter == 601`); it should be almost uniformly white, which
   completes the argument.
2. **Section 4.3.2** predicts the affected region is the occluder *dilated* by
   the filter support. Measure the dark blob in the occluded run against the
   ellipse in the texture; it should be visibly larger.

Amend **section 4.6.4** while you are there: it promises weight maps for all
runs. Change to **Variants 2 and 3 only** — Variant 1 has D = I, so its map is
uniformly white and carries nothing.

---

## STEP 7 — Section 4.6.3 "Test configurations"  (p.71)

Empty heading. Either fill it with the nominal and occluded configurations, or
delete the heading and fold the content into 4.6.2. Do not leave it empty.

---

## STEP 8 — Small fixes

- [ ] p.63, section 4.2.2 — `[The unweighted scheme is the special case D = I]`
      and `[What the weights may depend upon]` appear in square brackets. The
      `remark` environment is broken; this is also why `Remark 4.2.2` does not
      resolve anywhere in the chapter.
- [ ] p.69, section 4.5 — `Section ??` and `Chapter ??`, two broken references.
- [ ] p.69, eq (4.14) — the hat spans `DL_R` instead of the whole term.
- [ ] p.68 — "the normalized residual is reduced**,,** and" — double comma.
- [ ] p.74 — `Remark ??`.
- [ ] p.75 — `Section ??`.
- [ ] Figure captions — "positiong error" → "positioning error"; check
      Figure 4.4 is labelled Variant 3, not Variant 1.

---

## Not in this chapter's critical path

Deferred to future work, as agreed:

- the multi-pose study (4 variants × 3–4 initial poses) needed to rank the
  variants — section 4.7.1 now explicitly declines to rank them, so this is a
  stated gap rather than an unstated one;
- the intensity-versus-structure question raised in step 4;
- logging ‖D_k − D_{k−1}‖ to test section 4.5's assumption that the weights
  vary slowly relative to the control.

## Next actual task

Section 4.7.2, the occluded runs. Before running:

1. confirm `peppers_o_b.jpg` really contains the black ellipse — your earlier
   screenshot of the s\* window showed a clean image, which means it may not;
2. swap the two `sim.init` calls in **all three** files —
   `sim.init(Itexture, X)` before `cdMo` (clean reference),
   `sim.init(Itextured, X)` before `cMo` (occluded during servoing).
   They are currently the wrong way round.
