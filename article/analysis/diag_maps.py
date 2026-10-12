#!/usr/bin/env python3
"""Final-pass weight and influence maps of diagnostic runs (herpvs_campaign -m).

  python3 diag_maps.py --out fig.pdf PREFIX[:LABEL] ...

PREFIX is the -m prefix of a run; the maps PREFIX_w_<pass>.f32 and
PREFIX_infl_<pass>.f32 are read at --pass (default 600). Weight: mean over
the four channels. Influence: sum over channels of ||W l_j|| |W e_j|, shown on
a log scale relative to its maximum.
"""

import argparse
import os
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import analyze as an  # noqa: E402

H, W = 220, 300


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("runs", nargs="+")
    ap.add_argument("--pass", dest="k", type=int, default=600)
    ap.add_argument("--out", required=True)
    args = ap.parse_args(argv)
    plt = an.setup_mpl()
    n = len(args.runs)
    fig, axes = plt.subplots(2, n, figsize=(2.35 * n, 3.3), constrained_layout=True, squeeze=False)
    for j, spec in enumerate(args.runs):
        prefix, _, label = spec.partition(":")
        label = label or os.path.basename(prefix)
        w = np.fromfile(f"{prefix}_w_{args.k}.f32", dtype=np.float32).reshape(H, W)
        infl = np.fromfile(f"{prefix}_infl_{args.k}.f32", dtype=np.float32).reshape(H, W)
        axes[0, j].imshow(w, cmap="gray", vmin=0, vmax=1)
        axes[0, j].set_title(f"{label}: mean weight")
        axes[1, j].imshow(np.log10(infl / infl.max() + 1e-4), cmap="gray", vmin=-4, vmax=0)
        axes[1, j].set_title(f"{label}: influence (log)")
        for a in axes[:, j]:
            a.set_xticks([])
            a.set_yticks([])
            a.grid(False)
            for s in a.spines.values():
                s.set_visible(True)
                s.set_linewidth(0.5)
    fig.savefig(args.out)
    if args.out.endswith(".pdf"):
        fig.savefig(args.out[:-4] + ".png", dpi=150)


if __name__ == "__main__":
    main()
