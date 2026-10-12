#!/usr/bin/env python3
"""Occlusion-fraction sweep: success and accuracy against the occluded
fraction (PREREGISTRATION_ADDENDUM.md, S.4-S.5).

Inputs are the outputs of analyze.py for the sweep and for the confirmatory
design; the confirmatory dark disc (7.5 % at the desired pose) is drawn as
the first point of each curve, with its own marker, since its occluder is a
disc and not one of the sweep's ellipses.

  success      rate per (L, weighting, fraction), Clopper-Pearson 95 % CI
  accuracy     Hodges-Lehmann ratio V3/V2 over the poses where both succeed,
               exact 95 % CI, taken from analyze.py's comparisons.csv
  breakdown    smallest fraction at which the success rate falls below 1/2
  coverage     success and first-pass rejected fraction against the
               coverage of the occluder at each initial pose

Run:
  python3 sweep_figures.py --sweep sweep --confirmatory confirmatory \
      --masks <dir with uniform_dark_f*_mask.png and mask_disc.png> \
      --poses campaign/poses.csv
"""

import argparse
import os
import sys

import numpy as np
import pandas as pd
from PIL import Image
from scipy import stats

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, "..", "tools"))
import analyze as an  # noqa: E402
import make_occluders as mo  # noqa: E402

FRACTIONS = {"uniform_dark_disc": 0.075, "uniform_dark_f150": 0.15, "uniform_dark_f250": 0.25,
             "uniform_dark_f350": 0.35, "uniform_dark_f450": 0.45}
MASKS = {"uniform_dark_disc": "mask_disc.png", "uniform_dark_f150": "uniform_dark_f150_mask.png",
         "uniform_dark_f250": "uniform_dark_f250_mask.png", "uniform_dark_f350": "uniform_dark_f350_mask.png",
         "uniform_dark_f450": "uniform_dark_f450_mask.png"}
WEIGHTS = ["tukey", "hermite"]
INTER = ["current", "desired"]


def clopper_pearson(k, n, conf=0.95):
    a = 1 - conf
    lo = stats.beta.ppf(a / 2, k, n - k + 1) if k > 0 else 0.0
    hi = stats.beta.ppf(1 - a / 2, k + 1, n - k) if k < n else 1.0
    return lo, hi


def load(dirname, scenarios):
    runs = pd.read_csv(os.path.join(dirname, "runs.csv"))
    comps = pd.read_csv(os.path.join(dirname, "comparisons.csv"))
    mcn = pd.read_csv(os.path.join(dirname, "mcnemar.csv"))
    keep = lambda d: d[d.scenario.isin(scenarios)]  # noqa: E731
    return keep(runs[runs.weighting.isin(WEIGHTS)]), keep(comps), keep(mcn)


def coverage_table(mask_dir, poses_path):
    poses = mo.read_poses(poses_path)
    rows = []
    for scen, fname in MASKS.items():
        m = np.asarray(Image.open(os.path.join(mask_dir, fname)).convert("L")) > 127
        for pid, pose in poses:
            rows.append({"scenario": scen, "pose_id": pid, "coverage_initial": mo.image_mask(m, pose).mean()})
    return pd.DataFrame(rows)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--sweep", required=True)
    ap.add_argument("--confirmatory", required=True)
    ap.add_argument("--masks", required=True)
    ap.add_argument("--poses", required=True)
    args = ap.parse_args(argv)

    r1, c1, m1 = load(args.sweep, FRACTIONS)
    r0, c0, m0 = load(args.confirmatory, ["uniform_dark_disc"])
    runs = pd.concat([r0, r1], ignore_index=True)
    comps = pd.concat([c0.assign(source="confirmatory"), c1.assign(source="sweep")], ignore_index=True)
    mcn = pd.concat([m0.assign(source="confirmatory"), m1.assign(source="sweep")], ignore_index=True)
    runs["fraction"] = runs.scenario.map(FRACTIONS)
    cov = coverage_table(args.masks, args.poses)
    runs = runs.merge(cov, on=["scenario", "pose_id"], how="left")

    # Table: one row per (L, fraction)
    rows = []
    for inter in INTER:
        for scen, f in FRACTIONS.items():
            g = runs[(runs.interaction == inter) & (runs.scenario == scen)]
            row = {"interaction": inter, "fraction": f, "scenario": scen}
            for w in WEIGHTS:
                s = g[g.weighting == w]
                k, n = int(s.success.sum()), len(s)
                lo, hi = clopper_pearson(k, n)
                ok = s[s.success]
                row.update({f"{w}_success": k, f"{w}_n": n, f"{w}_rate_lo": lo, f"{w}_rate_hi": hi,
                            f"{w}_t_med": ok.final_t_mm.median(), f"{w}_r_med": ok.final_r_deg.median()})
            for out, tag in (("final_t_mm", "t"), ("final_r_deg", "r")):
                c = comps[(comps.interaction == inter) & (comps.scenario == scen) & (comps.A == "hermite") &
                          (comps.B == "tukey") & (comps.outcome == out)]
                if len(c):
                    c = c.iloc[0]
                    both = c.family == "primary"  # primary rows are already over the poses where both succeed
                    row.update({f"ratio_{tag}": c.ratio if both else c.ratio_s,
                                f"ci_lo_{tag}": c.ci95_lo if both else c.ci95_lo_s,
                                f"ci_hi_{tag}": c.ci95_hi if both else c.ci95_hi_s,
                                f"n_both_{tag}": c.n if both else c.n_s})
            m = mcn[(mcn.interaction == inter) & (mcn.scenario == scen) & (mcn.A == "hermite") & (mcn.B == "tukey")]
            if len(m):
                m = m.iloc[0]
                row.update({"V3_only": m.A_only, "V2_only": m.B_only, "p_mcnemar": m.p_mcnemar})
            rows.append(row)
    table = pd.DataFrame(rows)
    # Holm-adjusted p of the success tests, from the sweep's primary family
    prim = os.path.join(args.sweep, "primary.csv")
    if os.path.exists(prim):
        p = pd.read_csv(prim)
        p2 = p[p.test == "P2"].set_index(["interaction", "scenario"]).p_holm
        p1 = p[p.test == "P1"]
        table["p_holm_success"] = [p2.get((i, s), np.nan) for i, s in zip(table.interaction, table.scenario)]
        for tag, out in (("t", "final_t_mm"), ("r", "final_r_deg")):
            q = p1[p1.outcome == out].set_index(["interaction", "scenario"]).p_holm
            table[f"p_holm_{tag}"] = [q.get((i, s), np.nan) if s != "uniform_dark_disc" else np.nan
                                      for i, s in zip(table.interaction, table.scenario)]
    table.to_csv(os.path.join(args.sweep, "sweep_table.csv"), index=False)

    # Breakdown fraction
    bd = []
    for inter in INTER:
        for w in WEIGHTS:
            t = table[table.interaction == inter].sort_values("fraction")
            rate = t[f"{w}_success"] / t[f"{w}_n"]
            below = t.fraction[rate < 0.5]
            bd.append({"interaction": inter, "weighting": w,
                       "breakdown_fraction": below.min() if len(below) else np.nan})
    pd.DataFrame(bd).to_csv(os.path.join(args.sweep, "breakdown.csv"), index=False)
    runs.to_csv(os.path.join(args.sweep, "sweep_runs_with_coverage.csv"), index=False)

    # Figures
    plt = an.setup_mpl()
    os.makedirs(os.path.join(args.sweep, "figures"), exist_ok=True)
    titles = {"current": "L from the current image", "desired": "L from the desired image"}

    fig, axes = plt.subplots(1, 2, figsize=(7.0, 2.5), constrained_layout=True, sharey=True)
    for ax, inter in zip(axes, INTER):
        t = table[table.interaction == inter].sort_values("fraction")
        for j, w in enumerate(WEIGHTS):
            x = 100 * t.fraction.to_numpy() + (j - 0.5) * 0.8
            y = (t[f"{w}_success"] / t[f"{w}_n"]).to_numpy()
            err = np.vstack([y - t[f"{w}_rate_lo"], t[f"{w}_rate_hi"] - y])
            ax.errorbar(x, y, yerr=err, color=an.COLORS[w], linestyle=an.STYLES[w], marker=an.MARKERS[w],
                        ms=4, capsize=2, label=an.LABELS[w], markeredgecolor="white", markeredgewidth=0.6)
            ax.plot(x[0], y[0], an.MARKERS[w], ms=7, mfc="none", color=an.COLORS[w])
        ax.set_xlabel("occluded fraction at the desired pose (%)")
        ax.set_title(titles[inter])
        ax.set_ylim(0, 1.05)
        ax.set_xticks([7.5, 15, 25, 35, 45], ["7.5", "15", "25", "35", "45"])
    axes[0].set_ylabel("success rate (32 poses, 95 % CI)")
    axes[0].legend(frameon=False, loc="lower left")
    fig.savefig(os.path.join(args.sweep, "figures", "sweep_success.pdf"))
    plt.close(fig)

    fig, ax = plt.subplots(figsize=(3.45, 2.5), constrained_layout=True)
    for j, inter in enumerate(INTER):
        t = table[(table.interaction == inter)].sort_values("fraction").dropna(subset=["ratio_t"])
        x = 100 * t.fraction.to_numpy() + (j - 0.5) * 0.8
        y = t.ratio_t.to_numpy()
        err = np.vstack([y - t.ci_lo_t, t.ci_hi_t - y])
        ax.errorbar(x, y, yerr=err, color=["#52514e", "#1baf7a"][j], marker=["o", "^"][j], ms=4, capsize=2,
                    linestyle=["--", "-"][j], label=titles[inter], markeredgecolor="white", markeredgewidth=0.6)
    ax.axhline(1.0, color="#0b0b0b", linewidth=0.6)
    ax.axhline(0.8, color="#0b0b0b", linewidth=0.6, linestyle=":")
    ax.set_xlabel("occluded fraction at the desired pose (%)")
    ax.set_ylabel("V3 / V2 camera-centre error")
    ax.set_xticks([7.5, 15, 25, 35, 45], ["7.5", "15", "25", "35", "45"])
    ax.legend(frameon=False, fontsize=6, loc="upper left")
    fig.savefig(os.path.join(args.sweep, "figures", "sweep_ratio.pdf"))
    plt.close(fig)

    fig, axes = plt.subplots(1, 2, figsize=(7.0, 2.5), constrained_layout=True, sharey=True)
    edges = np.array([0.0, 0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.80])
    for ax, inter in zip(axes, INTER):
        for w in WEIGHTS:
            s = runs[(runs.interaction == inter) & (runs.weighting == w)]
            b = np.digitize(s.coverage_initial, edges) - 1
            xs, ys = [], []
            for i in range(len(edges) - 1):
                sel = s[b == i]
                if len(sel) >= 5:
                    xs.append(100 * sel.coverage_initial.mean())
                    ys.append(sel.success.mean())
            ax.plot(xs, ys, color=an.COLORS[w], linestyle=an.STYLES[w], marker=an.MARKERS[w], ms=4,
                    label=an.LABELS[w], markeredgecolor="white", markeredgewidth=0.6)
        ax.set_xlabel("occluded fraction at the initial pose (%)")
        ax.set_title(titles[inter])
        ax.set_ylim(0, 1.05)
    axes[0].set_ylabel("success rate (bins of ≥ 5 runs)")
    axes[0].legend(frameon=False, loc="lower left")
    fig.savefig(os.path.join(args.sweep, "figures", "sweep_success_vs_initial_coverage.pdf"))
    plt.close(fig)

    # Markdown summary
    with open(os.path.join(args.sweep, "sweep_report.md"), "w") as fh:
        fh.write("| L | fraction | V2 success [95 % CI] | V3 success [95 % CI] | V3-only / V2-only | p Holm (success) |"
                 " V3/V2 camera centre [95 % CI], n | p Holm |\n|---|---|---|---|---|---|---|---|\n")
        for _, t in table.iterrows():
            def sr(w):
                return f"{t[f'{w}_success']:.0f}/{t[f'{w}_n']:.0f} [{t[f'{w}_rate_lo']:.2f}, {t[f'{w}_rate_hi']:.2f}]"
            ratio = (f"{t.ratio_t:.2f} [{t.ci_lo_t:.2f}, {t.ci_hi_t:.2f}], {t.n_both_t:.0f}"
                     if np.isfinite(t.get("ratio_t", np.nan)) else "n/a")
            fh.write(f"| {t.interaction} | {100 * t.fraction:.1f} % | {sr('tukey')} | {sr('hermite')} | "
                     f"{t.get('V3_only', np.nan):.0f} / {t.get('V2_only', np.nan):.0f} | "
                     f"{t.get('p_holm_success', np.nan):.2g} | {ratio} | {t.get('p_holm_t', np.nan):.2g} |\n")
        fh.write("\nBreakdown fraction (success < 1/2):\n\n")
        for b in bd:
            fh.write(f"- {b['interaction']}, {b['weighting']}: {b['breakdown_fraction']}\n")
    print(open(os.path.join(args.sweep, "sweep_report.md")).read())


if __name__ == "__main__":
    main()
