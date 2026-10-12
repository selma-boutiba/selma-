#!/usr/bin/env python3
"""Plot one run of herpvs_campaign from its log.

  python3 analysis/plot_run.py one_run.csv [--out one_run.png]

Panels: camera-centre and rotation error of the camera relative to the
desired pose, the feature cost ||e||^2, and the gain lambda, against the
iteration. Prints the final errors (median of the last 50 iterations), as
analyze.py computes them.
"""

import argparse
import os
import sys

import numpy as np
import pandas as pd

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import analyze as an  # noqa: E402


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("log")
    ap.add_argument("--out", default=None, help="image file (default: <log>.png)")
    args = ap.parse_args(argv)
    meta, d = an.read_run(args.log)
    t = 1e3 * np.sqrt(d.tx ** 2 + d.ty ** 2 + d.tz ** 2)
    r = np.degrees(np.sqrt(d.tux ** 2 + d.tuy ** 2 + d.tuz ** 2))
    tf, rf = t.iloc[-50:].median(), r.iloc[-50:].median()
    print(f"{meta.get('weighting')}, L from the {meta.get('interaction')} image, pose {meta.get('pose_id')}: "
          f"final camera-centre error {tf:.4g} mm, rotation error {rf:.4g} deg, "
          f"{'success' if tf < 20 and rf < 1 else 'failure'} (20 mm, 1 deg)")
    plt = an.setup_mpl()
    fig, axes = plt.subplots(1, 3, figsize=(9.0, 2.6), constrained_layout=True)
    axes[0].semilogy(d["iter"], t, color="#2a78d6", label="camera centre (mm)")
    axes[0].semilogy(d["iter"], r, color="#eb6834", ls="--", label="rotation (deg)")
    axes[0].legend(frameon=False)
    axes[0].set_title("pose error")
    axes[1].semilogy(d["iter"], d.e2, color="#0b0b0b")
    axes[1].set_title("feature cost ||e||^2")
    axes[2].semilogy(d["iter"], d["lambda"], color="#0b0b0b")
    axes[2].set_title("gain lambda")
    for a in axes:
        a.set_xlabel("iteration")
    out = args.out or os.path.splitext(args.log)[0] + ".png"
    fig.savefig(out, dpi=150)
    print(f"figure: {out}")


if __name__ == "__main__":
    main()
