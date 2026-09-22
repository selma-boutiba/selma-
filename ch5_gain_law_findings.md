# The gain law, resolved — and the Chapter 5 starting point

Reading `herpvs_v3_hermite.cpp` settled the outstanding question about λ and µ,
and the answer corrects both Chapter 4 and my own earlier claim about it. It
also hands Chapter 5 a much better §5.2.4 than the plan had.

---

## 1. What N_e actually is

The code is:

```cpp
if (iter == 2) { Ne = normeErrorI; }                          // line ~549
...
lambda = pow(10, (      log10(normeErrorI) - 0.8*log10(Ne)));
mu     = pow(10, (5.0 * log10(normeErrorI) - 5.2*log10(Ne)));
...
std::cout << "Ne  =  " << (5.23*log10(Ne)) << std::endl;      // <-- not Ne
```

Three things follow.

**N_e is not a count of measurements.** It is `errorI.sumSquare()` captured at
iteration 2 — the *initial intensity error* — used purely as a normalising
constant. Chapter 4 writes N_e in a way that reads as a number of pixels or
features, which is why 45.0219 looked like nothing.

**The printed "Ne" is not N_e.** It is 5.23·log₁₀(N_e). Inverting it:
10^(45.0219/5.23) = **4.0588×10⁸** — which is exactly the initial intensity
error of 4.06×10⁸ that §4.7.1 already quotes. Solving instead for N_e from each
logged (λ, µ) pair, since µ = λ⁵/N_e^1.2, gives 4.0588×10⁸ from all three
occluded runs and 4.0367×10⁸ from the nominal one, the small difference being
the occluder's own contribution to the initial error. (The 5.23 in the print
statement against the 5.2 in the formula is a separate small inconsistency.)

**Chapter 4's equations are therefore correct, and my earlier claim that they
were not was wrong.** I had substituted the printed 45.0219 for N_e and got
µ = 0.72 against the 3.24×10⁻⁹ logged. The arithmetic was right and the input
was not. Equations (4.5) and (4.6) stand as written, provided ‖**e**_I‖²
denotes the sum of squares, which it does.

**What Chapter 4 does need** is one sentence saying what N_e is. As it stands a
reader will try to reconcile it with *P* = 264 000 and fail, exactly as I did.

---

## 2. The law restated — and this is what §5.2.4 should say

Write ē = ‖**e**_I‖² ⁄ N_e for the normalised intensity error, which runs from 1
at the reference iteration down to about 8×10⁻⁴ at convergence. Then the
reference law is, exactly:

> **λ = N_e^0.2 · ē**  and  **µ = ē⁵ ⁄ N_e^0.2**, with N_e^0.2 = 52.69

Verified against the logs to four significant figures:

| | ē | λ computed | λ logged | µ computed | µ logged |
|---|---|---|---|---|---|
| iteration 2 | 1.0 | 52.68 | ~53 (§4.7.1) | 1.898×10⁻² | 1.9×10⁻² |
| V2 occluded, 600 | 4.162×10⁻² | 2.1924 | 2.19257 | 2.369×10⁻⁹ | 2.36971×10⁻⁹ |
| V3 nominal, 600 | 7.726×10⁻⁴ | 0.040701 | 0.0407037 | 5.223×10⁻¹⁸ | 5.25935×10⁻¹⁸ |

This is a far better basis for §5.2.4 than "the exponential heuristic law",
because the criticism now writes itself and needs no appeal to authority:

- **λ is linear in ē.** It vanishes at the same rate as the error, and since
  the commanded velocity is also proportional to the residual, ‖**V**_c‖ decays
  as the cube of the error. That is the stall.
- **µ is the fifth power of ē.** One decade of error costs five decades of
  damping. The trust region is gone within a handful of iterations, and the
  scheme runs as undamped Gauss–Newton for essentially its whole trajectory.
- **Neither has any scenario dependence.** The one scene-dependent quantity is
  N_e, the initial error, and it enters only as a fixed scale factor. The law
  cannot respond to what the run is actually doing. That is precisely the
  limitation §5.1 sets out to attack, and now it is a statement about the law
  rather than an assertion about heuristics.

Also: λ at iteration 2 equals N_e^0.2 identically, so §4.7.1's "λ ≃ 53 at the
first iteration" is not an empirical observation — it is N_e^0.2. Worth saying,
because it looks like a measurement and is not.

---

## 3. A problem at iterations 0 and 1

`Ne` is initialised to 0 and assigned at `iter == 2`. For iterations 0 and 1,
`log10(Ne)` is −∞, so λ and µ both evaluate to **+∞**.

The runs converge, so something makes this survive — the matrix inversion
presumably returns zeros against an infinite diagonal — but it should not be
left as it is. Two reasons:

1. **Arm A is the baseline of Chapter 5** and it must be defined at every
   iteration, including the first two.
2. §4.7.1 says λ falls "from λ ≃ 53 at the first iteration"; that is iteration
   *two*, and the first two are undefined.

**The fix is one character: capture `Ne` at `iter == 0` instead of `iter == 2`.**
Then λ = N_e^0.2 from the very first iteration, nothing is infinite, and
§4.7.1's sentence becomes literally true. Verify that it does not change the
converged results before adopting it — it should not, but check.

---

## 4. Corrections to Chapter 4

- **Define N_e.** One sentence in §4.2 or §4.7.1: the initial value of the
  intensity error, 4.06×10⁸, used as a normalising constant.
- **§4.7.1, "2.5 grey levels root-mean-square".** From λ = 0.0407037 and
  N_e = 4.06×10⁸, the final intensity error is ‖**e**_I‖² = 3.137×10⁵, so the
  RMS is √(3.137×10⁵/76 800) = **2.02** grey levels. The figure should be 2.02,
  and it belongs to Variant 3, which the console now confirms.
- **§4.7.2.** Delete my note claiming the printed gains do not satisfy the
  printed equations. They do. Replace it with the restatement of item 2 if you
  want it there, though §5.2.4 is the better home.
- **Optional but worth it:** give the restated form λ = N_e^0.2 ē in §4.7.1
  where the stall is diagnosed. "The gain is linear in the normalised error"
  is a one-line explanation of a result that currently takes a paragraph.

---

## 5. The scheduler

`fuzzy_gain_scheduler.h` — header-only, C++11, no ViSP dependency, so it
compiles and can be exercised without building the servo. It contains all
three arms in one function, so the non-gain logic is identical across them by
construction.

**Integration.** Replace these two lines in each of the three `herpvs_v*.cpp`:

```cpp
lambda = pow(10, (log10(normeErrorI) - 0.8*log10(Ne)));
mu     = pow(10, (5*log10(normeErrorI) - 5.2*log10(Ne)));
```

with

```cpp
computeGains(gainMode, normeErrorI, Ne, fuzzy, lambda, mu);
```

and add near the top of `main`:

```cpp
#include "fuzzy_gain_scheduler.h"
...
GainMode gainMode = GAIN_FIXED;          // or from argv: -g 0 | 1 | 2
FuzzyGainScheduler fuzzy;                // default Config; see calibration below
```

Everything downstream is untouched: the interaction matrix, **D**, the Hessian,
and the ε floor `muFloor = 1e-6*meanDiag` all stay exactly as they are. That
floor is Chapter 4's and is retained in all three arms, so the arms differ in
the scheduled gains and in nothing else.

**One point for §5.3.1.** The plan says the input is the "normalized
photometric residual", which is ambiguous: the code carries *two* residuals,
`normeError` (the Hermite feature error, 1.55×10¹⁰) and `normeErrorI` (the
intensity error, 4.06×10⁸). The reference law uses the **intensity** error, so
the scheduler does too — otherwise Arm A and Arm B would be responding to
different quantities and the comparison would be confounded. Say which in
§5.3.1.

**Behaviour at the operating points Chapter 4 measured.** Compiled and run:

| state | ē | Arm A λ | Arm B λ | Arm B µ |
|---|---|---|---|---|
| iteration 2 | 1.0 | 52.68 | 51.46 | 3.8×10⁻⁴ |
| descending, far | 1×10⁻¹ | 5.27 | 23.85 | 2.6×10⁻⁵ |
| descending, mid | 1×10⁻² | 0.53 | 7.94 | 2.6×10⁻⁵ |
| **stalled, mid** | 1×10⁻² | 0.53 | **51.46** | 2.6×10⁻⁵ |
| near solution, descending | 8×10⁻⁴ | 0.042 | 7.94 | 4.8×10⁻⁶ |
| **stalled near solution — Ch. 4's stall** | 7.7×10⁻⁴ | 0.041 | **9.24** | 4.3×10⁻⁶ |
| **error rising near solution — V3's jitter** | 7.7×10⁻⁴ | 0.041 | **1.48** | 3.7×10⁻⁵ |
| error rising, far | 5×10⁻¹ | 26.34 | 1.45 | 1.2×10⁻² |

Read the last three rows together, because they are the argument of the
chapter. All three have a small or moderate residual; the reference law gives
them **the same gain**, because it sees only ē. The scheduler gives 9.24 to the
stall, 1.48 to the jitter and 7.94 to healthy convergence — it separates two
situations that are identical in ē and require opposite actions. That is what
the second input buys, and it is why a scalar function of the residual cannot
do this job however well it is tuned.

**Design points worth having in §5.3, with reasons:**

- Both inputs and **both outputs** are on logarithmic universes. ē spans more
  than three decades and µ spans sixteen; on a linear partition everything
  after the first few iterations falls inside one membership function and the
  scheduler is inert over exactly the region where the stall happens. I had λ
  linear at first and its middle term came out at 28 — far too long a step near
  convergence — which is how I found this.
- The second input is the **relative** trend, (ē_k − ē_{k−1})/ē_{k−1}, not the
  raw difference. The raw difference tends to zero as ē does, whether the run
  is converging or stalled, so it is nearly uninformative late in the servo.
- λ_min > 0 and µ_min > 0 are what make the scheduler differ *in kind* from the
  reference law rather than in tuning: the gain cannot vanish and the damping
  cannot switch off.
- **Stability, for §5.4.2.** The damped Hessian **H**_D + µ diag(**H**_D) + ε**I**
  is positive definite by construction, so the step is a descent direction for
  *every* λ > 0 and µ ≥ 0 — λ scales the step without altering its direction.
  Instability can therefore arise only from a step long enough to leave the
  region where the linearisation holds, and that is what λ_max bounds. The
  bound is not available analytically and is set empirically. This is both true
  and sufficient, and it is a better section than the Ċ < 0 derivation the plan
  proposed, which is not available for a 264 000-feature scheme.

---

## 6. What to run, in this order

**First: Arm A′, at the Chapter 4 nominal pose, with Variant 3.** One evening.

```
-g 1   with lambdaFloor = 1.0, muFloor = 1e-6
```

Compare the final pose error against Arm A's 0.999 mm and 0.385°. This is the
experiment that decides the shape of your chapter:

- If flooring λ recovers most of the three orders of magnitude, then the floor
  owns the final-accuracy result and the fuzzy scheduler's case rests on
  iteration count and convergence domain. Frame §5.7 around those, and §5.7.3
  becomes the headline instead of §5.7.1.
- If flooring λ alone overshoots or oscillates, that is a result too, and a
  useful one: it says the gain must be *scheduled* rather than merely bounded,
  which is the strongest possible motivation for the chapter.

Either outcome is worth knowing before a word of Chapter 5 is written, and far
better learned from your own run than from a question at the defence.

**Then sweep λ_floor** over, say, {0.1, 0.5, 1, 2, 5, 10} — six runs, a couple
of hours — to find where the floor stops helping and starts overshooting. That
calibrates `lambdaMin` and `lambdaMax` for Arm B on measured grounds rather
than on my defaults, and it is a figure for §5.7.4.

**Then Arm B** at the same pose, and only then the ten-pose campaign.

**Then the control surfaces** for §5.3.6:

```cpp
FuzzyGainScheduler fz;
fz.dumpSurface("surface.dat", 41);   // needs no simulator at all
```

writes a gnuplot-ready grid of (ê, trend, λ, µ). Two `splot` calls give you
both figures.

**And one thing to fix while you are in the file:** capture `Ne` at `iter == 0`
per item 3.
