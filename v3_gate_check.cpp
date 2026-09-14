// =====================================================================
//  v3_gate_check.cpp  --  reading the first live V3 run, and what to
//  measure next.
//
//  Observed at iteration 1:
//      ### gate active  = YES
//      ### |e| raw      = 3.48354e+11
//      ### |e| normalized = 5.38436e+12      (x 15.5)
//      |Tc| = 0.887794                       (was 0.565845 as V2)
//
//  FIRST, WHAT DOES *NOT* MATTER.
//
//  The 15.5-fold inflation is harmless in itself. vpRobust::MEstimator
//  derives its own scale from the vector it is handed --
//  m_mad = 1.4826 * median(|r|), then C = 4.6851 * sigma -- so the weights
//  depend only on the RATIOS r_i / MAD. Multiplying every residual by a
//  constant changes nothing. Do not "fix" the inflation by rescaling
//  error_n; that would be a no-op dressed up as a correction.
//
//  What the gate changes, and all it can change, is the SHAPE of the
//  residual distribution: which pixels are extreme relative to the rest.
//
//  SECOND, WHAT DOES MATTER.
//
//  An inflation of 15.5x in energy means a large population of pixels was
//  amplified near the clamp limit. Since Gbar is floored at 0.1, the most
//  any single residual can grow is 10x, i.e. 100x in energy. If a fraction
//  f of the residual energy sat on fully clamped pixels and the rest were
//  untouched, then 1 + 99f = 15.5, so f = 0.15. The true figure is lower,
//  because pixels with 0.1 < Gbar < 1 are amplified too -- but either way a
//  substantial part of the image is being pushed to the floor.
//
//  That is the signature of a BRIGHTNESS measure on a photograph with large
//  dark regions, not of a structure measure. structG is built from
//  w_h/w_v/w_d = I filtered by Dnm2/Dnm3/Dnm4, and each Dnm is the sum over
//  n,m = 0..4 of Gauss-Hermite products, whose DC gain relative to L1 norm
//  is 1.000. So Dnm * I approximates a local mean of I, structG
//  approximates local brightness, and the dark half of this image falls
//  below 0.1 x mean and is amplified tenfold. The gate is currently
//  answering "how bright is it here", not "how much structure is here".
//
//  Net effect on the control law: dark-region residuals are judged extreme
//  and rejected, bright-region residuals are compressed and retained. For a
//  BLACK occluder that is accidentally the desired behaviour. For a
//  specularity it is exactly backwards. And in the nominal case, with no
//  outlier present, it is an arbitrary reshuffling of the weights -- which
//  is why V3 may well come out slightly worse than V2 here, in the same way
//  V2 came out slightly worse than V1.
// =====================================================================


// =====================================================================
//  DO THIS FIRST -- let the run finish.
//
//  You need the 600-iteration pose error for Table 4.1 row 3 whatever the
//  diagnosis, and it is five minutes. Send the final six errorpose values
//  and the last |e| / lambda / |Tc|.
// =====================================================================


// =====================================================================
//  THEN MEASURE THE CLAMP. Three numbers settle how much of the gate is
//  real and how much is the floor.
//
//  Add inside computeHermiteNormalizedResidual(), after Gmean is computed
//  and before the scaling loop.
// =====================================================================

	if (iter == 2) {
		// iter is a local of init(); if this does not compile here, hoist
		// the three counters to file scope and print them from the loop.
		unsigned int nClamped = 0;
		std::vector<double> gb(nbIn);
		for (unsigned int k = 0; k < nbIn; k++) {
			gb[k] = structG[k] / Gmean;
			if (gb[k] < 0.1) nClamped++;
		}
		std::sort(gb.begin(), gb.end());
		std::cout << "### clamped   = " << nClamped << " / " << nbIn
		          << "  (" << (100.0 * nClamped / nbIn) << "%)" << std::endl;
		std::cout << "### Gbar med  = " << gb[nbIn / 2] << std::endl;
		std::cout << "### Gbar p10  = " << gb[nbIn / 10]
		          << "   p90 = " << gb[9 * nbIn / 10] << std::endl;
	}

//  HOW TO READ IT
//
//    Gbar med close to 1      -> the mean is a fair normalizer.
//    Gbar med well below 1    -> structG is strongly skewed, the mean is
//                                pulled up by a bright minority, and half
//                                the image is being amplified. Use the
//                                MEDIAN of structG in place of Gmean; it is
//                                a one-word change and it recentres Gbar
//                                on 1 by construction.
//
//    clamped below ~5%        -> the floor is a safety net, as intended.
//    clamped above ~15%       -> the floor IS the mechanism. A sixth of the
//                                image is receiving one identical factor of
//                                10, which carries no structural
//                                information at all. Reporting that as
//                                structure-aware weighting would not
//                                survive a viva.
// =====================================================================


// =====================================================================
//  THE ACTUAL FIX -- first-order kernels, as in diag_v3_gate.cpp STEP 3.
//
//  D_1 is an odd function, so the kernels D_{1,0} and D_{0,1} have exactly
//  zero DC gain and their response is a true gradient magnitude, invariant
//  to any additive brightness offset. Dropping the (0,0) term from the
//  existing sum does NOT achieve this -- measured, DC/L1 only falls from
//  1.000 to 0.810, because the even orders carry DC as well.
//
//  The kernels leave Dnm1..Dnm4 untouched, so the feature vector and the
//  interaction matrix are unchanged and V1 and V2 need not be re-run. Only
//  structG changes, and only in V3.
//
//  Expect the clamp fraction to CHANGE CHARACTER rather than simply fall:
//  with a genuine gradient measure, flat regions legitimately have low
//  structure and should be amplified -- that is the stated intent of the
//  method. The difference is that they will be the geometrically flat
//  regions rather than the dark ones, which is the claim Chapter 4 actually
//  makes. Verify this by displaying structG as an image: it should trace
//  edges, not reproduce the photograph.
//
//  Sanity check the kernels before trusting them: sum(G10s2) must be of
//  order 1e-17, not of order 1.
// =====================================================================


// =====================================================================
//  ORDER OF WORK
//
//   1. Let the current run finish. Record row 3. It is an honest result for
//      the gate as specified in the chapter, and it is the baseline the fix
//      has to beat.
//   2. Add the clamp diagnostic. Three numbers.
//   3. Switch structG to the first-order kernels. Re-run. Compare against
//      step 1.
//   4. Only then write Section 4.6's claim about structure-awareness, and
//      write it to match whichever version you keep.
//
//  Step 1 is publishable either way: "the gate as first formulated responds
//  to intensity rather than to structure, which the following experiment
//  demonstrates, and the corrected formulation is derived in Section X" is
//  a stronger thesis narrative than presenting only the version that works.
// =====================================================================
