// =====================================================================
//  weight_maps.cpp  --  weight and structure maps for Section 4.7.3
//
//  Two images per call, written as PNG so they drop straight into the
//  thesis as figures:
//
//    wmap_<tag>_<iter>.png   the Tukey weights w_j at their image
//                            positions. White = retained (w = 1),
//                            black = rejected (w = 0).
//
//    gmap_<tag>_<iter>.png   the structure measure G(p) of eq (4.9).
//                            This is the one that settles the
//                            brightness-vs-structure question: if it looks
//                            like the photograph, G tracks intensity; if it
//                            looks like an edge drawing, G tracks structure.
//
//  The weight vector has 264000 entries = 4 channels x 66000, so there are
//  four weights per pixel. The map shows their MEAN, which is the quantity
//  that matters for the control law: a pixel whose four responses are all
//  rejected contributes nothing, one with a mixed verdict contributes
//  partially. Change to a min() if you want the strictest reading.
// =====================================================================

// ---------------------------------------------------------------------
//  PART 1 -- add this function near Ihermite() / Idhermite(), and add its
//  declaration at the top next to the other prototypes:
//
//      void saveMaps(const vpColVector &w, const vpColVector &structG,
//                    int iterNum, const std::string &tag);
// ---------------------------------------------------------------------

void saveMaps(const vpColVector &w, const vpColVector &structG,
              int iterNum, const std::string &tag)
{
	const unsigned int wIn  = I.getWidth()  - 2 * bord;   // 300
	const unsigned int hIn  = I.getHeight() - 2 * bord;   // 220
	const unsigned int nbIn = hIn * wIn;                  // 66000

	if (nbIn == 0 || w.getRows() % nbIn != 0) return;
	const unsigned int nbCh = w.getRows() / nbIn;         // 4

	const std::string suffix = "_" + tag + "_" + std::to_string(iterNum) + ".png";

	// ---- weight map: mean weight over the four scales, 0..255 ----------
	// Border pixels carry no measurement; mid-grey marks them as "no data"
	// so they are not mistaken for rejections.
	vpImage<unsigned char> Iw(I.getHeight(), I.getWidth(), 128);
	for (unsigned int k = 0; k < nbIn; k++) {
		double m = 0.0;
		for (unsigned int c = 0; c < nbCh; c++) m += w[c * nbIn + k];
		m /= nbCh;
		if (m < 0.0) m = 0.0;
		if (m > 1.0) m = 1.0;
		Iw[bord + k / wIn][bord + k % wIn] = (unsigned char)(255.0 * m + 0.5);
	}
	vpImageIo::write(Iw, "wmap" + suffix);

	// ---- structure map: G(p), scaled so 3 x mean saturates -------------
	// Normalizing by the maximum would let a handful of bright pixels wash
	// the whole map out; three times the mean keeps the mid-tones visible.
	if (structG.getRows() == nbIn) {
		double sum = 0.0;
		for (unsigned int k = 0; k < nbIn; k++) sum += structG[k];
		double scale = 3.0 * sum / nbIn;
		if (scale < 1e-12) scale = 1e-12;

		vpImage<unsigned char> Ig(I.getHeight(), I.getWidth(), 0);
		for (unsigned int k = 0; k < nbIn; k++) {
			double g = 255.0 * structG[k] / scale;
			if (g > 255.0) g = 255.0;
			if (g < 0.0) g = 0.0;
			Ig[bord + k / wIn][bord + k % wIn] = (unsigned char)(g + 0.5);
		}
		vpImageIo::write(Ig, "gmap" + suffix);
	}
}

// ---------------------------------------------------------------------
//  PART 2 -- the call. Put it in the do{} loop AFTER the MEstimator call
//  and after computeHermiteNormalizedResidual(), so w and structG are both
//  filled for the current iteration.
//
//  iter has already been incremented by the cout at the top of the loop,
//  so iter-1 is the true iteration number. These four points show the
//  weighting at the start, during the transient, after it, and at the end.
// ---------------------------------------------------------------------

		if (iter == 2 || iter == 11 || iter == 51 || iter == 601) {
			saveMaps(w, structG, iter - 1, "v3");
		}

//  In V2 there is no structG. Call it with an empty vector -- the guard
//  above skips the structure map and you still get the weight map:
//
//      saveMaps(w, vpColVector(), iter - 1, "v2");
//
//  In V1, D = I by construction, so the weight map is uniformly white and
//  carries no information. Do not produce one; instead amend Section 4.6.4
//  to promise weight maps for Variants 2 and 3 only.
// ---------------------------------------------------------------------


// =====================================================================
//  HOW TO READ THE TWO MAPS
//
//  gmap, the decisive one. Open it next to Fig. 4.1(a).
//
//    Looks like the photograph (faces bright, dark suit and background
//    dark)        -> G tracks INTENSITY. Section 4.4.1's "band-pass
//                    character" sentence is wrong and must go, and eq (4.9)
//                    needs the first-order kernels before the chapter can
//                    claim structure-awareness.
//
//    Looks like an edge drawing (bright along contours, dark inside both
//    the light and the dark flat areas)
//                 -> G tracks STRUCTURE and the chapter's claim stands as
//                    written.
//
//  There is no ambiguous outcome here, which is why this figure is worth
//  more than the clamp statistics: it answers the question in one glance
//  and it is publishable either way.
//
//  wmap, for the occluded runs. This is the figure that makes the
//  robustness claim concrete rather than asserted: the ellipse should
//  appear as a black blob in V2 and V3 and be absent in V1. Two further
//  things to check, because both are claims the chapter makes:
//
//    - Section 4.3.2 argues the affected region is the occluder DILATED by
//      the filter support. Measure the black blob against the ellipse in
//      the input texture; it should be visibly larger, and that is direct
//      evidence for the paragraph on p.65.
//
//    - Section 4.3.3 predicts that early in the run, with the misalignment
//      still gross, valid measurements are rejected across the whole
//      frame. wmap at iteration 1 should therefore be widely speckled and
//      wmap at 600 mostly white. If iteration 1 is already clean, that
//      paragraph is overstated.
//
//  For the nominal case the weight maps are a control: they should show no
//  spatially coherent dark region, since nothing is corrupted.
// =====================================================================
