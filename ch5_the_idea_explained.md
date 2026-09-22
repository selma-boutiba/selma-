# How fuzzy scheduling applies to HER-PVS, and why — step by step

---

## Part 1. What your control loop does now

Every iteration of the `do { } while` loop in your code does six things. Here
they are with the lines that do them.

**Step 1 — move the camera and take a picture.**
```cpp
sim.setCameraPosition(cMo);  sim.getImage(I, cam);
```

**Step 2 — turn the picture into features.** `Ihermite()` convolves the image
with the four Hermite filters *D*(0.5), *D*(1.1), *D*(1.8), *D*(2.8), and
`sI.buildFrom(...)` stacks the responses into the feature vector *R*.
**This is Chapter 3.**

**Step 3 — measure how wrong you are.**
```cpp
sI.error(sId, error);                  // e = R - R*   (the feature error)
errorI[k] = Id[i][j] - I[i][j];        // the raw intensity error, separately
```

**Step 4 — work out which way to move.**
```cpp
sI.interaction(Lsd);      // L : how R changes when the camera moves
Hsd = Lsd.AtA();          // H = L^T L
```
In Variant 3 this is also where the robust weighting enters, multiplying both
*L* and *e* by the weights. **This is Chapter 4.**

**Step 5 — decide how *far* to move.**
```cpp
lambda = pow(10, (log10(normeErrorI) - 0.8*log10(Ne)));
mu     = pow(10, (5*log10(normeErrorI) - 5.2*log10(Ne)));
```
**These two lines are Chapter 5.** Nothing else.

**Step 6 — command the motion.**
```cpp
H = ((mu*diagHsd) + Hsd).inverseByLU();
e = H * Lsd.t() * error;
v = -lambda*e;
robot.setVelocity(vpRobot::CAMERA_FRAME, v);
```

So the loop separates cleanly into **a direction** (steps 2–4, your first two
chapters) and **a step length** (step 5). Chapter 5 changes only the step
length. That is why the three-layer architecture works: each chapter owns one
factor of the same product, and the others are held fixed.

---

## Part 2. What those two lines actually compute

Your published paper gives them as equations (24) and (25):

> λ = 10^(log₁₀(N_e) − 0.8 log₁₀(N_ei)),  µ = 10^(5 log₁₀(N_e) − 5.2 log₁₀(N_ei))
>
> "N_e represents the norm of the difference between a current and a desired
> image, while N_ei represents the norm of the difference between an initial
> image and a desired image."

In the code, `normeErrorI` is N_e and the C++ variable `Ne` is N_ei — the
*initial* error, captured once at iteration 2 and never changed. Write

> **ē = N_e / N_ei**

for the error relative to where it started: ē = 1 at the beginning, and it
falls to about 8×10⁻⁴ by the end of a nominal run. Then the two lines are,
*exactly*:

> **λ = N_ei^0.2 · ē**   and   **µ = ē⁵ / N_ei^0.2**,  with N_ei^0.2 = 52.69

I verified this against your logs to four significant figures — for instance
ē = 7.726×10⁻⁴ gives λ = 0.040701 against the 0.0407037 logged, and
µ = 5.22×10⁻¹⁸ against 5.259×10⁻¹⁸.

Read in that form the law says something very simple:

- **λ is directly proportional to the error.** Nothing else.
- **µ is the fifth power of the error.** Nothing else.
- The only scene-dependent quantity, N_ei, is a *constant* fixed at iteration 2.

The law therefore has exactly one input — how big the error is — and no
knowledge whatever of what the servo is *doing*.

---

## Part 3. Why that is a problem — three consequences

### 3.1 It guarantees the servo stalls before it arrives

Near the solution the residual is proportional to the pose error ε, so
ē ∝ ε². The commanded velocity is λ times a step that is itself proportional
to the residual:

> ‖**V**_c‖ ∝ λ · ‖**e**‖ ∝ ε² · ε = **ε³**

A velocity proportional to the cube of the error gives, in continuous time,
dε/dt = −cε³, whose solution is

> ε(t) ∝ 1/√t

That is *algebraic* decay, and it is hopeless: to halve the remaining error you
must quadruple the number of iterations. This is exactly what Chapter 4
measured — the runs stall at 0.34 to 1.0 mm and the residual settles on a floor
three to four orders of magnitude above the stopping threshold, while the
commanded velocity dies to 10⁻⁴ m/s.

Now suppose λ simply stops falling below some λ_min. Then

> ‖**V**_c‖ ∝ λ_min · ε,  so dε/dt = −c′ε,  so ε(t) ∝ e^(−c′t)

*Geometric* decay. The camera keeps closing the gap at a constant percentage
per iteration instead of creeping to a halt.

**And Chapter 4 accidentally ran this experiment.** Under occlusion the
intensity error cannot decay to zero, because the reference has no counterpart
to the occluder — so N_e stays large, ē stays around 4×10⁻², and λ sits at 2.19
instead of collapsing to 0.041. The result: the pose error came down to
**2.5 micrometres** instead of stalling at a millimetre. Three orders of
magnitude, from nothing but the gain refusing to vanish. That is the single
strongest piece of evidence in your thesis for Chapter 5.

### 3.2 It pushes *harder* when things are going *wrong*

This one is worse than the stall, and it is structural.

Suppose a step overshoots and the error goes **up**. Then ē goes up, and since
λ ∝ ē, **the law responds by increasing the gain.** It takes an even longer
step next time. A law whose only input is the error magnitude cannot tell the
difference between "the error is large because I have not started yet" and "the
error is large because I just overshot" — and those need opposite responses.

At ē = 0.5 the reference law commands λ = 26.3, whether ē is 0.5 on the way
down or 0.5 on the way back up.

### 3.3 The damping is switched off almost immediately

µ is the Levenberg–Marquardt damping: µ → 0 makes the scheme Gauss–Newton
(fast, but it needs the linearisation to hold), µ large makes it gradient
descent (slow, but safe from anywhere). It is your safety net when the linear
model is untrustworthy.

But µ ∝ ē⁵, so **one decade of error costs five decades of damping**. From
1.9×10⁻² at the start it reaches 1.9×10⁻⁷ once the error has fallen by a single
decade, and 5×10⁻¹⁸ by the end. For almost the whole trajectory the scheme runs
as undamped Gauss–Newton with no trust region at all.

---

## Part 4. Why a floor is not enough, and what is actually missing

The obvious fix is one line:

```cpp
lambda = std::max(lambda, lambdaMin);
```

**You should run this** — it is Arm A′ in the protocol, and it will probably
recover most of the accuracy. But a floor is blind. It applies the same minimum
gain in every situation, and Chapter 4 handed you two situations that are
**identical in ē** and require **opposite** gains:

| situation | what ē says | what should happen |
|---|---|---|
| **the stall** (§4.7.1) | small, unchanging | the step is too short → **raise** λ |
| **Variant 3's jitter** (§4.7.2) | small, unchanging | the step is too long → **lower** λ |

Chapter 4 measured the second one: Variant 3 ends with a commanded velocity
**thirty-four times** Variant 2's while its pose stops improving. The velocity
is not unfinished descent; it produces no progress. The weights keep moving the
minimum slightly under the controller, and the camera chases it.

**No function of ē alone can separate those two rows.** They have the same
input. To tell them apart you need a second input, and the natural one is:
*which way is the error going?*

| | error going **down** | error **flat** | error going **up** |
|---|---|---|---|
| **far from solution** | good — push | too short — push | overshoot — back off |
| **near solution** | good — keep going | converged or stalled | jitter — back off |

That table is the whole idea. Two inputs, and the two failure modes Chapter 4
found now sit in different cells and get different answers.

---

## Part 5. Why *fuzzy* in particular

You could write a formula for a 2-D surface λ(ē, trend). Five reasons not to:

1. **There is no principled form to write.** With one input you could argue for
   a power law. With two, any formula you invent is arbitrary — and then you are
   defending arbitrary exponents instead of defending statements.
2. **Fuzzy rules are defensible one at a time.** "If the error is medium and not
   decreasing, then the gain is high" is a sentence a jury can accept or
   challenge on its own. Nine such sentences are your design, and they go in the
   thesis as a table.
3. **No training data.** A neural network or reinforcement learning would need
   labelled examples of the *correct* gain, which you cannot produce — you would
   have to solve the problem to generate the data. Fuzzy needs only your
   understanding of the six cells above.
4. **The outputs are bounded by construction.** λ ∈ [λ_min, λ_max] always. The
   gain cannot vanish (which fixes the stall) and cannot explode (which is your
   stability guarantee). A formula would need clamps bolted on; here the bounds
   *are* the design.
5. **The surface is smooth.** Fuzzy interpolation means λ changes continuously
   as the run moves between regimes — no discontinuous jumps in commanded
   velocity, which a switch-based rule (`if (stalled) lambda = 5;`) would give
   you.

And a sixth, for your title: this is what "intelligent techniques" means in a
control thesis. It is a technique that reasons about the state of the process
in linguistic terms rather than evaluating a fixed formula.

---

## Part 6. Where it plugs in — the whole change

**Delete two lines. Add one.** In the loop:

```cpp
// lambda = pow(10, (log10(normeErrorI) - 0.8*log10(Ne)));     // delete
// mu     = pow(10, (5*log10(normeErrorI) - 5.2*log10(Ne)));   // delete
computeGains(gainMode, normeErrorI, Ne, fuzzy, lambda, mu);     // add
```

And before the loop:

```cpp
#include "fuzzy_gain_scheduler.h"
GainMode gainMode = GAIN_FIXED;      // -g 0 = A, 1 = A' (floored), 2 = B (fuzzy)
FuzzyGainScheduler fuzzy;
```

Nothing else changes — not the Hermite filters, not the interaction matrix, not
the weighting, not the Hessian, not the ε floor. That is what makes it a clean
ablation: **`gainMode` is the only thing that differs between arms.**

Inside, the scheduler does four things per iteration:

1. **ē = N_e / N_ei**, then maps it onto a logarithmic scale, because ē spans
   more than three decades and a linear scale would crush the whole endgame
   into one membership function.
2. **trend = (ē_k − ē_{k−1}) / ē_{k−1}** — the *relative* change, not the raw
   difference. The raw difference goes to zero as ē does whether you are
   converging or stalled, so it tells you nothing late in the run; the relative
   rate stays meaningful.
3. Evaluates the nine rules, each firing to the degree its two conditions hold,
   and blends their conclusions (Mamdani min–max, centre-of-gravity).
4. Maps the results onto [λ_min, λ_max] and [µ_min, µ_max], both
   logarithmically.

---

## Part 7. What the rules say, and how they behave

The nine rules, with the two Chapter 4 defects marked:

| ē \ trend | decreasing | flat | increasing |
|---|---|---|---|
| **large** | λ High, µ Low | λ High, µ Med | λ Low, µ **High** |
| **medium** | λ Med, µ Low | λ **High**, µ Low ← *the stall* | λ Low, µ Med |
| **small** | λ Med, µ VLow | λ Med, µ VLow | λ **Low**, µ Low ← *the jitter* |

Compiled and run at the operating points Chapter 4 measured:

| state | ē | **old** λ | **new** λ | **new** µ |
|---|---|---|---|---|
| start | 1.0 | 52.7 | 51.5 | 3.8×10⁻⁴ |
| descending, mid | 10⁻² | 0.53 | 7.9 | 2.6×10⁻⁵ |
| **stalled, mid** | 10⁻² | 0.53 | **51.5** | 2.6×10⁻⁵ |
| near solution, descending | 8×10⁻⁴ | 0.042 | 7.9 | 4.8×10⁻⁶ |
| **stalled near solution** | 7.7×10⁻⁴ | 0.041 | **9.2** | 4.3×10⁻⁶ |
| **jitter near solution** | 7.7×10⁻⁴ | 0.041 | **1.5** | 3.7×10⁻⁵ |
| **error rising, far** | 0.5 | **26.3** | **1.4** | 1.2×10⁻² |

Four rows are the argument of the chapter:

- Rows 2–3 have **the same ē** and the old law gives them the same λ. The new
  one gives 7.9 when descending and 51.5 when stalled.
- Rows 5–6 likewise: 9.2 to the stall, 1.5 to the jitter. **Opposite responses
  to identical error magnitudes** — which is precisely what the old law cannot
  produce, however it is tuned.
- The last row is §3.2 above: the error is rising and the old law answers with
  λ = 26.3. The new one drops to 1.4 and raises the damping by four orders of
  magnitude.

The first row matters too, for a different reason: the scheduler *reproduces*
the old law at the start (51.5 against 52.7). It is not a different controller
bolted on; it agrees with the reference law where the reference law is right,
and differs where Chapter 4 showed it to be wrong.

---

## Part 8. Four things to fix in the file you sent

`tutorial-viewer14.cpp` is the **V1 baseline** — no robust weighting, and no ε
floor on the damped Hessian. Four notes before you build Chapter 5 on it.

**1. Build Chapter 5 on Variant 3, not on this.** Chapter 4's contribution is
the Hermite-informed weighting, so the gain layer must sit on top of it, with
**D** held fixed. Take `herpvs_v3_hermite.cpp` and change only the two gain
lines. If Chapter 5 were built on V1 it would silently abandon Chapter 4.

**2. The texture wiring is inverted.** Line 245 is `sim.init(Itextured, X)`
before `cdMo`, so the **desired** image comes from `peppers_o_b.jpg` — the
occluded one — and line 333 is `sim.init(Itexture, X)` before `cMo`, so the
**current** image is the clean `peppers.jpg`. That is backwards: the reference
should be clean and the servoing images occluded. Swap the two `sim.init` calls.

**3. These ten poses are far more aggressive than Chapter 4's**, and that is
good but must be stated. Chapter 4 ran from Z = 1.1 m to Z = 1.3 m — a 200 mm
retreat. The ten poses commented in this file all start near **Z = 3.2 m** with
rotations around 30°, so the displacement is roughly ten times larger. Arm A's
baseline must therefore be re-measured at these poses; Chapter 4's 0.999 mm and
0.385° do not transfer. This is exactly what §5.6.2 needs, and it means the
convergence-domain question of §5.7.3 has a real chance of separating the arms.

**4. `Ne` is captured at `iter == 2` and initialised to 0**, so at iterations 0
and 1 `log10(Ne)` is −∞ and both gains evaluate to **+∞**. Change it to
`iter == 0`. The scheduler handles `Ne <= 0` already, but Arm A must be defined
at every iteration too.

---

## Part 9. One thing the paper raises for Chapter 4

Your published paper reports, for the partial-occlusion scenario, a final error
of **0.0015 mm in 230 iterations** — with no robust weighting at all, since the
paper's HER-PVS is what Chapter 4 calls Variant 1.

Chapter 4 reports Variant 1 under occlusion settling at **1293 µm**, and builds
its whole case on that failure.

Both can be true — the paper's occluder, pose and texture differ from Chapter
4's — but **a jury member who has read the paper will ask**, and the thesis
should answer before being asked. The likely explanation is that the two
occlusion tests are not of comparable severity: the paper substitutes a black
region into the current image at a pose starting from Z = 3.2 m, while Chapter 4
uses an ellipse covering 5.80 % of the measurements at the desired pose from
Z = 1.1 m. Whichever it is, one sentence in §4.6.3 or §4.7.2 stating that the
occlusion configuration here is more severe than the published one, and in what
respect, closes the gap. If you can quantify the two coverages, better still.

Two smaller notes on the paper while it is open:

- §5.2.4 should attribute the adaptive law to the paper's reference **[8]**,
  which is where equations (24)–(25) come from, not to a different reference.
  And Chapter 4 should adopt the paper's own notation, **N_e** for the current
  and **N_ei** for the initial error — which removes the confusion entirely, and
  which Chapter 4 currently does not use.
- The paper says both are the "norm" of an image difference; the code uses
  `sumSquare()`, the *squared* norm. Harmless for the ratio ē but worth making
  consistent, since the exponents 0.8 and 5.2 were tuned against whichever the
  code does.

---

## Part 10. The order to do things in

1. **Swap the texture wiring** and move `Ne` to `iter == 0`. Ten minutes.
2. **Copy the two gain lines out of V3 and put `computeGains` in.** Build with
   `-g 0` and confirm it reproduces Chapter 4's Arm A exactly — same λ, same µ,
   same final pose. This is the check that the refactor changed nothing.
3. **Run `-g 1`** (floored, λ_min = 1.0) at one of the ten poses. **This is the
   experiment that decides the shape of your chapter.** If the floor alone
   recovers most of the accuracy, then §5.7 is about convergence domain and
   iteration count, not final accuracy. If the floor overshoots or oscillates,
   that is the strongest possible motivation for scheduling rather than
   clamping.
4. **Sweep λ_min** over {0.1, 0.5, 1, 2, 5, 10} — six runs — to calibrate
   λ_min and λ_max on measurement rather than on my defaults.
5. **Run `-g 2`** at the same pose, then the full ten-pose campaign across all
   three arms.
6. **Dump the control surfaces** with `fuzzy.dumpSurface("surface.dat")` for the
   §5.3.6 figures. That needs no simulator at all.
