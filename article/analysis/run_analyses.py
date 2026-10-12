#!/usr/bin/env python3
"""The pre-registered analyses (any operating system).

  python analysis/run_analyses.py confirmatory replication sweep [--runs runs] [--out results]

Runs analyze.py on runs/<design>/logs with exactly the arguments fixed in
PREREGISTRATION.md (confirmatory) and PREREGISTRATION_ADDENDUM.md (R.6
replication, S.6 sweep), writes to results/<design>/, then the sweep
figures (which need results/confirmatory) and the exploratory iteration-
budget ratio of each design.
"""

import argparse
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)
import analyze  # noqa: E402
import budget_ratio  # noqa: E402
import sweep_figures  # noqa: E402

POSES = os.path.join(ROOT, "campaign", "poses.csv")
SWEEP = ["uniform_dark_f150", "uniform_dark_f250", "uniform_dark_f350", "uniform_dark_f450"]


def arguments(design, logs, out):
    base = [logs, "--poses", POSES, "--out", out]
    if design == "confirmatory":   # the defaults of analyze.py encode the primary family
        return base
    if design == "replication":
        return base + ["--primary", "desired:coffee_dark_disc:hermite:tukey", "--primary-success", "",
                       "--decision-labels", "replicated,not replicated"]
    if design == "sweep":
        p = ",".join(f"{L}:{s}:hermite:tukey" for L in ("current", "desired") for s in SWEEP) + ","
        return base + ["--primary", p, "--primary-success", p,
                       "--decision-labels", "gain at every fraction,gain not at every fraction"]
    if design == "pilot":
        return [logs, "--poses", os.path.join(ROOT, "campaign", "poses_pilot.csv"), "--out", out]
    sys.exit(f"no pre-registered analysis for {design}")


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("designs", nargs="+")
    ap.add_argument("--runs", default=os.path.join(ROOT, "runs"))
    ap.add_argument("--out", default=os.path.join(ROOT, "results"))
    ap.add_argument("--textures", default=os.path.join(ROOT, "textures"))
    args = ap.parse_args(argv)
    for d in args.designs:
        logs, out = os.path.join(args.runs, d, "logs"), os.path.join(args.out, d)
        print(f"== {d}: {logs} -> {out}", flush=True)
        analyze.main(arguments(d, logs, out))
        print(open(os.path.join(out, "decision.txt")).read().strip())
        if d in ("confirmatory", "replication", "sweep"):
            budget_ratio.main([logs, "--out", os.path.join(out, "budget_ratio.csv")])
        if d == "sweep":
            conf = os.path.join(args.out, "confirmatory")
            if os.path.isdir(conf):
                sweep_figures.main(["--sweep", out, "--confirmatory", conf, "--masks", args.textures,
                                    "--poses", POSES])
            else:
                print("sweep figures skipped: analyse the confirmatory campaign first")


if __name__ == "__main__":
    main()
