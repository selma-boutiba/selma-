// =====================================================================
//  diag_v3_gate.cpp  --  one-line diagnostic for V3
//
//  WHY THIS MATTERS
//  computeHermiteNormalizedResidual() begins with
//
//      error_n = error;
//      if (error.getRows() != nbPixels) {
//          structG.resize(0);
//          return;                 // <-- silent no-op
//      }
//
//  so if the customized vpFeatureLuminance stacks the four Hermite scales
//  into the feature vector, error has 4 x 76800 = 307200 rows, the guard
//  fires, error_n is a plain copy of error, and V3 hands the RAW residual to
//  MEstimator() -- i.e. V3 executes exactly the same arithmetic as V2.
//  That is the simplest explanation of the two bit-identical consoles, and
//  it would mean the Hermite gate has never been exercised in any run.
//
//  STEP 1 -- run the test (V3 only, tutorial-viewer13.cpp)
//  Paste the block below immediately AFTER the line
//
//      sI.error(sId, error);
//
//  inside the do{} loop, rebuild, run, and read the first line of output.
// =====================================================================

		if (iter == 2) {
			std::cout << "### dim(error)  = " << error.getRows() << std::endl;
			std::cout << "### nbPixels    = " << (I.getHeight() * I.getWidth()) << std::endl;
			std::cout << "### dim(Lsd)    = " << Lsd.getRows() << " x " << Lsd.getCols() << std::endl;
			std::cout << "### gate active = "
			          << ((error.getRows() == I.getHeight() * I.getWidth()) ? "YES" : "NO -- V3 IS RUNNING AS V2")
			          << std::endl;
		}

// =====================================================================
//  HOW TO READ IT
//
//    dim(error) = 76800   -> gate active. V3 differs from V2, and the
//                            identical consoles must be explained by (2)
//                            a build/run mixup or (3) MAD cancellation.
//
//    dim(error) = 307200  -> gate NEVER fires. Every V3 result reported so
//    (or any other value)    far is a V2 result. Table 4.1 row 3 and
//                            Figure 4.4 are duplicates of row 2 / Fig 4.3,
//                            and section 4.7.1's "the structural information
//                            is inert" sentence has no support. Apply STEP 2.
// =====================================================================


// =====================================================================
//  STEP 2 -- only if dim(error) is a multiple of nbPixels other than 1.
//
//  Replace the whole body of computeHermiteNormalizedResidual() with this.
//  It applies one per-pixel gate to every stacked channel.
//
//  YOU MUST PICK THE RIGHT INDEX LINE. Open your customized
//  vpFeatureLuminance::buildFrom() and look at the loop nesting that fills
//  the feature vector s:
//
//    for (scale c) for (pixel k)  s[cnt++] = ...   -> CHANNEL-BLOCKED
//    for (pixel k) for (scale c)  s[cnt++] = ...   -> INTERLEAVED
//
//  Uncomment the matching line and delete the other. Getting this wrong
//  silently scrambles the gate, so check it rather than guessing.
// =====================================================================

void computeHermiteNormalizedResidual(const vpColVector &error, vpColVector &structG, vpColVector &error_n)
{
	const unsigned int height   = I.getHeight();
	const unsigned int width    = I.getWidth();
	const unsigned int nbPixels = height * width;

	error_n = error;

	if (nbPixels == 0 || error.getRows() % nbPixels != 0) {
		// Genuinely unrecognizable layout: skip, but say so loudly rather
		// than failing silently as the previous version did.
		std::cout << "### WARNING: gate skipped, dim(error) = "
		          << error.getRows() << " is not a multiple of " << nbPixels << std::endl;
		structG.resize(0);
		return;
	}

	const unsigned int nbChan = error.getRows() / nbPixels;

	structG.resize(nbPixels);
	double sumG = 0.0;
	unsigned int k = 0;
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j++) {
			double gh = w_h[i][j], gv = w_v[i][j], gd = w_d[i][j];
			structG[k] = std::sqrt(gh * gh + gv * gv + gd * gd);
			sumG += structG[k];
			k++;
		}
	}

	double Gmean = sumG / nbPixels;
	if (Gmean < 1e-12) Gmean = 1e-12;

	for (unsigned int idx = 0; idx < nbPixels; idx++) {
		double Gbar = structG[idx] / Gmean;
		if (Gbar < 0.1) Gbar = 0.1;
		for (unsigned int c = 0; c < nbChan; c++) {
			unsigned int p = c * nbPixels + idx;   // CHANNEL-BLOCKED
			// unsigned int p = idx * nbChan + c;  // INTERLEAVED
			error_n[p] = error[p] / Gbar;
		}
	}
}


// =====================================================================
//  STEP 3 -- the gate measures brightness, not structure.
//
//  This is independent of STEP 1 and 2 and is the substantive weakness of
//  the contribution as implemented.
//
//  w_h, w_v, w_d are I filtered by Dnm2, Dnm3, Dnm4, and each Dnm is the
//  SUM over n,m = 0..4 of separable Gauss-Hermite products. That sum is
//  low-pass dominated: its DC gain relative to its L1 norm is 1.000, so
//  Dnm * I is close to a local mean of I, and structG is therefore close to
//  local BRIGHTNESS. Dropping the (0,0) term does not fix this -- measured,
//  the ratio only falls to 0.810, because the even orders also carry DC.
//
//  Consequence for the occluded experiments: a BLACK occluder has low local
//  brightness, so structG is small there, Gbar clamps at 0.1, the residual
//  is amplified ten-fold and Tukey rejects it. The mechanism appears to
//  work -- but for the wrong reason, and a BRIGHT outlier (a specularity,
//  the case the chapter also claims) would be PROTECTED instead of
//  rejected. Do not write the brightness-independent claim in 4.6 unless
//  the gate is changed.
//
//  The fix is first-order kernels, which are odd and so have exactly zero
//  DC. Add these four alongside Hermite() -- they leave Dnm1..Dnm4 and
//  therefore the feature vector completely untouched, so only V3 changes
//  and V1/V2 need not be re-run.
// =====================================================================

vpMatrix G10s2(kersize, kersize), G01s2(kersize, kersize);
vpMatrix G10s3(kersize, kersize), G01s3(kersize, kersize);
vpMatrix G10s4(kersize, kersize), G01s4(kersize, kersize);

void HermiteDerivKernels()
{
	G10s2 = 0; G01s2 = 0; G10s3 = 0; G01s3 = 0; G10s4 = 0; G01s4 = 0;

	for (int i = 0; i < kersize; i++) {
		for (int j = 0; j < kersize; j++) {
			double x = j - ((kersize - 1) / 2.0);
			double y = ((kersize - 1) / 2.0) - i;

			// order (1,0) and (0,1) only: D_1 is odd, so sum(kernel) = 0
			G10s2[i][j] = dnFunction(1, x, sigma2) * dnFunction(0, y, sigma2);
			G01s2[i][j] = dnFunction(0, x, sigma2) * dnFunction(1, y, sigma2);
			G10s3[i][j] = dnFunction(1, x, sigma3) * dnFunction(0, y, sigma3);
			G01s3[i][j] = dnFunction(0, x, sigma3) * dnFunction(1, y, sigma3);
			G10s4[i][j] = dnFunction(1, x, sigma4) * dnFunction(0, y, sigma4);
			G01s4[i][j] = dnFunction(0, x, sigma4) * dnFunction(1, y, sigma4);
		}
	}
}

// Call HermiteDerivKernels() once, right after Hermite(), in init().
// Then declare six more double images alongside w_h/w_v/w_d
//
//   vpImage<double> g10_2(240,320,0), g01_2(240,320,0);
//   vpImage<double> g10_3(240,320,0), g01_3(240,320,0);
//   vpImage<double> g10_4(240,320,0), g01_4(240,320,0);
//
// and filter the CURRENT image into them at the end of Ihermite():
//
//   vpImageFilter::filter(I, g10_2, G10s2); vpImageFilter::filter(I, g01_2, G01s2);
//   vpImageFilter::filter(I, g10_3, G10s3); vpImageFilter::filter(I, g01_3, G01s3);
//   vpImageFilter::filter(I, g10_4, G10s4); vpImageFilter::filter(I, g01_4, G01s4);
//
// Finally replace the structG line in the gate with the derivative energy,
// which is a genuine multi-scale gradient magnitude and is invariant to any
// additive brightness offset:
//
//   structG[k] = std::sqrt(g10_2[i][j]*g10_2[i][j] + g01_2[i][j]*g01_2[i][j]
//                        + g10_3[i][j]*g10_3[i][j] + g01_3[i][j]*g01_3[i][j]
//                        + g10_4[i][j]*g10_4[i][j] + g01_4[i][j]*g01_4[i][j]);
//
// Sanity check before trusting it: print sum(G10s2) -- it must be ~1e-17,
// not ~1. If it is not, dnFunction(1,.) is not odd and something is wrong
// with the kernel indexing.
