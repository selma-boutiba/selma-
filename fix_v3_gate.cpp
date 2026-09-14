// =====================================================================
//  fix_v3_gate.cpp  --  repairing the V3 structure gate
//
//  WHAT THE DIAGNOSTIC ESTABLISHED
//
//      ### dim(error)  = 264000
//      ### nbPixels    = 76800
//      ### gate active = NO -- V3 IS RUNNING AS V2
//
//  264000 = 4 x 66000, and 66000 = 220 x 300 = (240 - 2*bord) x (320 - 2*bord)
//  with bord = 10. So the feature is the FOUR Hermite scale responses over
//  the interior region only -- the ten-pixel border the filters cannot reach
//  is excluded, exactly as ViSP's own vpFeatureLuminance excludes its border.
//
//  The old guard compared against 76800, the FULL image. It could never
//  match, so it returned on every iteration of every run and V3 has always
//  been V2. Note that the generalized version I sent earlier would also have
//  skipped: it tested error.getRows() % 76800 == 0, and 264000 is not a
//  multiple of 76800. Both counts were wrong; 66000 is the right one.
//
//  Two things must now be fixed, in this order.
// =====================================================================


// =====================================================================
//  STEP A -- determine the channel ordering. DO NOT SKIP THIS.
//
//  The gate is one scale factor per PIXEL, but the residual has four
//  components per pixel, so we must know how buildFrom() lays them out:
//
//      channel-blocked : p = c * 66000 + k     (four separate loops)
//      interleaved     : p = k * 4 + c         (one loop, s[l++] four times)
//
//  Guessing wrong scrambles the gate silently -- every pixel would be scaled
//  by the factor belonging to a different pixel, which looks like noise
//  rather than like a bug. The test below decides it from the data.
//
//  The four scales differ enormously in smoothing (sigma1 = 0.5 is almost no
//  smoothing, sigma4 = 2.8 is heavy), so the four channels must have clearly
//  different residual variances. Group the residual both ways and print the
//  four variances of each grouping: under the TRUE layout the four numbers
//  differ by a large factor; under the false one they come out nearly equal,
//  because each group is then a uniform sample of all four channels.
//
//  Paste this next to the ### block, inside the same if (iter == 2).
// =====================================================================

		if (iter == 2) {
			const unsigned int nbIn = (I.getHeight() - 2 * bord) * (I.getWidth() - 2 * bord); // 66000
			const unsigned int nbCh = error.getRows() / nbIn;                                 // 4

			std::cout << "### nbIn = " << nbIn << "  nbCh = " << nbCh
			          << "  remainder = " << (error.getRows() % nbIn) << std::endl;

			for (unsigned int c = 0; c < nbCh; c++) {
				double sBlk = 0.0, sInt = 0.0;
				for (unsigned int k = 0; k < nbIn; k++) {
					double eb = error[c * nbIn + k];   // channel-blocked hypothesis
					double ei = error[k * nbCh + c];   // interleaved hypothesis
					sBlk += eb * eb;
					sInt += ei * ei;
				}
				std::cout << "### chan " << c
				          << "   blocked RMS = " << std::sqrt(sBlk / nbIn)
				          << "   interleaved RMS = " << std::sqrt(sInt / nbIn) << std::endl;
			}
		}

// =====================================================================
//  READING STEP A
//
//  Whichever COLUMN shows four clearly different numbers is the true layout.
//  Example of what channel-blocked looks like:
//
//      ### chan 0   blocked RMS = 2140    interleaved RMS = 1149
//      ### chan 1   blocked RMS = 1205    interleaved RMS = 1148
//      ### chan 2   blocked RMS =  702    interleaved RMS = 1149
//      ### chan 3   blocked RMS =  388    interleaved RMS = 1148
//
//  -- blocked varies by 5x, interleaved is flat, so the layout is blocked.
//  A monotone trend with scale (largest for sigma1, smallest for sigma4) is
//  the expected signature, since heavier smoothing suppresses the residual.
//
//  If BOTH columns come out flat, stop and read your customized
//  vpFeatureLuminance::buildFrom() directly -- the layout is something else
//  and none of the code below applies.
// =====================================================================


// =====================================================================
//  STEP B -- the corrected gate.
//
//  Replaces computeHermiteNormalizedResidual() entirely. Uncomment the ONE
//  index line that STEP A selected and delete the other.
//
//  Note the index arithmetic for structG: feature element k of a channel
//  corresponds to image pixel (bord + k / wIn, bord + k % wIn), so the gate
//  is built over the same interior region the feature uses. Building it over
//  the full 320 x 240 and indexing with k would misalign every row by 20
//  pixels and drift progressively down the image -- the single easiest way
//  to get a plausible-looking wrong answer here.
// =====================================================================

void computeHermiteNormalizedResidual(const vpColVector &error, vpColVector &structG, vpColVector &error_n)
{
	const unsigned int hIn = I.getHeight() - 2 * bord;   // 220
	const unsigned int wIn = I.getWidth()  - 2 * bord;   // 300
	const unsigned int nbIn = hIn * wIn;                 // 66000

	error_n = error;

	if (nbIn == 0 || error.getRows() % nbIn != 0) {
		// Say so loudly rather than failing silently as the old version did.
		std::cout << "### WARNING: gate skipped, dim(error) = " << error.getRows()
		          << " is not a multiple of " << nbIn << std::endl;
		structG.resize(0);
		return;
	}

	const unsigned int nbCh = error.getRows() / nbIn;    // 4

	// Per-pixel structure energy over the interior region, same indexing as
	// the feature vector.
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

	for (unsigned int k = 0; k < nbIn; k++) {
		double Gbar = structG[k] / Gmean;   // ~1 on average
		if (Gbar < 0.1) Gbar = 0.1;         // cap amplification in flat regions
		for (unsigned int c = 0; c < nbCh; c++) {
			unsigned int p = c * nbIn + k;      // CHANNEL-BLOCKED
			// unsigned int p = k * nbCh + c;   // INTERLEAVED
			error_n[p] = error[p] / Gbar;
		}
	}
}

// =====================================================================
//  HOW TO CONFIRM THE GATE IS NOW LIVE
//
//  Keep the ### block. On a repaired run it must print
//
//      ### gate active = YES
//
//  and, more usefully, V3's console must now DIFFER from V2's. If the two
//  are still digit-for-digit identical after this fix, the difference really
//  does cancel in the MAD and that is a genuine (and reportable) finding
//  rather than a bug. Only at that point is the sentence about the
//  normalization being inert in the nominal case defensible.
//
//  Change the guard condition in the ### block to compare against nbIn, not
//  I.getHeight()*I.getWidth(), or it will keep reporting NO.
// =====================================================================


// =====================================================================
//  STILL OPEN AFTER THIS FIX -- structG measures brightness, not structure.
//
//  w_h, w_v, w_d are I filtered by Dnm2, Dnm3, Dnm4, and each Dnm is the SUM
//  over n,m = 0..4 of separable Gauss-Hermite products. That sum is low-pass
//  dominated: DC gain relative to L1 norm is 1.000, so Dnm * I approximates a
//  local mean of I and structG approximates local BRIGHTNESS. Dropping the
//  (0,0) term does not fix it -- measured, the ratio only falls to 0.810,
//  because the even orders also carry DC.
//
//  Against a BLACK occluder this accidentally does the right thing: low
//  brightness -> small Gbar -> clamped to 0.1 -> residual amplified tenfold
//  -> Tukey rejects it. A BRIGHT outlier such as a specularity would be
//  PROTECTED instead. Do not make the brightness-independent claim in 4.6
//  until the gate uses the first-order kernels in diag_v3_gate.cpp STEP 3,
//  which are odd and therefore exactly DC-free.
//
//  Order of work: STEP A, then STEP B, then re-run V3 and see whether it
//  differs from V2 at all. Only then decide whether the derivative kernels
//  are needed for the thesis or belong in future work.
// =====================================================================
