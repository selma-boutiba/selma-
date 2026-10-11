#!/usr/bin/env python3
"""Statistical analysis of the simulation campaign.

Reads the per-iteration logs written by campaign::Logger (one CSV per run,
"# key=value" metadata lines followed by a header), computes per-run metrics,
compares weightings pairwise over the same initial poses, and writes tables,
a Markdown report and figures.

  analyze.py LOGDIR --poses poses.csv --out results/

Statistical plan (fixed before the confirmatory runs; see PLAN.md):
  unit          initial pose; every comparison is paired over the poses
                common to both weightings within one (interaction, scenario)
  outcomes      final camera-centre position error (mm, from cdMc) and
                rotation error (deg), each the median over the last K
                iterations; the thesis' target-frame translation is reported
                alongside (final_tobj_mm) but is not an outcome
  effect        Hodges-Lehmann estimate of the median log-ratio A/B,
                back-transformed, with its exact distribution-free CI
  test          Wilcoxon signed-rank (exact when possible), two-sided
  multiplicity  Holm within the pre-registered primary family (--primary),
                and separately within the remaining, secondary comparisons
  equivalence   the (--equiv-conf) CI of the ratio inside [1/1.25, 1.25];
                0.95 is TOST at 5 % per outcome, Bonferroni over two outcomes
  success       paired success indicators, exact McNemar test
"""

import argparse
import glob
import math
import os
import sys
from itertools import combinations

import numpy as np
import pandas as pd
from scipy import stats

WEIGHTINGS = ["none", "tukey", "hermite", "penalised", "adaptive"]
LABELS = {"none": "V1 none", "tukey": "V2 Tukey", "hermite": "V3 Hermite-informed",
          "penalised": "V4 structure-penalised", "adaptive": "V5 adaptive floor"}
# Fixed slot per weighting (dataviz reference palette, validated light mode);
# line style and marker carry identity in greyscale and for CVD readers.
COLORS = {"none": "#2a78d6", "tukey": "#eb6834", "hermite": "#1baf7a",
          "penalised": "#eda100", "adaptive": "#e87ba4"}
STYLES = {"none": "-", "tukey": "--", "hermite": "-.", "penalised": ":", "adaptive": (0, (5, 1, 1, 1))}
MARKERS = {"none": "o", "tukey": "s", "hermite": "^", "penalised": "D", "adaptive": "v"}

# Comparisons of the primary family: (A, B) reports the ratio A / B.
PRIMARY = [("tukey", "none"), ("hermite", "tukey"), ("penalised", "tukey"),
           ("adaptive", "tukey"), ("hermite", "penalised")]
EQUIV_MARGIN = 1.25
# Provisional primary family (interaction, scenario, A, B); fix it after the
# pilot and before the confirmatory runs, then do not change it.
DEFAULT_PRIMARY = "desired:uniform_dark_f070:hermite:tukey,desired:uniform_dark_f070:adaptive:tukey"


# ---------------------------------------------------------------------------
#  Reading
# ---------------------------------------------------------------------------
def read_run(path):
    meta, skip = {}, 0
    with open(path) as fh:
        for line in fh:
            if not line.startswith("#"):
                break
            k, _, v = line[1:].strip().partition("=")
            meta[k.strip()] = v.strip()
            skip += 1
    df = pd.read_csv(path, skiprows=skip)
    meta["file"] = os.path.basename(path)
    return meta, df


def scenario_of(meta):
    ref = os.path.basename(meta.get("ref_texture", ""))
    servo = os.path.basename(meta.get("servo_texture", ""))
    return "nominal" if servo == ref else os.path.splitext(servo)[0]


# ---------------------------------------------------------------------------
#  Per-run metrics
# ---------------------------------------------------------------------------
def tail_slope(it, y, start):
    m = (it >= start) & (y > 0) & np.isfinite(y)
    if m.sum() < 10:
        return np.nan
    return float(np.polyfit(np.log10(it[m]), np.log10(y[m]), 1)[0])


def run_metrics(meta, df, k_final, slope_start, t_thr, r_thr, v_thr):
    it = df["iter"].to_numpy(float)
    t = np.sqrt(df["tx"] ** 2 + df["ty"] ** 2 + df["tz"] ** 2).to_numpy() * 1e3          # mm, camera centre
    has_o = "ox" in df.columns
    to = (np.sqrt(df["ox"] ** 2 + df["oy"] ** 2 + df["oz"] ** 2).to_numpy() * 1e3) if has_o else np.full(len(df), np.nan)
    r = np.degrees(np.sqrt(df["tux"] ** 2 + df["tuy"] ** 2 + df["tuz"] ** 2).to_numpy())  # deg
    v = np.sqrt(df["vx"] ** 2 + df["vy"] ** 2 + df["vz"] ** 2).to_numpy()                 # m/s
    k = min(k_final, len(df))
    finite = np.isfinite(t).all() and np.isfinite(r).all()
    ft = float(np.median(t[-k:])) if finite else np.inf
    fr = float(np.median(r[-k:])) if finite else np.inf
    below = np.nonzero(v < v_thr)[0]
    margin = (df["h_min"] / (df["h_trace"] / 6.0)).to_numpy()
    tail = slice(max(0, len(df) - 100), len(df))
    return {
        "file": meta["file"],
        "weighting": meta.get("weighting"),
        "interaction": meta.get("interaction"),
        "scenario": scenario_of(meta),
        "pose_id": int(meta.get("pose_id", -1)),
        "n_iter": int(len(df)),
        "final_t_mm": ft,
        "final_r_deg": fr,
        "final_tobj_mm": float(np.median(to[-k:])),   # thesis convention, target origin
        "last_t_mm": float(t[-1]),
        "last_r_deg": float(r[-1]),
        "final_e2": float(np.median(df["e2"].to_numpy()[-k:])),
        "success": bool(ft < t_thr and fr < r_thr),
        "iters_to_v": int(it[below[0]]) if below.size else np.nan,
        "v_tail": float(np.median(v[tail])),
        "jitter_t_um": float(np.std(t[tail]) * 1e3),
        "slope_t": tail_slope(it, t, slope_start),
        "slope_r": tail_slope(it, r, slope_start),
        "rank_margin": float(np.nanmin(margin)) if len(margin) else np.nan,
        "dW_tail": float(np.median(df["dW"].to_numpy()[tail])),
        "frac_w25_first": float(df["frac_w25"].iloc[0]),
        "frac_w25_last": float(df["frac_w25"].iloc[-1]),
        "t_weight_ms": float(np.median(df["t_weight_ms"])),
    }


# ---------------------------------------------------------------------------
#  Paired statistics
# ---------------------------------------------------------------------------
def signed_rank_null(n):
    """Exact null distribution of the Wilcoxon signed-rank statistic T+."""
    m = n * (n + 1) // 2
    c = np.zeros(m + 1)
    c[0] = 1.0
    for i in range(1, n + 1):
        c[i:] = c[i:] + c[:-i].copy() if i <= m else c[i:]
    return c / 2.0 ** n


def hodges_lehmann(d, conf):
    """HL estimate of the pseudo-median and its exact CI (Walsh averages)."""
    d = np.asarray(d, float)
    n = len(d)
    i, j = np.triu_indices(n)
    walsh = np.sort((d[i] + d[j]) / 2.0)
    est = float(np.median(walsh))
    cdf = np.cumsum(signed_rank_null(n))
    alpha = 1.0 - conf
    c = int(np.searchsorted(cdf, alpha / 2.0, side="right")) - 1  # largest c: P(T<=c) <= a/2
    if c < 0:
        return est, -np.inf, np.inf
    m = len(walsh)
    return est, float(walsh[c]), float(walsh[m - c - 1])


def holm(p):
    p = np.asarray(p, float)
    order = np.argsort(p)
    adj = np.empty_like(p)
    running = 0.0
    for rank, idx in enumerate(order):
        running = max(running, (len(p) - rank) * p[idx])
        adj[idx] = min(1.0, running)
    return adj


def compare(runs, a, b, outcome, equiv_conf=0.95):
    """Paired comparison of weightings a and b over common poses."""
    A = runs[runs.weighting == a].set_index("pose_id")[outcome]
    B = runs[runs.weighting == b].set_index("pose_id")[outcome]
    common = A.index.intersection(B.index)
    x, y = A.loc[common].to_numpy(float), B.loc[common].to_numpy(float)
    ok = np.isfinite(x) & np.isfinite(y) & (x > 0) & (y > 0)
    n = int(ok.sum())
    res = {"A": a, "B": b, "outcome": outcome, "n": n}
    if n < 6:
        return {**res, "ratio": np.nan, "ci95_lo": np.nan, "ci95_hi": np.nan,
                "eq_lo": np.nan, "eq_hi": np.nan, "p": np.nan, "frac_A_better": np.nan}
    d = np.log(x[ok] / y[ok])
    est, lo95, hi95 = hodges_lehmann(d, 0.95)
    _, lo_eq, hi_eq = hodges_lehmann(d, equiv_conf)
    method = "exact" if n <= 50 and np.count_nonzero(d) == n and len(np.unique(np.abs(d))) == n else "approx"
    p = float(stats.wilcoxon(d, alternative="two-sided", method=method).pvalue) if np.any(d != 0) else 1.0
    return {**res, "ratio": math.exp(est), "ci95_lo": math.exp(lo95), "ci95_hi": math.exp(hi95),
            "eq_lo": math.exp(lo_eq), "eq_hi": math.exp(hi_eq), "p": p,
            "frac_A_better": float(np.mean(d < 0))}


def mcnemar(runs, a, b):
    A = runs[runs.weighting == a].set_index("pose_id")["success"]
    B = runs[runs.weighting == b].set_index("pose_id")["success"]
    common = A.index.intersection(B.index)
    x, y = A.loc[common].to_numpy(bool), B.loc[common].to_numpy(bool)
    b10, b01 = int(np.sum(x & ~y)), int(np.sum(~x & y))
    p = float(stats.binomtest(b10, b10 + b01, 0.5).pvalue) if b10 + b01 else 1.0
    return {"A": a, "B": b, "A_only": b10, "B_only": b01, "p_mcnemar": p}


def decoupling(runs):
    """Kendall tau, within each pose, between final ||e||^2 and the final
    pose error across weightings (does the cost rank the accuracy?)."""
    taus = []
    for _, g in runs.groupby("pose_id"):
        if g.weighting.nunique() >= 3 and np.isfinite(g.final_t_mm).all():
            taus.append(stats.kendalltau(g.final_e2, g.final_t_mm).statistic)
    taus = np.array([t for t in taus if np.isfinite(t)])
    if len(taus) == 0:
        return {"n": 0, "median_tau": np.nan, "frac_negative": np.nan}
    return {"n": int(len(taus)), "median_tau": float(np.median(taus)),
            "frac_negative": float(np.mean(taus < 0))}


# ---------------------------------------------------------------------------
#  Figures
# ---------------------------------------------------------------------------
def setup_mpl():
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    plt.rcParams.update({
        "font.size": 8, "axes.titlesize": 8, "axes.labelsize": 8, "legend.fontsize": 7,
        "xtick.labelsize": 7, "ytick.labelsize": 7, "axes.edgecolor": "#52514e",
        "axes.labelcolor": "#0b0b0b", "xtick.color": "#52514e", "ytick.color": "#52514e",
        "axes.grid": True, "grid.color": "#e4e3df", "grid.linewidth": 0.5,
        "axes.spines.top": False, "axes.spines.right": False, "lines.linewidth": 1.4,
        "pdf.fonttype": 42, "savefig.dpi": 300, "figure.facecolor": "white",
    })
    return plt


def fig_ecdf(plt, runs, path, title):
    fig, axes = plt.subplots(1, 2, figsize=(7.0, 2.4), constrained_layout=True)
    for ax, col, xl in zip(axes, ["final_t_mm", "final_r_deg"],
                           ["final camera-centre error (mm)", "final rotation error (deg)"]):
        for w in WEIGHTINGS:
            x = np.sort(runs.loc[runs.weighting == w, col].to_numpy(float))
            if len(x) == 0:
                continue
            y = np.arange(1, len(x) + 1) / len(x)
            ax.step(x, y, where="post", color=COLORS[w], linestyle=STYLES[w], label=LABELS[w])
            ax.plot(x[len(x) // 2], y[len(x) // 2], MARKERS[w], color=COLORS[w], ms=4,
                    markeredgecolor="white", markeredgewidth=0.6)
        ax.set_xscale("log")
        ax.set_xlabel(xl)
        ax.set_ylim(0, 1.02)
    axes[0].set_ylabel("fraction of initial poses")
    axes[1].legend(loc="lower right", frameon=False)
    fig.suptitle(title, fontsize=8)
    fig.savefig(path)
    plt.close(fig)


def fig_curves(plt, logs, path, title, col="t"):
    fig, ax = plt.subplots(figsize=(3.45, 2.4), constrained_layout=True)
    for w in WEIGHTINGS:
        curves = [d for m, d in logs if m.get("weighting") == w]
        if not curves:
            continue
        n = min(len(d) for d in curves)
        if col == "t":
            Y = np.array([np.sqrt(d.tx[:n] ** 2 + d.ty[:n] ** 2 + d.tz[:n] ** 2) * 1e3 for d in curves])
        else:
            Y = np.array([np.degrees(np.sqrt(d.tux[:n] ** 2 + d.tuy[:n] ** 2 + d.tuz[:n] ** 2)) for d in curves])
        it = curves[0]["iter"].to_numpy()[:n]
        q1, med, q3 = np.nanpercentile(Y, [25, 50, 75], axis=0)
        ax.fill_between(it, q1, q3, color=COLORS[w], alpha=0.15, linewidth=0)
        ax.plot(it, med, color=COLORS[w], linestyle=STYLES[w], label=LABELS[w])
    ax.set_yscale("log")
    from matplotlib.ticker import FuncFormatter, LogLocator
    ax.yaxis.set_major_locator(LogLocator(base=10, subs=(1.0, 2.0, 5.0)))
    ax.yaxis.set_major_formatter(FuncFormatter(lambda y, _: f"{y:g}"))
    ax.yaxis.set_minor_formatter(FuncFormatter(lambda y, _: ""))
    ax.set_xlabel("iteration")
    ax.set_ylabel("camera-centre error (mm)" if col == "t" else "rotation error (deg)")
    ax.legend(frameon=False, loc="lower left", fontsize=6)
    ax.set_title(title)
    fig.savefig(path)
    plt.close(fig)


def fig_success_by_bin(plt, runs, poses, path, title):
    if poses is None:
        return
    r = runs.merge(poses[["pose_id", "bin", "mean_disp_px"]], on="pose_id", how="left")
    bins = sorted(r["bin"].dropna().unique())
    fig, ax = plt.subplots(figsize=(3.45, 2.6), constrained_layout=True)
    xs = np.arange(len(bins))
    present = [w for w in WEIGHTINGS if (r.weighting == w).any()]
    width = 0.8 / max(1, len(present))
    for i, w in enumerate(present):
        rate = [r[(r.weighting == w) & (r["bin"] == b)].success.mean() for b in bins]
        ax.bar(xs + (i - (len(present) - 1) / 2) * width, rate, width * 0.92, color=COLORS[w],
               label=LABELS[w], edgecolor="white", linewidth=0.6)
    centres = [r[r["bin"] == b].mean_disp_px.mean() for b in bins]
    ax.set_xticks(xs, [f"{c:.0f} px" for c in centres])
    ax.set_xlabel("mean image displacement of the bin")
    ax.set_ylabel("success rate")
    ax.set_ylim(0, 1.05)
    ax.legend(frameon=False, fontsize=6, loc="upper center", bbox_to_anchor=(0.5, -0.3), ncol=3)
    ax.set_title(title)
    fig.savefig(path)
    plt.close(fig)


# ---------------------------------------------------------------------------
#  Report
# ---------------------------------------------------------------------------
def fmt_ratio(row):
    if not np.isfinite(row["ratio"]):
        return "n/a"
    return f"{row['ratio']:.3g} [{row['ci95_lo']:.3g}, {row['ci95_hi']:.3g}]"


def latex_summary(summary):
    lines = [r"\begin{tabular}{@{}lllrrrrr@{}}", r"\toprule",
             r"Interaction & Scenario & Weighting & $n$ & $\lVert\Delta\mathbf{t}\rVert$ (mm) & "
             r"$\lVert\Delta\boldsymbol{\theta}\rVert$ ($^\circ$) & Success & Tail slope \\",
             r"\midrule"]
    for _, s in summary.iterrows():
        lines.append(
            f"{s.interaction} & {s.scenario.replace('_', ' ')} & {LABELS.get(s.weighting, s.weighting)} & "
            f"{s.n} & {s.t_med:.3g} [{s.t_q1:.3g}, {s.t_q3:.3g}] & "
            f"{s.r_med:.3g} [{s.r_q1:.3g}, {s.r_q3:.3g}] & {100 * s.success:.0f}\\% & {s.slope_t:.2f} \\\\")
    lines += [r"\bottomrule", r"\end{tabular}"]
    return "\n".join(lines)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("logdir")
    ap.add_argument("--poses", default=None)
    ap.add_argument("--out", default="results")
    ap.add_argument("--k-final", type=int, default=50)
    ap.add_argument("--slope-start", type=int, default=200)
    ap.add_argument("--t-thr", type=float, default=20.0,
                    help="success: camera-centre error (mm); fix after the pilot")
    ap.add_argument("--r-thr", type=float, default=1.0, help="success: rotation (deg); fix after the pilot")
    ap.add_argument("--v-thr", type=float, default=1e-3, help="convergence: ||v|| (m/s)")
    ap.add_argument("--primary", default=DEFAULT_PRIMARY,
                    help="comma-separated interaction:scenario:A:B entries of the primary family")
    ap.add_argument("--equiv-conf", type=float, default=0.95)
    ap.add_argument("--no-figures", action="store_true")
    args = ap.parse_args(argv)

    files = sorted(glob.glob(os.path.join(args.logdir, "*.csv")))
    if not files:
        sys.exit(f"no CSV logs in {args.logdir}")
    logs = [read_run(f) for f in files]
    runs = pd.DataFrame([run_metrics(m, d, args.k_final, args.slope_start,
                                     args.t_thr, args.r_thr, args.v_thr) for m, d in logs])
    poses = pd.read_csv(args.poses) if args.poses else None
    os.makedirs(args.out, exist_ok=True)
    runs.to_csv(os.path.join(args.out, "runs.csv"), index=False)

    # Summary per cell
    rows = []
    for (inter, scen, w), g in runs.groupby(["interaction", "scenario", "weighting"]):
        rows.append({"interaction": inter, "scenario": scen, "weighting": w, "n": len(g),
                     "t_med": g.final_t_mm.median(), "t_q1": g.final_t_mm.quantile(0.25),
                     "t_q3": g.final_t_mm.quantile(0.75), "t_max": g.final_t_mm.max(),
                     "r_med": g.final_r_deg.median(), "r_q1": g.final_r_deg.quantile(0.25),
                     "r_q3": g.final_r_deg.quantile(0.75), "r_max": g.final_r_deg.max(),
                     "success": g.success.mean(), "slope_t": g.slope_t.median(),
                     "jitter_t_um": g.jitter_t_um.median(), "rank_margin_min": g.rank_margin.min(),
                     "frac_w25_first": g.frac_w25_first.median(), "t_weight_ms": g.t_weight_ms.median()})
    order = {w: i for i, w in enumerate(WEIGHTINGS)}
    summary = pd.DataFrame(rows).sort_values(
        ["interaction", "scenario", "weighting"], key=lambda c: c.map(order) if c.name == "weighting" else c)
    summary.to_csv(os.path.join(args.out, "summary.csv"), index=False)

    # Paired comparisons, Holm over the whole family
    primary = {tuple(e.split(":")) for e in args.primary.split(",") if e}
    comps, mcn, dec = [], [], []
    for (inter, scen), g in runs.groupby(["interaction", "scenario"]):
        present = set(g.weighting)
        for a, b in PRIMARY:
            if a in present and b in present:
                fam = "primary" if (inter, scen, a, b) in primary else "secondary"
                for outcome in ("final_t_mm", "final_r_deg"):
                    comps.append({"interaction": inter, "scenario": scen, "family": fam,
                                  **compare(g, a, b, outcome, args.equiv_conf)})
                mcn.append({"interaction": inter, "scenario": scen, **mcnemar(g, a, b)})
        dec.append({"interaction": inter, "scenario": scen, **decoupling(g)})
    comps = pd.DataFrame(comps)
    if len(comps):
        comps["p_holm"] = np.nan
        for fam in ("primary", "secondary"):
            sel = comps.p.notna() & (comps.family == fam)
            if sel.any():
                comps.loc[sel, "p_holm"] = holm(comps.loc[sel, "p"].to_numpy())
        comps["equivalent"] = (comps.eq_lo >= 1 / EQUIV_MARGIN) & (comps.eq_hi <= EQUIV_MARGIN)
        comps = comps.sort_values(["family", "interaction", "scenario"])
    comps.to_csv(os.path.join(args.out, "comparisons.csv"), index=False)
    pd.DataFrame(mcn).to_csv(os.path.join(args.out, "mcnemar.csv"), index=False)
    pd.DataFrame(dec).to_csv(os.path.join(args.out, "decoupling.csv"), index=False)
    with open(os.path.join(args.out, "summary_table.tex"), "w") as fh:
        fh.write(latex_summary(summary) + "\n")

    # Markdown report
    with open(os.path.join(args.out, "report.md"), "w") as fh:
        fh.write(f"# Campaign results\n\n{len(runs)} runs, {runs.pose_id.nunique()} poses.\n\n")
        fh.write("## Final pose error per cell (median [IQR])\n\n")
        fh.write("| interaction | scenario | weighting | n | translation (mm) | rotation (deg) | success | tail slope |\n")
        fh.write("|---|---|---|---|---|---|---|---|\n")
        for _, s in summary.iterrows():
            fh.write(f"| {s.interaction} | {s.scenario} | {s.weighting} | {s.n} | "
                     f"{s.t_med:.3g} [{s.t_q1:.3g}, {s.t_q3:.3g}] | {s.r_med:.3g} [{s.r_q1:.3g}, {s.r_q3:.3g}] | "
                     f"{100 * s.success:.0f} % | {s.slope_t:.2f} |\n")
        fh.write("\n## Paired comparisons (ratio A/B, HL estimate [95 % CI]; Holm-adjusted p within family)\n\n")
        fh.write("| family | interaction | scenario | A / B | outcome | n | ratio [95 % CI] | p (Holm) | equivalent (±25 %) |\n")
        fh.write("|---|---|---|---|---|---|---|---|---|\n")
        for _, c in comps.iterrows():
            fh.write(f"| {c.family} | {c.interaction} | {c.scenario} | {c.A} / {c.B} | {c.outcome} | {c.n} | "
                     f"{fmt_ratio(c)} | {c.p_holm:.3g} | {'yes' if c.equivalent else 'no'} |\n")
        fh.write("\n## Does the final cost rank the accuracy? (Kendall tau within pose)\n\n")
        for d in dec:
            fh.write(f"- {d['interaction']}, {d['scenario']}: median tau {d['median_tau']:.2f}, "
                     f"negative in {100 * d['frac_negative']:.0f} % of {d['n']} poses\n")

    if not args.no_figures:
        plt = setup_mpl()
        fdir = os.path.join(args.out, "figures")
        os.makedirs(fdir, exist_ok=True)
        for (inter, scen), g in runs.groupby(["interaction", "scenario"]):
            tag = f"{inter}_{scen}"
            fig_ecdf(plt, g, os.path.join(fdir, f"ecdf_{tag}.pdf"), f"{scen}, L from the {inter} image")
            sel = [(m, d) for m, d in logs if m.get("interaction") == inter and scenario_of(m) == scen]
            fig_curves(plt, sel, os.path.join(fdir, f"curves_{tag}.pdf"), f"{scen}, {inter} L")
            fig_success_by_bin(plt, g, poses, os.path.join(fdir, f"success_{tag}.pdf"), f"{scen}, {inter} L")

    print(f"{len(runs)} runs analysed; results in {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
