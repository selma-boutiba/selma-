// =====================================================================
//  WHERE TO PASTE  --  tutorial-viewer13.cpp (V3), inside the do{} loop
//
//  The five lines marked  >>> PASTE  go immediately after
//  sI.interaction(Lsd);  which is line 548, and before the long comment
//  block that precedes computeHermiteNormalizedResidual().
//
//  Everything not marked >>> PASTE is your existing code, shown only so
//  you can line up the anchors. Do not duplicate it.
// =====================================================================

		// ---------- Levenberg Marquardt method --------------

		if (iter == 2)
		{
			Ne = normeErrorI;
		}

		sI.interaction(Lsd);

// >>> PASTE FROM HERE
		if (iter == 2) {
			std::cout << "### dim(error)  = " << error.getRows() << std::endl;
			std::cout << "### nbPixels    = " << (I.getHeight() * I.getWidth()) << std::endl;
			std::cout << "### dim(Lsd)    = " << Lsd.getRows() << " x " << Lsd.getCols() << std::endl;
			std::cout << "### gate active = "
			          << ((error.getRows() == I.getHeight() * I.getWidth()) ? "YES" : "NO -- V3 IS RUNNING AS V2")
			          << std::endl;
		}
// >>> PASTE TO HERE

		// Robust re-weighting: compute a Tukey M-estimator weight per pixel,
		// then apply it to both the interaction matrix and the error so
		// outliers (occlusions, specularities) are down-weighted in the
		// normal equations, following Collewet & Marchand, "Photometric
		// visual servoing", IEEE T-RO 2011. The residual fed to the
		// M-estimator is first Hermite-normalized (see
		// computeHermiteNormalizedResidual()) so the robust threshold is
		// structure-aware instead of using one global image-wide scale.
		computeHermiteNormalizedResidual(error, structG, error_n);

// =====================================================================
//  The print fires once, on the first pass through the loop, because iter
//  was already incremented to 2 by the cout at the top of the loop. It
//  sits in the same "if (iter == 2)" condition that sets Ne, so you can
//  also simply add the four cout lines inside THAT existing block if you
//  prefer -- the effect is identical.
//
//  Output to look for, near the top of the console:
//
//    ### dim(error)  = 76800        -> gate active
//    ### dim(error)  = 307200       -> gate dead, V3 has been running as V2
//
//  Nothing else in the run changes, so you can leave the print in place.
// =====================================================================
