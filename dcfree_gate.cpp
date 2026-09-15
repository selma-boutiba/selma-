// =====================================================================
//  dcfree_gate.cpp  --  make the structure measure actually measure
//                       structure. V3 only.
//
//  WHY. gmap_v3_1.pgm is a recognizable copy of the target photograph.
//  Measured on it: mean|grad G| / mean G = 0.062, and 62.8% of pixels lie
//  within 10% of their own 9x9 local mean -- G is a smoothed intensity,
//  not a structure measure. The weights follow it: mean weight rises from
//  109 in the second-darkest decile of G to 248 in the ninth, and
//  corr(gmap, wmap) = +0.484. The gate's operative rule is "trust the
//  bright half of the image".
//
//  The cause is that w_h, w_v, w_d are I filtered by Dnm2, Dnm3, Dnm4, and
//  each Dnm is the SUM over n,m = 0..4 of Gauss-Hermite products. That sum
//  has a DC gain equal to its L1 norm to three decimal places. Excluding
//  the (0,0) term does not help: measured, the ratio only falls from 1.000
//  to 0.810, because the even orders carry DC too.
//
//  The remedy is to use the FIRST-ORDER kernels alone. D_1 is an odd
//  function, so D_{1,0} and D_{0,1} have exactly zero DC gain and their
//  response is a true gradient magnitude, invariant to any additive
//  brightness offset.
//
//  Dnm1..Dnm4 are NOT touched, so the feature vector and the interaction
//  matrix are unchanged. V1 and V2 need no re-run; only V3 changes.
//
//  FIVE EDITS, all in tutorial-viewer13.cpp.
// =====================================================================


// ---------------------------------------------------------------------
//  EDIT 1 -- prototypes, at the top beside the other declarations
//  (around line 121). dnFunction is defined late in the file, so it needs
//  one too if it does not already have one.
// ---------------------------------------------------------------------

double dnFunction(int n, double x, double sigma);
void HermiteDerivKernels();


// ---------------------------------------------------------------------
//  EDIT 2 -- globals, next to the Dnm1..Dnm4 declarations (around line
//  221) and next to w_a/w_h/w_v/w_d (around line 191).
// ---------------------------------------------------------------------

vpMatrix G10s2(kersize, kersize), G01s2(kersize, kersize);
vpMatrix G10s3(kersize, kersize), G01s3(kersize, kersize);
vpMatrix G10s4(kersize, kersize), G01s4(kersize, kersize);

vpImage<double> g10_2(240, 320, 0); vpImage<double> g01_2(240, 320, 0);
vpImage<double> g10_3(240, 320, 0); vpImage<double> g01_3(240, 320, 0);
vpImage<double> g10_4(240, 320, 0); vpImage<double> g01_4(240, 320, 0);


// ---------------------------------------------------------------------
//  EDIT 3 -- the kernel builder, placed right after Hermite() so that
//  dnFunction is already defined above it.
//
//  Only orders (1,0) and (0,1) are used, at the same three scales the
//  structure measure of eq (4.9) uses. No summation over orders: that is
//  precisely what reintroduced the DC term.
// ---------------------------------------------------------------------

void HermiteDerivKernels()
{
	G10s2 = 0; G01s2 = 0; G10s3 = 0; G01s3 = 0; G10s4 = 0; G01s4 = 0;

	for (int i = 0; i < kersize; i++) {
		for (int j = 0; j < kersize; j++) {
			double x = j - ((kersize - 1) / 2.0);
			double y = ((kersize - 1) / 2.0) - i;

			G10s2[i][j] = dnFunction(1, x, sigma2) * dnFunction(0, y, sigma2);
			G01s2[i][j] = dnFunction(0, x, sigma2) * dnFunction(1, y, sigma2);
			G10s3[i][j] = dnFunction(1, x, sigma3) * dnFunction(0, y, sigma3);
			G01s3[i][j] = dnFunction(0, x, sigma3) * dnFunction(1, y, sigma3);
			G10s4[i][j] = dnFunction(1, x, sigma4) * dnFunction(0, y, sigma4);
			G01s4[i][j] = dnFunction(0, x, sigma4) * dnFunction(1, y, sigma4);
		}
	}

	// Sanity check. D_1 is odd, so every one of these sums must vanish to
	// rounding. If any prints order 1 rather than order 1e-17, the kernel
	// indexing is wrong and the gate will be no better than before.
	double s2 = 0, s3 = 0, s4 = 0;
	for (int i = 0; i < kersize; i++)
		for (int j = 0; j < kersize; j++) {
			s2 += G10s2[i][j]; s3 += G10s3[i][j]; s4 += G10s4[i][j];
		}
	std::cout << "### sum(G10) per scale = " << s2 << "  " << s3 << "  " << s4
	          << "   (must be ~1e-17)" << std::endl;
}


// ---------------------------------------------------------------------
//  EDIT 4 -- two call sites.
//
//  (a) in init(), immediately after the existing  Hermite();
// ---------------------------------------------------------------------

	HermiteDerivKernels();

// ---------------------------------------------------------------------
//  (b) at the END of Ihermite(), after the existing vpImageFilter::filter
//      calls. Only the CURRENT image is needed: the gate is evaluated on I,
//      never on Id, so Idhermite() is left alone.
// ---------------------------------------------------------------------

	vpImageFilter::filter(I, g10_2, G10s2); vpImageFilter::filter(I, g01_2, G01s2);
	vpImageFilter::filter(I, g10_3, G10s3); vpImageFilter::filter(I, g01_3, G01s3);
	vpImageFilter::filter(I, g10_4, G10s4); vpImageFilter::filter(I, g01_4, G01s4);


// ---------------------------------------------------------------------
//  EDIT 5 -- in computeHermiteNormalizedResidual(), replace the three
//  lines that build structG from w_h/w_v/w_d:
//
//      double gh = w_h[i][j], gv = w_v[i][j], gd = w_d[i][j];
//      structG[k] = std::sqrt(gh * gh + gv * gv + gd * gd);
//
//  with the derivative energy over the same three scales. Everything else
//  in that function -- the interior indexing, Gmean, the 0.1 floor, the
//  per-channel application -- stays exactly as it is.
// ---------------------------------------------------------------------

		structG[k] = std::sqrt(g10_2[i][j] * g10_2[i][j] + g01_2[i][j] * g01_2[i][j]
		                     + g10_3[i][j] * g10_3[i][j] + g01_3[i][j] * g01_3[i][j]
		                     + g10_4[i][j] * g10_4[i][j] + g01_4[i][j] * g01_4[i][j]);


// =====================================================================
//  WHAT TO SEND BACK
//
//   1. the "### sum(G10) per scale" line -- three numbers near 1e-17;
//   2. gmap_v3_1.pgm -- it must now look like an EDGE DRAWING: bright along
//      the contours of the face, hair and shoulder, dark inside the flat
//      areas whether those areas are light or dark. If it still resembles
//      the photograph, the edit did not take effect;
//   3. wmap_v3_1.pgm and wmap_v3_600.pgm;
//   4. the tail of the console at iteration 600 -- the six errorpose values,
//      |e|, lambda, |Tc|.
//
//  For (3), extend the save condition so the last iteration is captured:
//
//      if (iter == 2 || iter == 601) saveMaps(w, structG, iter - 1, "v3");
//
//  THE COMPARISON THAT MATTERS is item 4 against the run you already have:
//  rotational error 0.089 deg, |e| 6.59e7, |Tc| 2.88e-5, lambda 0.0365.
//
//    Better or equal      -> Section 4.4 stands as written, the structure
//                            claim is earned, and the brightness-based run
//                            becomes an ablation of the structure measure
//                            itself -- a stronger chapter than either alone.
//
//    Clearly worse        -> report both. The brightness measure outperforms
//                            the structure measure on this scene, which is a
//                            real and publishable finding: it says the
//                            informative regions of this target are the
//                            bright ones, and that G's DC component was
//                            doing useful work rather than merely being an
//                            oversight. Section 4.4 then has to be rewritten
//                            around response magnitude instead of structure.
//
//  Either outcome is a result. What is not acceptable is the current state,
//  in which the text claims one mechanism and the code implements another.
// =====================================================================
