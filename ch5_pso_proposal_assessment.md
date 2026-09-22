# Chapter 5 as PSO-tuned scales — assessment

**Short answer.** It is a legitimate chapter with a genuine hook in Chapter 4,
and §4.4.2 already says in so many words that optimising the scale set "is
warranted, though it is not attempted here". But it is the weaker of the two
Chapter 5 candidates, for four reasons that are matters of evidence rather than
of taste — and it contains one design flaw that would make the optimisation
degenerate if left uncorrected.

There is also a two-hour experiment that would tell you whether this chapter
has a result in it at all, and you should run that before committing a month.

---

## 1. The design flaw: a residual-based fitness is degenerate over σ

§5.3.2 proposes a fitness "based on error norm convergence, photometric
residual, robustness criterion". **A fitness built on the photometric residual
cannot be used here, and the reason is not subtle.**

Changing σ changes the filters, which changes the feature vector itself. A bank
with larger scales produces smaller high-order responses, so it produces a
smaller ‖**e**‖ *for the same pose error*. The residuals of two candidate σ
vectors are not measurements of the same quantity in different amounts; they
are measurements of different quantities. An optimiser minimising ‖**e**‖ over
σ will therefore not find the scale set that positions best — it will find the
scale set whose responses are smallest, which is a degenerate solution and
quite possibly the worst one.

This compounds a result Chapter 4 already established for a fixed σ: the
unweighted variant attains the *smallest* feature error of the three at the
*worst* pose, by a factor of four hundred, which is why §4.6.4 admits the
feature error as a convergence indicator only and never as a measure of
accuracy.

**The fitness must be the pose error**, which simulation gives you for nothing.
Two consequences follow, and both belong in §5.3.2:

- The optimisation is possible **only in simulation**, since a real system has
  no ground-truth pose. That bounds what the contribution can claim: it is a
  design-time procedure, not an adaptive one.
- If you want a residual in the fitness at all, it must be the **normalised
  ratio** ‖**e**_final‖/‖**e**₀‖, which is invariant to a uniform rescaling of
  the feature vector and therefore comparable across σ sets. Never the absolute
  norm. Say why in the text — the reasoning above is itself worth a paragraph,
  and it demonstrates that you understood the trap rather than avoided it by
  luck.

---

## 2. The technical issue that may explain your own motivating measurement

Chapter 4 measured the per-scale contribution to the feature error as
**23.6 %, 39.6 %, 32.8 % and 4.1 %** for σ = 0.5, 1.1, 1.8, 2.8. The fourth
scale contributes almost nothing, and that is the empirical case for retuning
the set.

But there may be a simpler explanation, and if it is right it changes what
Chapter 5 should optimise.

**Your kernels are 9×9, so the half-width is 4 pixels.** For σ = 2.8 that is
1.43σ. A Gaussian truncated at 1.43σ loses about 15 % of its mass, and a
Hermite function of order 4 has its outermost extremum near √(2n+1)σ = 3σ ≈
8.4 pixels — **twice the available support**. The order-4 responses at σ = 2.8
are therefore not the Gauss–Hermite functions the derivation assumes; they are
severely truncated approximations of them, with the orthogonality between
orders correspondingly damaged. σ = 1.8 is affected too, at 2.2σ of support.

If that is what is happening, the 4.1 % is not telling you *σ = 2.8 is a poor
scale*. It is telling you **9×9 is too small a kernel for σ = 2.8**, and the
remedy is a per-scale support — 9×9, 11×11, 15×15, 21×21, say — rather than a
search over σ at fixed support.

**And it would confound the PSO.** With the support fixed at 9×9, truncation
penalises large σ for reasons that have nothing to do with the information the
scale carries. The optimiser would be driven toward small scales by an artefact
of the implementation, and the resulting "optimal" scale set would be an
artefact too.

**I have now measured this, and it is real.** Taking your `dnFunction` form
and comparing the L₂ energy captured on your 9×9 grid against a 81×81
reference, at unit sample spacing:

| σ | order 0 | order 2 | order 4 | 2-D order (4,4) |
|---|---|---|---|---|
| 0.5 | 1.0000 | 1.0000 | 1.0000 | 1.0000 |
| 1.1 | 1.0000 | 1.0000 | 1.0000 | 1.0000 |
| 1.8 | 0.9997 | 0.9819 | 0.9831 | 0.9666 |
| **2.8** | 0.9784 | **0.7854** | 0.8863 | **0.7855** |

And the half-width needed to capture 99 % of the order-4 energy at each scale:

| σ | required kernel | you use |
|---|---|---|
| 0.5 | 5×5 | 9×9 ✓ |
| 1.1 | 7×7 | 9×9 ✓ |
| 1.8 | 11×11 | 9×9 — marginal |
| 2.8 | **17×17** | 9×9 — **insufficient** |

So the fourth scale loses roughly **twenty-one per cent** of the energy of its
order-(4,4) response, and twenty-one per cent of its 1-D order-2 response, to
truncation. The first two scales lose nothing; the third is marginal.

That is a large enough shortfall to be a substantial part of the 4.1 % you
measured, and it means the honest reading of that measurement is not "σ = 2.8
carries little information" but **"a 9×9 kernel cannot represent σ = 2.8"**.
The two are quite different conclusions and they point to different chapters.

Two consequences:

- **§5.3.3's bounds.** At fixed 9×9 support, σ must be capped near 1.3 to keep
  the orders orthogonal — which excludes two of your four current scales. A PSO
  run at fixed support would be driven toward small σ by truncation rather than
  by information, and its "optimum" would be an artefact of the kernel size.
- **This is worth a sentence in Chapter 3 or §4.4.2 regardless of Chapter 5**,
  since it is a limitation of the filter bank as implemented and it explains a
  measurement the thesis already reports. It does not invalidate anything —
  the scheme works, and Chapter 4's results stand — but the fourth scale is
  not delivering the Gauss–Hermite response the derivation assumes, and saying
  so is better than having it noticed.

---

## 3. The cost, with numbers

Every fitness evaluation is a full servo run, because the fitness is the pose
error at convergence.

| | Runs | At 0.5 s/iteration, 600 iterations |
|---|---|---|
| One fitness evaluation, one pose | 1 | 5 min |
| 15 particles × 20 generations | 300 | **25 hours** |
| ...averaged over 3 poses | 900 | **75 hours** |
| ...× 2 scenarios (nominal, occluded) | 1800 | **150 hours** |
| ...§5.7 sensitivity over 5 PSO settings | 9000 | **31 days** |

The 0.5 s figure is the best in your logs; the earlier snapshot showed 8–11 s
per iteration, which multiplies all of that by eighteen. Run without display.

It is not impossible — 25 hours is a weekend, if you accept one pose and one
scenario. But note what the budget forces: a single-pose, single-scenario
optimisation is precisely the overfitting a reviewer will attack (see item 4),
and §5.7's sensitivity analysis, which is *not* optional for a metaheuristic
chapter, is what makes the arithmetic unaffordable. Compare with fuzzy gain
scheduling, whose whole campaign is about sixty runs and five hours.

If you go this way, the budget is spent well as follows: short-horizon fitness
(pose error at 300 iterations rather than 600), three poses, one scenario for
the optimisation and the others held back for testing, 15 particles × 20
generations. That is about 37 hours and it is defensible.

---

## 4. The criticism most likely to be fatal, and how to disarm it

σ tuned on one texture at one pose is σ tuned for *that texture at that pose*.
A chapter that optimises on the peppers image and reports improved accuracy on
the peppers image has reported overfitting, and a jury will say so in the first
five minutes.

**The chapter needs a train/test split, and it needs to be visible in the
protocol, not conceded in the discussion.** Optimise on one texture and a
subset of poses; report on held-out textures and held-out poses. Expect the
gain to shrink, and report that shrinkage — it is the most scientifically
interesting number the chapter can produce, because it measures how
scene-specific the optimal scale set is. If the held-out gain is near zero, the
honest conclusion is that the empirical scale set is adequate and that scale
tuning does not transfer, which is a real result and a defensible chapter.

Two smaller points in the same direction:

- **§5.4.4, "offline vs online".** Online is not available: you cannot run
  three hundred servo simulations inside a control loop. Say so plainly rather
  than leaving both options open. And note the consequence — a chapter that
  tunes parameters offline cannot later criticise offline tuning, which is
  exactly what the alternative Chapter 5 plan's §5.1 does. The two proposals
  are not merely different; on this point they are opposed.
- **§5.6.2, robustness to illumination.** You do not have this experiment yet,
  and σ is the wrong lever for it. Uniform brightness change adds a DC offset
  to the image, and what determines sensitivity to that is **which Hermite
  orders you use**, not their scales: the odd orders are zero-mean and immune,
  while D₀,₀ is a pure Gaussian and is fully exposed. That is the same DC
  property Chapter 4 turned up in the structure measure. So if illumination
  robustness matters, the decision variable is the **order set**, not σ — see
  item 6.

---

## 5. Against the fuzzy-gain alternative, on the evidence

| | Fuzzy gain scheduling | PSO over σ |
|---|---|---|
| Headroom measured in Ch. 4 | **three orders of magnitude** | unmeasured; 4.1 % energy on one scale, possibly a truncation artefact |
| Campaign cost | ~60 runs, ~5 hours | 300–9000 runs, 25 h – 31 days |
| Objective | metrics already defined in §4.6.4 | degenerate unless rebuilt on pose error; simulation-only |
| Generalisation | scene-independent by construction | scene-specific; needs a train/test split |
| Usable on real hardware | yes | no — needs ground truth |
| De-risking experiment | Arm A′, one evening | σ sweeps, two hours |
| Novelty | moderate | low — tuning a parameter with a metaheuristic is the canonical recipe |

The last row deserves saying directly, because you set the standard yourself
earlier in this work when you asked for a research contribution rather than a
known recipe. "Apply PSO to tune the parameters of X" is about as close to a
known recipe as this field has. That does not make it wrong — theses contain
such chapters and pass — but it is a weaker claim than the Hermite-informed
weighting of Chapter 4, whereas a gain scheduler that resolves a tension
Chapter 4 measured would be a comparable one.

The decisive row is the first. Chapter 4 handed you a measured
three-order-of-magnitude deficiency in the gain schedule. It handed you no
comparable measurement about σ.

---

## 6. If you want PSO, three better places to put it

Your interest in PSO is longstanding and there is no reason to abandon it. But
these three targets are stronger than four continuous scales.

**(a) The Hermite order set, by binary PSO.** The feature uses orders up to
n = m = 4, which is twenty-eight convolutions. Which of those orders actually
carry pose information is an open question that Chapter 4 does not answer, and
selecting a subset is a combinatorial problem PSO handles naturally. It attacks
two things at once that σ attacks neither of: the computational cost, which
scales directly with the number of orders retained, and illumination
sensitivity, which depends on whether DC-bearing orders are included. The
search space is larger but each evaluation is cheaper, since a smaller bank
runs faster.

**(b) The membership functions of the fuzzy scheduler.** This combines both
Chapter 5 proposals instead of choosing between them, and it answers the one
criticism the fuzzy chapter is genuinely exposed to — that the rule base and
the partition are hand-placed. It keeps the measured three-order-of-magnitude
headroom as the chapter's payoff and uses PSO where hand-tuning is weakest.
The chapter would then be *fuzzy gain scheduling with PSO-tuned partitions*,
which is a fuller "intelligent techniques" story than either alone.

**(c) The scales *and* their supports jointly**, if you keep this proposal. Per
item 2, σ and the kernel size cannot sensibly be decoupled, so let PSO search
pairs (σ_i, k_i) under the constraint k_i ≥ 6σ_i + 1. That turns the truncation
problem from a confound into part of the formulation, and it is a more
interesting optimisation than four scales at fixed support.

---

## 7. The free result you should take either way

Chapter 4 measured the fourth scale at **4.1 %** of the feature error energy.
Dropping it gives a feature vector of 198 000 rather than 264 000 components —
**a quarter fewer measurements and a quarter less convolution work** — for what
is very likely a negligible loss of accuracy.

That is **one run** to verify, it needs no optimiser, and it is a reportable
result: a quarter of the computational cost removed at no measurable cost in
accuracy. Do it this week whichever chapter you write. And if it turns out the
fourth scale *does* matter to the pose error despite contributing 4.1 % of the
energy, that is a more interesting finding still, because it would mean energy
share is not a proxy for information content — which is worth knowing before
you build a fitness function on either.

---

## 8. What I would do

1. ~~The truncation check~~ — **done, and it came back positive** (item 2).
   The fourth scale loses a fifth of its energy to the 9×9 support, so the
   4.1 % that motivates this proposal is at least partly an artefact of the
   kernel size. Before optimising σ, either enlarge the supports (17×17 for
   σ = 2.8) and re-measure the per-scale energies, or accept that the search
   must be over (σ, kernel size) pairs. **Re-measuring the four energies with
   adequate supports is the single most informative run available to you right
   now**, because it tells you whether the scale set is genuinely unbalanced or
   merely badly discretised.
2. **Four one-dimensional σ sweeps**, varying one scale at a time over six
   values, with the **final pose error** as the output. Twenty-four runs, about
   two hours. This is the experiment that decides whether this chapter exists:
   if the pose error varies by a couple of per cent across the whole plausible
   range of σ, the landscape is flat, PSO has nothing to find, and you have
   learned it in an afternoon instead of a month. If it varies materially, or
   non-monotonically, the chapter has a result and PSO is justified.
3. **Drop the fourth scale and measure** (item 7). One run, and a result either
   way.
4. **Then choose.** If step 2 shows a flat landscape, write the fuzzy gain
   chapter — Chapter 4's measurement says where the accuracy is. If it shows a
   rugged one, this chapter is viable, and option 6(c) is the version of it I
   would write.

My recommendation, stated plainly so you can disagree with it: **fuzzy gain
scheduling is the stronger Chapter 5, and PSO belongs inside it as 6(b) rather
than as a chapter of its own.** But steps 1 and 2 cost one afternoon between
them and they answer the question with your own data rather than my judgement,
which is the better basis for a decision of this size.

---

## 9. A small consequence of dropping the future-work section

The seven forward references are gone, but two sentences in Chapter 4 now say
"not attempted here" about the very thing Chapter 5 will attempt:

- **§4.4.2** — "the unequal contribution of the four scales ... indicates that
  such an optimisation is warranted, though it is not attempted here."
- **§4.7.2** — "A gain that does not vanish with the error is accordingly the
  most immediate improvement available to the scheme."

Whichever topic you choose, the matching sentence should point forward to
Chapter 5 by chapter reference — "and is taken up in Chapter~\ref{ch:...}" —
which reads better than either the section reference I removed or the bare
"not attempted here". The other sentence stays as it is. That is one edit, made
once you have decided.
