// =====================================================================
//  v3_gate_final.cpp  --  the finished V3 gate. Ordering resolved.
//
//  STEP A returned:
//      ### nbIn = 66000  nbCh = 4  remainder = 0
//      ### chan 0   blocked RMS = 1115.35   interleaved RMS = 1144.52
//      ### chan 1   blocked RMS = 1445.22   interleaved RMS = 1147.27
//      ### chan 2   blocked RMS = 1315.24   interleaved RMS = 1150.04
//      ### chan 3   blocked RMS =  464.302  interleaved RMS = 1152.97
//
//  The interleaved column is flat at 1148.7, which is the global RMS
//  sqrt(3.48354e11 / 264000) -- the signature of a uniform sample of all
//  four channels. The blocked column varies 3.1-fold and its own RMS is
//  also 1148.7, so those four groups partition the residual energy.
//
//  Layout is CHANNEL-BLOCKED:  p = c * 66000 + k.
//
//  TWO EDITS, both in tutorial-viewer13.cpp (V3) only.
// =====================================================================


// =====================================================================
//  EDIT 1 -- replace the whole body of computeHermiteNormalizedResidual()
//  with this. Nothing to choose; the ordering is settled.
// =====================================================================

void computeHermiteNormalizedResidual(const vpColVector &error, vpColVector &structG, vpColVector &error_n)
{
	const unsigned int hIn  = I.getHeight() - 2 * bord;   // 220
	const unsigned int wIn  = I.getWidth()  - 2 * bord;   // 300
	const unsigned int nbIn = hIn * wIn;                  // 66000

	error_n = error;

	if (nbIn == 0 || error.getRows() % nbIn != 0) {
		std::cout << "### WARNING: gate skipped, dim(error) = " << error.getRows()
		          << " is not a multiple of " << nbIn << std::endl;
		structG.resize(0);
		return;
	}

	const unsigned int nbCh = error.getRows() / nbIn;     // 4

	// Per-pixel structure energy over the SAME interior region the feature
	// uses: feature element k of a channel is image pixel
	// (bord + k / wIn, bord + k % wIn).
	structG.resize(nbIn);
	double sumG = 0.0;
	for (unsigned int k = 0; k < nbIn; k++) {
		const unsigned int i = bord + k / wIn;
		const unsigned int j = bord + k % wIn;
		double gh = w_h[i][j], gv = w_v[i][j], gd = w_d[i][j];
		structG[k] = std::sqrt(gh * gh + gv * gv + gd * gd);
		sumG += structG[k];
	}

	double Gmean = sumG / nbIn;
	if (Gmean < 1e-12) Gmean = 1e-12;

	// One scale factor per pixel, applied to all four stacked channels.
	for (unsigned int k = 0; k < nbIn; k++) {
		double Gbar = structG[k] / Gmean;   // ~1 on average
		if (Gbar < 0.1) Gbar = 0.1;         // cap amplification in flat regions
		for (unsigned int c = 0; c < nbCh; c++) {
			error_n[c * nbIn + k] = error[c * nbIn + k] / Gbar;
		}
	}
}


// =====================================================================
//  EDIT 2 -- the ### block still compares against 76800, so it will keep
//  reporting NO even after the gate works. Replace the whole diagnostic
//  block (both the original ### lines and the STEP A ordering test, which
//  has served its purpose) with this shorter one.
// =====================================================================

		if (iter == 2) {
			const unsigned int nbIn = (I.getHeight() - 2 * bord) * (I.getWidth() - 2 * bord);
			std::cout << "### dim(error)  = " << error.getRows()
			          << "   nbIn = " << nbIn
			          << "   nbCh = " << (error.getRows() / nbIn) << std::endl;
			std::cout << "### gate active = "
			          << ((structG.getRows() == nbIn) ? "YES" : "NO") << std::endl;
			std::cout << "### |e| raw = " << error.sumSquare()
			          << "   |e| normalized = " << error_n.sumSquare() << std::endl;
		}

//  Place this AFTER the computeHermiteNormalizedResidual(...) call, not
//  before -- it inspects structG and error_n, which that call fills.
//
//  On a repaired run it must print  ### gate active = YES  and the two |e|
//  values must DIFFER. If they are equal the gate is still a no-op.
// =====================================================================


// =====================================================================
//  WHAT TO EXPECT, AND WHAT EACH OUTCOME MEANS
//
//  (a) V3's console now differs from V2's.
//      The contribution is live for the first time. Re-run V3 nominal,
//      refill Table 4.1 row 3, regenerate Figure 4.4, then do the occluded
//      runs.
//
//  (b) V3 still matches V2 digit for digit.
//      Then the normalization genuinely cancels in the MAD -- vpRobust
//      computes its scale as 1.4826 * median(|residual|), and dividing every
//      residual by a factor whose mean is 1 can leave the ratio
//      residual/MAD close to unchanged. That is a real result and worth a
//      paragraph: it would mean a per-pixel rescaling of the residual cannot
//      by itself change which measurements a redescending estimator selects,
//      and the gate would have to act on the WEIGHTS rather than on the
//      residual. Check it before concluding: print the MAD, or equivalently
//      median(|error|) and median(|error_n|), in the same block.
//
//  Either way the gate is no longer silently dead, which is the point.
// =====================================================================
