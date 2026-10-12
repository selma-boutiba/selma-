#!/usr/bin/env python3
"""V3/V2 accuracy against the iteration budget (exploratory).

For every (interaction, scene) cell of a campaign, over the poses where both
V2 and V3 succeed at the final pass, the median per-pose ratio of the
camera-centre errors V3/V2 at several iterations k, each error being the
median over the 25 iterations up to k; and the median reduction of each
weighting's error between iterations 300 and the last.

  python3 budget_ratio.py <logdir> --out <csv>
"""

import argparse
import glob
import os
import re

import numpy as np
import pandas as pd

KS = [100, 200, 300, 400, 500, 550, 600]


def trajectory(path):
    d = pd.read_csv(path, comment="#", usecols=["tx", "ty", "tz", "tux", "tuy", "tuz"])
    t = 1e3 * np.sqrt(d.tx ** 2 + d.ty ** 2 + d.tz ** 2).to_numpy()
    r = np.degrees(np.sqrt(d.tux ** 2 + d.tuy ** 2 + d.tuz ** 2)).to_numpy()
    return t, r


def success(t, r, t_thr=20.0, r_thr=1.0):
    return np.median(t[-50:]) < t_thr and np.median(r[-50:]) < r_thr


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("logdir")
    ap.add_argument("--out", required=True)
    args = ap.parse_args(argv)
    pat = re.compile(r"L(\d)_(.+)_w(\d)_p(\d+)\.csv$")
    cells = {}
    for f in glob.glob(os.path.join(args.logdir, "*.csv")):
        m = pat.search(os.path.basename(f))
        if m and m.group(3) in ("2", "3"):
            cells.setdefault((m.group(1), m.group(2)), {}).setdefault(int(m.group(4)), {})[m.group(3)] = f
    rows = []
    for (L, scen), poses in sorted(cells.items()):
        per = []
        for p, files in poses.items():
            if set(files) != {"2", "3"}:
                continue
            t2, r2 = trajectory(files["2"])
            t3, r3 = trajectory(files["3"])
            if not (success(t2, r2) and success(t3, r3)):
                continue
            per.append([np.median(t3[k - 25:k]) / np.median(t2[k - 25:k]) for k in KS] +
                       [t2[-1] / t2[299], t3[-1] / t3[299]])
        if not per:
            continue
        a = np.median(np.array(per), axis=0)
        row = {"interaction": "current" if L == "0" else "desired", "scenario": scen, "n_both": len(per)}
        row.update({f"ratio_k{k}": v for k, v in zip(KS, a[:len(KS)])})
        row.update({"V2_t_last_over_t300": a[-2], "V3_t_last_over_t300": a[-1]})
        rows.append(row)
    df = pd.DataFrame(rows)
    df.to_csv(args.out, index=False, float_format="%.4f")
    print(df.round(2).to_string(index=False))


if __name__ == "__main__":
    main()
