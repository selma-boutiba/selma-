# Which scenarios for Chapter 5 — yes, all three, but for different reasons than the plan gives

**Short answer: keep all three.** But they are not three robustness tests. For a
chapter about the gain, they are **three different gain regimes**, and each one
tests a different failure. Reframing them this way changes which is primary,
and it turns one of them into a falsification test rather than a demonstration.

---

## 1. What each scenario does to the gain

Everything follows from λ = N_ei^0.2 · ē with ē = N_e/N_ei, and from what
happens to N_e — the intensity error — as the pose converges.

| scenario | N_e as the pose converges | ē floors at | λ ends at | the failure it tests |
|---|---|---|---|---|
| **nominal** | decays freely toward zero | ~8×10⁻⁴ | **0.041** | **the stall** — the gain collapses and the servo creeps to a halt |
| **occlusion** | cannot decay: no counterpart in the reference over ~6 % of the image | ~4×10⁻² | **2.19** | **control condition** — the gain is already floored by accident |
| **illumination** | cannot decay: mismatch over the *whole* image | to be measured, probably high | **pinned high** | **overshoot** — a floor cannot help, only scheduling can |

The three numbers in the λ column are measured, not predicted — 0.041 and 2.19
are from your own iteration-600 logs.

---

## 2. Nominal becomes the primary scenario — a reversal from Chapter 4

In Chapter 4 occlusion was the headline and nominal was the control: the point
was what robust weighting buys when measurements are corrupted, and the clean
scene was there to measure what it costs when they are not.

**Chapter 5 is the other way round.** The gain defect only appears when the
error is free to decay to near zero, because that is the only time λ collapses.
So the nominal scenario is where Arm A fails, where the floor and the scheduler
have something to fix, and where the three-orders-of-magnitude headroom lives.

Say this explicitly in §5.6.3. A reader arriving from Chapter 4 will expect
occlusion to be the important case and needs to be told it is not.

---

## 3. Occlusion becomes a control condition — and that is worth more than another demonstration

This is the part the plan gets backwards, and correcting it is the most
scientifically valuable change to the protocol.

Chapter 4 already established that **under occlusion the gain does not
collapse.** The occluder leaves an irreducible residual, so N_e stays large, ē
floors near 4×10⁻², λ sits at 2.19 instead of 0.041 — and the run reaches
**2.5 µm** instead of stalling at a millimetre. The defect Chapter 5 exists to
fix is, in this scenario, already absent.

So the prediction is: **under occlusion the three arms should perform
similarly.** And that makes it a test of the chapter's own explanation rather
than a restatement of Chapter 4:

- If A, A′ and B land in the same place under occlusion, the mechanism is
  confirmed — the accidental floor and the deliberate floor do the same thing,
  which is exactly what the theory says.
- If Arm A is markedly worse under occlusion too, then the explanation is
  wrong: the gain collapse is not what was limiting the nominal runs, and
  something else is, and the chapter needs to find out what before it claims
  anything.

A protocol that can fail is worth far more in a defence than one that can only
succeed. State the prediction in §5.6.4, before the results, so that the
agreement reads as a confirmed prediction and not as a null result you are
explaining away afterwards.

---

## 4. Illumination is the scenario where Arm B can beat Arm A′ — and the only one

Here is the argument, and it is the strongest case for keeping illumination.

A floor is `lambda = max(lambda, lambdaMin)`. It can only ever **raise** the
gain. So:

- **Nominal**: λ collapses to 0.041 → the floor raises it → the floor works.
- **Occlusion**: λ is already 2.19 → the floor changes nothing → no effect.
- **Illumination**: if the residual mismatch pins ē high, then λ is pinned
  *high* — and **a floor is useless, because the problem is a gain that is too
  large, not too small.**

Only a scheduler can reduce the gain, and only because it has the second input:
the rule "error small or flat and *rising* → λ Low" has no counterpart in any
clamp. If Arm B beats Arm A′ anywhere, it will be here.

So: nominal shows the floor is necessary, occlusion shows the mechanism is
right, **illumination is where the scheduler earns its keep.** Three scenarios,
three distinct jobs, none of them redundant.

---

## 5. But verify the premise first — one run, and I found a reason to doubt it

The illumination argument above assumes the residual stays large under an
illumination change. Whether it does depends on how the Hermite features
respond to a uniform brightness shift, and I measured that.

Your `Hermite()` sums the orders into one kernel per scale:

```cpp
for (int n = 0; n <= n_max; ++n)
  for (int m = 0; m <= m_max; ++m)
    Dnm1[i][j] += dnFunction(n, x, sigma1) * dnFunction(m, y, sigma1);
```

The result:

| σ | DC gain | L₁ norm | DC/L₁ | all entries same sign? |
|---|---|---|---|---|
| 0.5 | 15.408 | 15.408 | **1.0000** | **yes** |
| 1.1 | 20.835 | 20.835 | **1.0000** | **yes** |
| 1.8 | 19.075 | 22.280 | 0.856 | no |
| 2.8 | 6.706 | 8.806 | 0.762 | no |

For comparison, a single band-pass term D₁,₀ at σ = 1.1 has DC gain
−1.1×10⁻¹⁶ against an L₁ norm of 4.08 — that is, **exactly zero**, as a
band-pass filter should.

So the summed kernels are not band-pass. At the two finest scales they are
strictly positive — pure low-pass — and a uniform brightness change of Δ adds
Δ × DC to every feature component. **This is the same effect Chapter 4 found in
the structure measure**, where summing the orders produced a measure of local
*brightness* instead of local structure, and where you fixed it by rebuilding
the measure from the odd, zero-mean orders.

Two things follow, and I want to be careful about which is which.

**For Chapter 5, it is a measurement to make, not a conclusion to draw.** Run
one servo with the illumination-changed texture and print both `normeError` and
`normeErrorI` at convergence. That tells you directly whether ē floors high
(and λ is pinned, and §4 above holds) or floors low (and illumination behaves
like the nominal case, and the scenario tests the stall again rather than
overshoot). Five minutes, and the answer decides what §5.7.2 is about.

**For the paper's claim, it is a question worth being ready for.** Your paper
attributes illumination robustness to the filters being "Gaussian-like
band-pass". That is true of the individual D_{n,m} with n+m ≥ 1; it is not true
of their sum, which is what the code convolves with. The illumination results
in the paper are measurements and they stand — but if a jury member asks *why*
they worked, DC rejection by these kernels cannot be the answer, and the honest
reply is that the smoothing improves the conditioning of the interaction
matrix rather than that it removes the brightness term. Worth thinking about
before the defence.

(There is a cheap fix — omit the (0,0) term from the sum, or keep the orders as
separate channels — but it changes the *feature definition*, which is
Chapter 3 and the published paper. That puts it outside Chapter 5, whose whole
design rests on holding the features and the weighting fixed. Note it as a
finding; do not act on it in this chapter.)

---

## 6. The design, concretely

**Three arms × three scenarios × ten poses = 90 runs.** All three arms on
Variant 3, **D** identical throughout.

At the ~309 ms per iteration your paper reports, 600 iterations is about three
minutes, so the campaign is **four to five hours**. Affordable, and it is nine
times the evidence Chapter 4 had.

Present it as a 3 × 3 table of distributions, not 90 numbers:

| | nominal | occlusion | illumination |
|---|---|---|---|
| **A** fixed | mean ± spread, worst | | |
| **A′** floored | | | |
| **B** fuzzy | | | |

each cell giving the mean and worst-case final pose error over the ten poses.
That is the table Chapter 4 could not produce, and it is what answers the
single-run objection that runs through the whole of Chapter 4.

**Metrics** — four, and the third is new and necessary:

1. **final pose error**, translation and rotation — the primary criterion, for
   the reason §4.6.4 gives;
2. **iterations to converge**, defined by ‖V_c‖ < 10⁻³ m/s and *not* by the
   inherited ‖e‖² < 10⁴ threshold, which Chapter 4 showed is never reached;
3. **terminal jitter** — ‖V_c‖ once the pose has stopped improving. This is the
   metric that will show the illumination result, and Chapter 4 gives you a
   baseline for it: Variant 3 ended at 6.3×10⁻⁶ against Variant 2's 1.8×10⁻⁷;
4. **number of poses converged**, out of ten, per arm per scenario — this is
   the convergence-domain measurement of §5.7.3, and with ten poses starting
   near Z = 3.2 m it has a real chance of separating the arms.

**Practical points:**

- **Use 320 × 240 for all three scenarios.** Your paper's Scenario 1 used
  386 × 230; mixing image sizes changes *P* and makes the tables
  non-comparable, which is the same trap §4.6.2 had to be rescued from.
- **Use the same ten poses in all three scenarios.** Only the texture pair
  changes. That keeps the design factorial and lets you attribute any
  difference to the scenario rather than to the pose.
- **Fix the texture wiring first** (reference clean, servoing images
  perturbed), in all three scenarios, or the illumination and occlusion arms
  are both inverted.
- **Nominal here means the same texture in both**, so the residual comes from
  the pose alone — as in §4.6.3.

---

## 7. What to run, in order

1. **The illumination probe.** One run, illumination texture, print
   `normeError` and `normeErrorI` at convergence. This decides whether §5.7.2
   is about overshoot or about the stall. Five minutes.
2. **Arm A′ nominal, one pose.** The experiment that decides the chapter's
   shape, as before.
3. **λ_min sweep**, six runs, to calibrate the bounds on measurement.
4. **The 90-run campaign.**

Do 1 and 2 before writing any of §5.6 or §5.7. Both are short, and between them
they fix what the chapter is about.
