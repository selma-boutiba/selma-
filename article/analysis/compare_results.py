#!/usr/bin/env python3
"""Compare a re-run of a campaign with the committed results.

  python3 analysis/compare_results.py results/confirmatory confirmatory

Both arguments are output directories of analyze.py. Runs are matched by
weighting, interaction matrix, scene and pose. Reports, per outcome, the
largest and median relative difference of the final errors, the runs whose
success status differs, and the primary-family results side by side.
Differences of a few per cent in single runs are expected across compilers
and ViSP versions (VALIDATION.md, section 2); the success counts and the
statistics should agree to the reported precision.
"""

import argparse
import os

import numpy as np
import pandas as pd

KEY = ["weighting", "interaction", "scenario", "pose_id"]


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("mine")
    ap.add_argument("reference")
    args = ap.parse_args(argv)
    a = pd.read_csv(os.path.join(args.mine, "runs.csv"))
    b = pd.read_csv(os.path.join(args.reference, "runs.csv"))
    m = a.merge(b, on=KEY, suffixes=("_mine", "_ref"))
    print(f"{len(a)} runs here, {len(b)} in the reference, {len(m)} matched")
    for col in ("final_t_mm", "final_r_deg"):
        rel = np.abs(m[col + "_mine"] - m[col + "_ref"]) / m[col + "_ref"].abs().clip(lower=1e-12)
        print(f"{col}: relative difference median {np.median(rel):.2e}, max {rel.max():.2e}")
    diff = m[m.success_mine != m.success_ref]
    print(f"runs whose success differs: {len(diff)}")
    if len(diff):
        print(diff[KEY + ["final_t_mm_mine", "final_t_mm_ref"]].to_string(index=False))
    pa, pb = os.path.join(args.mine, "primary.csv"), os.path.join(args.reference, "primary.csv")
    if os.path.exists(pa) and os.path.exists(pb):
        x = pd.read_csv(pa)[["test", "interaction", "scenario", "outcome", "n", "effect", "p_holm"]]
        y = pd.read_csv(pb)[["test", "interaction", "scenario", "outcome", "n", "effect", "p_holm"]]
        print("\nprimary family (here | reference)")
        print(x.merge(y, on=["test", "interaction", "scenario", "outcome"], suffixes=(" here", " ref"))
              .to_string(index=False, float_format=lambda v: f"{v:.4g}"))


if __name__ == "__main__":
    main()
