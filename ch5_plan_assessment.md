# Chapter 5 plan — assessment

**Short answers.** Yes, it can be coded, and easily — about 200 lines with no new
dependency, plus two lines changed in the existing servo loop. Yes, it is
correct in outline. And yes, it is the best of the candidates I can see, for a
reason that is not a matter of taste: Chapter 4 *measured* the gain schedule to
be the largest single source of error remaining in the scheme.

But the plan has one omission that decides whether the chapter survives a
viva, and five technical problems that Chapter 4's own data expose. Those are
items 3 to 5 below. Read item 3 first if you read nothing else.

---

## 1. Why the topic is right, with the evidence

This is not an arbitrary next step. Chapter 4 arrived at it by accident and
quantified it:

- The gain is proportional to the squared intensity error and the commanded
  velocity is proportional to the residual in addition, so the velocity decays
  roughly as the **cube** of the error. The nominal runs therefore stall while
  a sub-millimetre misalignment remains — not because the residual has been
  minimised, but because the gain has died with it.
- The occluded runs, in which a fixed residual keeps the gain some **54 times
  higher**, reach the desired pose **three orders of magnitude** more closely.
- That mechanism is established independently, not inferred: the damping and
  the gain are driven by the same quantity with a fifth-power relation between
  them, and the predicted and observed damping ratios agree to **0.62 %**
  across three runs, the exponent recovered from the data being 9.997 against
  the 10 of the damping law.

No other candidate for Chapter 5 has a measured three-order-of-magnitude
headroom sitting in the previous chapter's results. And the damping is worse
than under-used: because µ ∝ ‖e‖¹⁰, it falls from 1.9×10⁻² to 5.3×10⁻¹⁸ over a
single run. **The scheme has no trust-region protection whatever for the last
99 % of its iterations.** A scheduler that keeps µ in a sensible range is
therefore attacking something genuinely absent, not tuning something already
adequate.

The three-layer structure — features in Chapter 3, weighting in Chapter 4,
gain in Chapter 5, each the sole operative variable with the others held fixed
— is also a clean thesis architecture, and Chapter 4 now supplies Chapter 5's
motivation from measurement rather than from assertion. That is worth a great
deal in a defence.

**On fuzzy specifically, against the alternatives.** It is the right tool here:
it needs no training data, which matters because you cannot easily label
"optimal" gains; it is interpretable, so the rule table and the control
surfaces are printable and defensible as design rather than as a fitted black
box; its cost is negligible against 264 000-dimensional convolutions; and it
sits squarely under "intelligent techniques". Note in passing that **PSO would
be self-defeating as the main idea here**, because optimising gains offline per
scenario is precisely the "set offline, scenario-agnostic" limitation §5.1
sets out to criticise. Where PSO *would* fit is tuning the membership-function
placement offline — which is a legitimate answer to §5.7.4's sensitivity
question, and a way to keep the idea you liked earlier without contradicting
the chapter's premise.

---

## 2. Can it be coded — concretely

Yes, and it is the easiest of the three chapters to implement. ViSP has no
fuzzy component, so hand-roll it; that is better for the thesis anyway, since
everything is reproducible and there is no external version to pin.

**A `FuzzyGainScheduler` class, roughly 200 lines, standard library only:**

- triangular/trapezoidal membership evaluation (a dozen lines);
- min–max Mamdani inference over the rule table (a nested loop over rules);
- centre-of-gravity defuzzification on a discretised output universe of, say,
  101 points (a weighted sum);
- two outputs, so two rule tables or one table with two consequents.

**Insertion into the existing loop** is a two-line replacement, exactly where λ
and µ are currently computed from the intensity error. Everything downstream —
the interaction matrix, **D**, the Hessian, the floor — is untouched.

**Also needed, and small:**

- a mode flag so the arms share identical non-gain logic;
- per-iteration logging of ē, Δē, λ, µ for the diagnostics;
- a separate tiny program that sweeps (ē, Δē) on a grid and dumps the surface
  for the §5.3.6 figures. Keep this out of the servo loop — it is offline and
  it does not need the simulator at all.

I can write the scheduler and the integration whenever you want it.

---

## 3. The omission that matters most: the plan compares against the wrong baseline

§5.6.4 defines Arm A as fixed gains at the Chapter 4 configuration and Arm B as
fuzzy-scheduled gains. That comparison will show the fuzzy system winning, and
it will not survive the first question.

The question is: **a one-line clamp, λ ← max(λ, λ_min), also stops the gain
vanishing.** Chapter 4 already proposes exactly that. So what did
twenty-five rules, two input partitions, a Mamdani engine and a
defuzzification step buy over `std::max`?

Unless the chapter answers that, a reader is entitled to conclude the fuzzy
machinery is decoration on a clamp. And I think the clamp will in fact capture
most of the final-accuracy gain, because pinning λ is precisely what the
occluded runs did accidentally — λ sat at 2.19 and the pose came to within
three micrometres.

**So the ablation needs three arms, not two:**

| Arm | Gains | What it isolates |
|---|---|---|
| A | Chapter 4 law, unmodified | the baseline as it stands |
| A′ | Chapter 4 law with floors λ ≥ λ_min, µ ≥ µ_min | how much of the gain is just "don't let it vanish" |
| B | Fuzzy-scheduled | what *scheduling* adds over *flooring* |

This is the same discipline Chapter 4 applied when it required the three
variants to differ only in **D**, and declared the fourth configuration rather
than letting a reader discover it. Applying it here is consistency, not
pedantry.

**And it reframes the chapter in a way that is better, not worse.** If the
floor captures the final-accuracy gain, then the fuzzy system's case rests on
the two things a clamp cannot do:

- **iteration count** — a clamp cannot raise λ early to converge faster;
- **convergence domain** — a clamp cannot raise µ when the linear model stops
  being trustworthy, and Chapter 4 shows µ is effectively switched off after a
  handful of iterations, so there is real room here.

That makes §5.7.3 the headline of the chapter rather than §5.7.1, and it is a
claim about something a scalar law structurally cannot do. That is a much
stronger position than "our gains are better tuned".

---

## 4. The strongest argument for the fuzzy approach, which the plan does not make

There is a tension in Chapter 4's findings that a two-input scheduler can
resolve and a scalar law cannot.

- Chapter 4 wants λ **not** to vanish, or the servo stalls a millimetre short.
- But Chapter 4 also reports that Variant 3 comes to rest with a commanded
  velocity **thirty-four times** Variant 2's while its pose stops improving —
  a terminal jitter, caused by weights that keep changing after the pose has
  settled. Damping that jitter wants λ **smaller** near convergence.

A single monotone function of the residual cannot do both. A scheduler with a
second input can, because it can distinguish the two situations:

- residual still appreciable, no progress → **gain too low** → raise λ;
- residual small, error increasing or alternating → **overshooting** → lower λ.

Both are "no progress"; they require opposite actions; and telling them apart
is exactly what the second input is for. That is the argument that makes the
fuzzy layer necessary rather than ornamental, and it has the added merit that
**Chapter 5's contribution would then repair a defect Chapter 4 measured in
Chapter 4's own contribution.** Put that in §5.1 and add terminal jitter —
‖V_c‖ after the pose has settled — to the §5.6.3 metrics, since you now have
a baseline number for it.

---

## 5. Five technical problems

### 5.1 The universes of discourse must be logarithmic

ē = ‖e_k‖/‖e_0‖ falls from 1 to about 2×10⁻⁴ over a nominal run — **3.7
decades**. Partition [0, 1] linearly into five triangular sets and everything
below 0.2 lands in one of them, which is to say the whole of the run after the
first few iterations. All the behaviour the scheduler must discriminate is
crushed into the leftmost membership function.

Use log ē on the universe, or normalise differently. This is not a refinement;
a linear partition makes the scheduler inert over the region where the stall
you are trying to fix actually happens.

### 5.2 Δē is nearly uninformative as a progress measure, and valuable as an oscillation detector

Chapter 4 reports the decay monotone in every run: no plateau before
convergence, no rebound, no secondary minimum. So Δē is almost always negative
and shrinks to zero as the run settles — and it shrinks to zero both when the
servo has converged *and* when it has stalled with error remaining. As a
progress meter it is weak, and a 5×5 rule table on (ē, Δē) will have most of
its cells never visited, which an examiner will spot in the logs.

Two fixes, and they compose:

- **Keep Δē signed and design the rules around its sign**, not its magnitude.
  A positive Δē is the upswing of an oscillation, and "ē small **and** Δē
  positive → lower λ" is the rule that catches the jitter of item 4. The
  signed quantity earns its place this way; the magnitude quadrants do not.
- **Consider the relative rate Δē/ē instead of, or alongside, Δē.** It does not
  degenerate as ē → 0, so it stays informative exactly where raw Δē dies.

A third option is worth knowing about because it is the textbook quantity for
this job: the **trust-region ratio**, ρ = achieved reduction / reduction
predicted by the linear model. Classical Levenberg–Marquardt adapts µ from
precisely this. You already have **L**, **e** and **V**_c, so ρ costs almost
nothing, and a fuzzy system on (ē, ρ) is both more principled and easier to
defend than one on (ē, Δē). It is a bigger departure from your plan, so I
raise it rather than recommend it — but it is what a reviewer from the
optimisation side will ask why you did not use.

Start with a **3×3** table, not 5×5. Nine rules you can justify individually
beat twenty-five of which fifteen never fire.

### 5.3 §5.4.2's stability claim is stronger than anything derivable

"Admissible bounds on λ, µ ensuring Ċ < 0 in the neighbourhood of r*" cannot be
delivered for a 264 000-feature photometric scheme with an approximated
pseudo-inverse. Chapter 4's §4.5, as corrected, already concedes that only
local stability is available, that the analysis holds for the frozen system at
each instant, and that local minima cannot be excluded. Adding a time-varying
gain makes that worse, not better — now **D** *and* (λ, µ) both vary.

What you *can* say is clean, correct and sufficient:

> The damped Hessian **H**_D + µ diag(**H**_D) + ε**I** is positive definite by
> construction, so **V**_c is a descent direction for **every** λ > 0 and
> µ ≥ 0; λ scales the step without altering its direction. Instability can
> therefore arise only from a step long enough to leave the region in which
> the linearisation holds, which bounds what λ_max must guarantee. That bound
> is not available analytically and is set empirically.

That is a better section than the one planned, because it is true, and because
it turns λ_max from an arbitrary constant into the one parameter with a stated
role.

### 5.4 Two metrics in §5.6.3 need redefining

- **"Iterations to threshold"** — Chapter 4 established that the inherited
  absolute threshold ‖e‖² < 10⁴ is never reached in any run, which is why §4.6.4
  had to be rewritten around the commanded velocity. Define it as iterations
  until ‖V_c‖ < 10⁻³ m s⁻¹, or until the pose error comes within a stated
  factor of its final value. Otherwise every run "fails" again.
- **"Terminal photometric cost"** — Chapter 4's central demonstration is that
  the unweighted scheme attains the *smallest* feature error at the *worst*
  pose. §4.6.4 accordingly admits the feature error as a convergence indicator
  only, never as a measure of accuracy. Carry that policy into Chapter 5
  explicitly, or the chapter contradicts its predecessor.

### 5.5 §5.2.4 may be criticising the wrong law

The plan calls the reference law "the exponential heuristic law of [30]". The
law in your code is a **power** law, λ = ‖e_I‖²/N_e^0.8 with µ ∝ ‖e_I‖¹⁰ — not
an exponential. Check whether [30] is the source of the power law or of a
different adaptive-gain form, because §5.2.4's criticism has to land on the law
you are actually replacing. If it is an exponential in the reference and a
power law in the code, say so and criticise the code's law, since that is
Arm A.

---

## 6. Two things in the plan that are blocked right now

### The implementation flag contradicts a decision you already made

§5.5 proposes "extension of the unified ablation binary with a gain-mode flag".
In Chapter 4 you chose three separate `.cpp` files over a single binary with a
mode switch, and I agreed at the time.

**For Chapter 5 the opposite is right, and the reason is in the plan itself.**
The stated invariant is bit-identical non-gain logic across the arms. With
three arms, separate files means triplicating six hundred lines and hoping they
do not drift — and one accidental divergence invalidates the whole comparison.
A flag makes the invariant structural rather than a matter of care. Chapter 4's
reasoning does not carry over because there the arms differed in a *block* of
logic; here they differ in *two lines*.

### Arm A is not reproducible until one question about the code is answered

This is a genuine blocker, and it is the second invariant the plan itself
names. Chapter 4 turned up that **the printed gain and damping do not satisfy
the printed equations**: with λ = 2.33425 and N_e = 45.0219, the formulas give
µ = 0.72 against the 3.24×10⁻⁹ logged, and the error quantity µ uses is smaller
than λ's by a constant factor of 6.83 at every operating point. Also N_e =
45.0219 is not a count of measurements, though the thesis writes it as though
it were.

Arm A *is* that law. If Chapter 4 does not state it correctly, Chapter 5's
baseline is undefined and both chapters carry the same error. It is one look
at two lines of code.

### And the answer to the plan's other invariant: carry Variant 3 forward

The plan says Arm A is undefined until "the Ch. 4 variant discrepancy" is
resolved. Chapter 4 leaves Variants 2 and 3 tied — Tukey better in translation
by 28 %, Hermite better in rotation by 8 %. **Carry Variant 3.** Three reasons:

1. It is the thesis's own contribution. Carrying Variant 2 into Chapter 5 would
   amount to abandoning Chapter 4's proposal in the chapter after proposing it.
2. The layered narrative requires each layer to be *the proposed* one, with the
   others held fixed. Variant 3 is that layer.
3. Variant 3 is the variant with the terminal jitter, which is the defect
   item 4 argues a scheduler can repair. Carrying Variant 2 forfeits the best
   available cross-chapter result.

State the choice and the reason in §5.6.4, with the parity result cited. A
reader who sees Variant 3 carried forward without explanation will assume you
believe it won; a reader who sees the choice justified on architecture will
credit the candour.

---

## 7. One experimental-design point about §5.6.2

Ten initial poses is the right answer to Chapter 4's single-run weakness, and
it is the most valuable thing in the protocol.

But **§5.7.3 cannot report a convergence-domain extension unless some poses
defeat Arm A.** Chapter 4 used one pose at which all three variants converged;
ten more of the same kind would show ten more convergences and measure no
domain at all. So sweep the displacement magnitude first, with Arm A alone,
until it stalls badly or diverges — then place the ten poses across that
boundary, some inside and some outside. The sweep is cheap and it tells you
where to put the campaign.

On cost: three arms × ten poses × two scenarios × 600 iterations is sixty runs.
At the ~0.5 s per iteration seen in your recent logs that is about five hours;
at the ~9 s per iteration seen in the earlier ones it is four days. Run without
display.

---

## 8. Recommended revision of the plan

Keep the structure. The changes are:

- **§5.1** — motivate from Chapter 4's measurement (the cubic decay, the 54×
  gain ratio, the three orders of magnitude, the damping switched off after a
  handful of iterations). Add the tension of item 4 as the reason a *two-input*
  scheduler is needed rather than a better scalar law.
- **§5.2.4** — criticise the law that is actually in the code (item 5.5), and
  state plainly that µ ∝ ‖e‖¹⁰ leaves the scheme undamped for almost the whole
  run. That is the opening for §5.3.
- **§5.3.1–5.3.3** — logarithmic universe for ē; signed Δē with the rule base
  built around its sign; 3×3 to begin with. Mention the trust-region ratio as
  the alternative considered and say why you did or did not take it.
- **§5.4.2** — replace the Ċ < 0 bounds with the positive-definiteness
  argument of item 5.3.
- **§5.5** — single binary with a gain-mode flag, and say why this reverses
  Chapter 4's choice.
- **§5.6.2** — sweep for Arm A's failure boundary first, then site the ten
  poses across it.
- **§5.6.3** — redefine "iterations to threshold"; carry forward §4.6.4's
  policy on the feature error; **add terminal jitter as a metric**.
- **§5.6.4** — **three arms: A fixed, A′ floored, B fuzzy.** This is the change
  that matters.
- **§5.7** — expect §5.7.3 (convergence domain) and the iteration count to
  carry the chapter, with final accuracy shared between A′ and B.
- **§5.7.4** — keep the sensitivity study; it is not optional for a
  hand-designed rule base. PSO over the membership functions belongs here if
  you want it.

---

## 9. What to do first

1. **Settle the λ and µ code question.** Arm A depends on it and so does
   Chapter 4. Two lines.
2. **Add the floors and run Arm A′ at the Chapter 4 pose.** One evening. If
   flooring λ recovers most of the three orders of magnitude, you know the
   shape of your chapter before writing any of it — and you will have framed
   §5.7 around convergence domain rather than around a result A′ already owns.
3. **Sweep for Arm A's failure boundary**, so the ten poses are sited where
   they measure something.
4. **Then** write the scheduler. It is 200 lines and it is the least risky part
   of the chapter.

Doing 2 before 4 is the important ordering. It is the experiment that tells you
what the fuzzy layer has to beat, and it is far better to learn that from your
own run than from a question at the defence.
