// ===========================================================================
//  fuzzy_gain_scheduler.h
//
//  Fuzzy gain scheduling for Hermite-based photometric visual servoing.
//  Chapter 5 of the thesis. Header-only, C++11, no dependencies beyond the
//  standard library -- in particular it does NOT depend on ViSP, so it can be
//  unit-tested and its control surfaces dumped without building the servo.
//
//  ---------------------------------------------------------------------------
//  WHAT IT REPLACES
//
//  The reference law in herpvs_v*.cpp is
//      lambda = pow(10, (      log10(normeErrorI) - 0.8*log10(Ne)));
//      mu     = pow(10, (5.0 * log10(normeErrorI) - 5.2*log10(Ne)));
//  with Ne captured as normeErrorI at iteration 2. Writing
//      ebar = normeErrorI / Ne
//  for the normalised intensity error, that law is exactly
//      lambda = Ne^0.2 * ebar          (Ne^0.2 = 52.69 for Ne = 4.059e8)
//      mu     = ebar^5 / Ne^0.2
//  so the gain is LINEAR in the normalised error and the damping is its FIFTH
//  POWER. Both vanish as the servo converges, which is the defect Chapter 4
//  measured: the commanded velocity decays as the cube of the error and the
//  run stalls a millimetre short, while the damping is gone -- 5.2e-18 -- long
//  before that.
//
//  ---------------------------------------------------------------------------
//  THE THREE ARMS
//
//  GAIN_FIXED    the reference law above, unchanged. Arm A.
//  GAIN_FLOORED  the same law with lambda and mu clamped from below. Arm A'.
//                This exists because a one-line clamp also stops the gain
//                vanishing, and unless the fuzzy scheduler is compared
//                against it, its contribution cannot be separated from the
//                clamp's. RUN THIS ARM BEFORE WRITING THE CHAPTER.
//  GAIN_FUZZY    the scheduler below. Arm B.
//
//  ---------------------------------------------------------------------------
//  DESIGN NOTES, for section 5.3 of the thesis
//
//  Input 1, the normalised error, is taken on a LOGARITHMIC universe. ebar
//  falls from 1 to about 8e-4 over a nominal run -- more than three decades --
//  so a linear partition of [0,1] would place everything after the first few
//  iterations inside a single membership function and the scheduler would be
//  inert over exactly the region where the stall occurs.
//
//  Input 2 is the RELATIVE trend, (ebar_k - ebar_{k-1}) / ebar_{k-1}, and not
//  the raw difference. The raw difference tends to zero as ebar does, whether
//  the run is converging or stalled, so it carries little information late in
//  the servo; the relative rate is scale-free and stays informative. Its SIGN
//  is what identifies the two failure modes Chapter 4 found, which the rule
//  base is built around -- see the table in inference().
//
//  Both outputs are bounded. lambda is mapped linearly onto
//  [lambdaMin, lambdaMax]; mu is mapped LOGARITHMICALLY onto [muMin, muMax]
//  because it spans decades. lambdaMin > 0 and muMin > 0 are what make the
//  scheduler's behaviour differ in kind from the reference law: the gain
//  cannot vanish and the damping cannot switch off.
//
//  Stability: the damped Hessian H_D + mu diag(H_D) + eps I is positive
//  definite by construction, so the step is a descent direction for EVERY
//  lambda > 0 and mu >= 0; lambda scales the step without altering its
//  direction. Instability can therefore only come from a step long enough to
//  leave the region in which the linearisation holds, which is what
//  lambdaMax bounds. That bound is not available analytically and is set
//  empirically -- see calibrate() below.
// ===========================================================================

#ifndef FUZZY_GAIN_SCHEDULER_H
#define FUZZY_GAIN_SCHEDULER_H

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <vector>

// ---------------------------------------------------------------------------
enum GainMode {
  GAIN_FIXED   = 0,   // Arm A : the reference law
  GAIN_FLOORED = 1,   // Arm A': the reference law, clamped from below
  GAIN_FUZZY   = 2    // Arm B : the fuzzy scheduler
};

// ---------------------------------------------------------------------------
//  Trapezoidal membership function. A triangle is b == c; a left shoulder is
//  a == b == -INF_; a right shoulder is c == d == +INF_.
// ---------------------------------------------------------------------------
struct TrapMF {
  double a, b, c, d;

  TrapMF() : a(0), b(0), c(0), d(0) {}
  TrapMF(double a_, double b_, double c_, double d_) : a(a_), b(b_), c(c_), d(d_) {}

  double grade(double x) const {
    if (x <= a || x >= d) return 0.0;
    if (x >= b && x <= c) return 1.0;
    if (x < b) return (b > a) ? (x - a) / (b - a) : 1.0;
    return (d > c) ? (d - x) / (d - c) : 1.0;
  }
};

// ---------------------------------------------------------------------------
class FuzzyGainScheduler {
public:
  struct Config {
    double lambdaMin;    // gain floor  -- the quantity Arm A' calibrates
    double lambdaMax;    // gain ceiling -- bounds the step length
    double muMin;        // damping floor: keeps a trust region at all times
    double muMax;        // damping ceiling
    double decades;      // log10 span of the ebar universe
    double trendScale;   // relative change mapped to full membership
    int    samples;      // defuzzification grid

    Config()
      : lambdaMin(1.0),      // the occluded runs converged to micrometres at
                             // lambda = 2.19, and stalled at 0.041; a floor of
                             // 1.0 sits inside the range that worked. CALIBRATE.
        lambdaMax(100.0),    // on the log map this puts the High term near 50,
                             // matching the reference law's peak of
                             // Ne^0.2 = 52.7 at the reference iteration
        muMin(1e-6),
        muMax(5e-2),         // the reference law starts at mu = 1.9e-2
        decades(4.0),        // ebar spans 1 .. 1e-4
        trendScale(0.05),    // 5 % change per iteration is "fast"
        samples(101)
    {}
  };

  explicit FuzzyGainScheduler(const Config &cfg = Config())
    : m_cfg(cfg), m_prevEbar(-1.0), m_lambda(cfg.lambdaMax), m_mu(cfg.muMax),
      m_ehat(1.0), m_that(0.0)
  {
    buildSets();
  }

  void reset() { m_prevEbar = -1.0; }

  // -------------------------------------------------------------------------
  //  Call once per servo iteration, before the control law.
  //    normeErrorI : errorI.sumSquare() for the current image
  //    Ne          : the same quantity at the reference iteration
  //  Both arms use the same ebar, so the comparison is not confounded by a
  //  difference in how the error is normalised.
  // -------------------------------------------------------------------------
  void update(double normeErrorI, double Ne) {
    const double ebar = (Ne > 0.0) ? normeErrorI / Ne : 1.0;

    // Input 1: logarithmic, 1 at the reference iteration, 0 when converged.
    const double lo = -m_cfg.decades;
    double l10 = (ebar > 0.0) ? std::log10(ebar) : lo;
    m_ehat = clamp((l10 - lo) / m_cfg.decades, 0.0, 1.0);

    // Input 2: relative trend. Zero on the first call, when there is no
    // previous sample -- which lands in the (Large, Zero) cell and asks for a
    // large gain, the correct opening move.
    double rel = 0.0;
    if (m_prevEbar > 0.0) rel = (ebar - m_prevEbar) / m_prevEbar;
    m_that = clamp(rel / m_cfg.trendScale, -1.0, 1.0);
    m_prevEbar = ebar;

    inference(m_ehat, m_that, m_lambda, m_mu);
  }

  double lambda() const { return m_lambda; }
  double mu()     const { return m_mu; }

  // Diagnostics for the per-iteration log and for section 5.7.
  double ehat() const { return m_ehat; }
  double trend() const { return m_that; }

  const Config &config() const { return m_cfg; }

  // -------------------------------------------------------------------------
  //  Control surfaces for the figures of section 5.3.6. Writes a grid of
  //  (ehat, trend, lambda, mu) to a text file. Offline: needs no simulator.
  // -------------------------------------------------------------------------
  void dumpSurface(const char *path, int n = 41) const {
    FILE *f = std::fopen(path, "w");
    if (!f) return;
    std::fprintf(f, "# ehat trend lambda mu\n");
    for (int i = 0; i < n; ++i) {
      const double e = double(i) / double(n - 1);
      for (int j = 0; j < n; ++j) {
        const double t = -1.0 + 2.0 * double(j) / double(n - 1);
        double lam, m;
        inference(e, t, lam, m);
        std::fprintf(f, "%.6f %.6f %.8g %.8g\n", e, t, lam, m);
      }
      std::fprintf(f, "\n");
    }
    std::fclose(f);
  }

private:
  // ---- linguistic terms ---------------------------------------------------
  //  ehat : Small / Medium / Large normalised error
  //  trend: Negative (improving) / Zero (stalled) / Positive (worsening)
  TrapMF m_eS, m_eM, m_eL;
  TrapMF m_tN, m_tZ, m_tP;
  //  lambda: Low / Medium / High           (normalised, mapped linearly)
  //  mu    : VeryLow / Low / Medium / High (normalised, mapped in log)
  TrapMF m_lLow, m_lMed, m_lHigh;
  TrapMF m_mVLow, m_mLow, m_mMed, m_mHigh;

  void buildSets() {
    const double I = 1e30;
    m_eS = TrapMF(-I, -I, 0.15, 0.45);
    m_eM = TrapMF(0.20, 0.40, 0.60, 0.80);
    m_eL = TrapMF(0.55, 0.85, I, I);

    m_tN = TrapMF(-I, -I, -0.35, -0.05);
    m_tZ = TrapMF(-0.20, -0.05, 0.05, 0.20);
    m_tP = TrapMF(0.05, 0.35, I, I);

    m_lLow  = TrapMF(-I, -I, 0.00, 0.25);
    m_lMed  = TrapMF(0.15, 0.35, 0.55, 0.75);
    m_lHigh = TrapMF(0.60, 0.85, I, I);

    m_mVLow = TrapMF(-I, -I, 0.00, 0.20);
    m_mLow  = TrapMF(0.10, 0.25, 0.35, 0.50);
    m_mMed  = TrapMF(0.35, 0.50, 0.60, 0.75);
    m_mHigh = TrapMF(0.65, 0.85, I, I);
  }

  // -------------------------------------------------------------------------
  //  THE RULE BASE -- section 5.3.4. Nine rules, each justified.
  //
  //   ehat \ trend |  Negative        |  Zero            |  Positive
  //   -------------+------------------+------------------+------------------
  //   Large        |  lambda High     |  lambda High     |  lambda Low
  //                |  mu     Low      |  mu     Medium   |  mu     High
  //   Medium       |  lambda Medium   |  lambda High     |  lambda Low
  //                |  mu     Low      |  mu     Low      |  mu     Medium
  //   Small        |  lambda Medium   |  lambda Medium   |  lambda Low
  //                |  mu     VeryLow  |  mu     VeryLow  |  mu     Low
  //
  //  Large/Negative   far away and descending: push, and trust the model.
  //  Large/Zero       far away and not descending: the step is too short, or
  //                   the model is poor; raise the gain and add damping as
  //                   insurance.
  //  Large/Positive   far away and worsening: the step overshoots or the
  //                   linearisation has failed; shorten it and damp hard.
  //  Medium/Negative  mid-course and descending: moderate gain, little damping.
  //  Medium/Zero      *** THE STALL CHAPTER 4 MEASURED. *** Error still
  //                   appreciable, no progress, model sound: raise the gain
  //                   and keep the damping low. The reference law does the
  //                   opposite here, because lambda is linear in ebar.
  //  Medium/Positive  mid-course and worsening: shorten, damp.
  //  Small/Negative   near the solution and descending: keep a moderate gain
  //                   -- NOT a vanishing one, which is Chapter 4's lesson --
  //                   and take the damping out of the way.
  //  Small/Zero       near the solution, no progress: converged, or held up.
  //                   Moderate gain; not high, or it will chatter.
  //  Small/Positive   *** THE TERMINAL JITTER OF VARIANT 3. *** Near the
  //                   solution and the error is rising: the weights are still
  //                   moving the minimum under the controller. Shorten the
  //                   step.
  //
  //  Small/Zero and Medium/Zero are both "no progress" and they call for
  //  opposite actions. Telling them apart is what the second input is for,
  //  and it is why a scalar function of the residual cannot do this job.
  // -------------------------------------------------------------------------
  void inference(double ehat, double that, double &lambdaOut, double &muOut) const {
    const double eS = m_eS.grade(ehat), eM = m_eM.grade(ehat), eL = m_eL.grade(ehat);
    const double tN = m_tN.grade(that), tZ = m_tZ.grade(that), tP = m_tP.grade(that);

    // Firing strengths, Mamdani conjunction by minimum.
    const double r[9] = {
      std::min(eL, tN), std::min(eL, tZ), std::min(eL, tP),
      std::min(eM, tN), std::min(eM, tZ), std::min(eM, tP),
      std::min(eS, tN), std::min(eS, tZ), std::min(eS, tP)
    };

    const TrapMF *lam[9] = { &m_lHigh, &m_lHigh, &m_lLow,
                             &m_lMed,  &m_lHigh, &m_lLow,
                             &m_lMed,  &m_lMed,  &m_lLow };
    const TrapMF *dmp[9] = { &m_mLow,  &m_mMed,  &m_mHigh,
                             &m_mLow,  &m_mLow,  &m_mMed,
                             &m_mVLow, &m_mVLow, &m_mLow };

    const double nl = defuzzify(r, lam);
    const double nm = defuzzify(r, dmp);

    // BOTH outputs are mapped logarithmically. The reference law drives lambda
    // over [0.04, 53] and mu over [1.9e-2, 5e-18]; on a linear map the middle
    // linguistic term of lambda would land near 28, which is far too
    // aggressive a step near convergence. On the log map the three terms fall
    // at roughly 50, 8 and 1.6 for the default bounds, which spans the range
    // the reference law actually visits.
    lambdaOut = m_cfg.lambdaMin * std::pow(m_cfg.lambdaMax / m_cfg.lambdaMin, nl);
    muOut     = m_cfg.muMin     * std::pow(m_cfg.muMax     / m_cfg.muMin,     nm);
  }

  // Aggregate by maximum of the clipped consequents, then centre of gravity.
  double defuzzify(const double r[9], const TrapMF *const sets[9]) const {
    double num = 0.0, den = 0.0;
    for (int k = 0; k < m_cfg.samples; ++k) {
      const double y = double(k) / double(m_cfg.samples - 1);
      double agg = 0.0;
      for (int i = 0; i < 9; ++i) {
        if (r[i] <= 0.0) continue;
        agg = std::max(agg, std::min(r[i], sets[i]->grade(y)));
      }
      num += y * agg;
      den += agg;
    }
    return (den > 0.0) ? num / den : 0.5;   // no rule fired: neutral output
  }

  static double clamp(double x, double lo, double hi) {
    return (x < lo) ? lo : ((x > hi) ? hi : x);
  }

  Config m_cfg;
  double m_prevEbar;
  double m_lambda, m_mu;
  double m_ehat, m_that;
};

// ---------------------------------------------------------------------------
//  One place where all three arms are computed, so that the non-gain logic is
//  identical across them by construction rather than by care.
//
//  Call it where the two reference lines currently sit:
//
//      computeGains(gainMode, normeErrorI, Ne, fuzzy, lambda, mu);
//
//  and delete
//      lambda = pow(10, (log10(normeErrorI) - 0.8*log10(Ne)));
//      mu     = pow(10, (5*log10(normeErrorI) - 5.2*log10(Ne)));
// ---------------------------------------------------------------------------
inline void computeGains(GainMode mode,
                         double normeErrorI, double Ne,
                         FuzzyGainScheduler &fuzzy,
                         double &lambda, double &mu,
                         double lambdaFloor = 1.0,
                         double muFloor     = 1e-6) {
  if (Ne <= 0.0) {            // reference not yet captured: see the note in
    lambda = 1.0;             // ch5_gain_law_findings.md on iterations 0 and 1,
    mu     = 1e-2;            // where the reference law evaluates to infinity
    return;
  }

  const double ebar = normeErrorI / Ne;
  const double s    = std::pow(Ne, 0.2);      // 52.69 for Ne = 4.059e8

  switch (mode) {
    case GAIN_FIXED:                          // Arm A
      lambda = s * ebar;
      mu     = std::pow(ebar, 5.0) / s;
      break;

    case GAIN_FLOORED:                        // Arm A'
      lambda = std::max(s * ebar, lambdaFloor);
      mu     = std::max(std::pow(ebar, 5.0) / s, muFloor);
      break;

    case GAIN_FUZZY:                          // Arm B
      fuzzy.update(normeErrorI, Ne);
      lambda = fuzzy.lambda();
      mu     = fuzzy.mu();
      break;
  }
}

#endif  // FUZZY_GAIN_SCHEDULER_H
