#!/usr/bin/env python3
"""Figures of the manuscript, from the committed campaign outputs.

  python3 paper/make_figures.py --scratch <dir>

<dir> holds the per-run logs (runs/<design>/logs) and the textures
(campaign_tex/, campaign_tex2/); only Figs. 1 and 6 need it. Every other
figure is drawn from confirmatory/, replication/ and sweep/.

  fig_scenes.pdf   occluder geometry on the target; second texture
  fig_maps.pdf     copy of replication/figures/diag_maps_pose1.pdf
  fig_forest.pdf   V3/V2 ratio with exact 95 % CI, every testable cell
  fig_basin.pdf    basin failures by displacement bin, V1-V4
  fig_sweep.pdf    copy of sweep/figures/sweep_success.pdf
  fig_curves.pdf   median camera-centre error against iteration
  fig_budget.pdf   V3/V2 ratio against the iteration budget (exploratory)
"""

import argparse
import glob
import os
import shutil
import sys

import numpy as np
import pandas as pd
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, os.path.join(ROOT, "analysis"))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import analyze as an  # noqa: E402
import make_poses as mp  # noqa: E402

OUT = os.path.join(HERE, "figures")
INK, INK2, GRID = "#0b0b0b", "#52514e", "#e4e3df"
W4 = ["none", "tukey", "hermite", "penalised"]


def fig_scenes(plt, scratch):
    """(a) occluder outlines in target coordinates; (b, c) the second texture."""
    from skimage import measure
    tex = os.path.join(scratch, "campaign_tex")
    fig, axes = plt.subplots(1, 3, figsize=(7.0, 2.25), constrained_layout=True,
                             gridspec_kw={"width_ratios": [1.0, 1.0, 1.0]})
    ax = axes[0]
    hx, hy = mp.TARGET_HALF_X, mp.TARGET_HALF_Y
    ax.add_patch(plt.Rectangle((-hx, -hy), 2 * hx, 2 * hy, fc="#f2f1ee", ec=INK2, lw=0.6))
    masks = [("mask_disc.png", "7.5 %", "-", 1.2), ("uniform_dark_f150_mask.png", "15 %", "--", 0.9),
             ("uniform_dark_f250_mask.png", "25 %", "--", 0.9), ("uniform_dark_f350_mask.png", "35 %", "--", 0.9),
             ("uniform_dark_f450_mask.png", "45 %", "--", 0.9)]
    for k, (f, lab, ls, lw) in enumerate(masks):
        m = np.asarray(Image.open(os.path.join(tex, f)).convert("L")) > 127
        h, w = m.shape
        for c in measure.find_contours(m.astype(float), 0.5):
            X = (c[:, 1] + 0.5) / w * 2 * hx - hx
            Y = (c[:, 0] + 0.5) / h * 2 * hy - hy
            ax.plot(X, Y, color=INK if k == 0 else INK2, ls=ls, lw=lw)
    ax.text(0.135, 0.16, "disc (7.5 %)", ha="left", va="center", fontsize=6, color=INK)
    ax.text(-0.29, -0.195, "dashed: ellipses of 15, 25, 35, 45 %", ha="left", va="center", fontsize=6, color=INK2)
    ax.set_xlim(-hx - 0.01, hx + 0.01)
    ax.set_ylim(hy + 0.01, -hy - 0.01)   # Y downwards, as in the image
    ax.set_aspect("equal")
    ax.set_xlabel("X (m)")
    ax.set_ylabel("Y (m)")
    ax.set_title("(a) occluders on the target")
    ax.grid(False)
    for a, f, t in ((axes[1], "campaign_tex2/coffee.png", "(b) second texture"),
                    (axes[2], "campaign_tex2/coffee_dark_disc.png", "(c) with the dark disc")):
        a.imshow(np.asarray(Image.open(os.path.join(scratch, f)).convert("L")), cmap="gray", vmin=0, vmax=255,
                 extent=(-hx, hx, hy, -hy))
        a.set_title(t)
        a.set_xticks([])
        a.set_yticks([])
        a.grid(False)
    fig.savefig(os.path.join(OUT, "fig_scenes.pdf"))
    plt.close(fig)


def forest_rows():
    """(group, label, interaction, ratio, lo, hi, n, primary)"""
    rows = []
    c = pd.read_csv(os.path.join(ROOT, "confirmatory", "comparisons.csv"))
    names = {"nominal": "clean", "uniform_dark_disc": "dark disc", "textured_disc": "textured disc",
             "low_contrast_disc": "low-contrast disc"}
    for scen in ["nominal", "uniform_dark_disc", "textured_disc", "low_contrast_disc"]:
        for inter in ["current", "desired"]:
            r = c[(c.interaction == inter) & (c.scenario == scen) & (c.A == "hermite") & (c.B == "tukey") &
                  (c.outcome == "final_t_mm")].iloc[0]
            prim = r.family == "primary"
            rows.append(("Thesis texture, 7.5 % disc", names[scen], inter,
                         r.ratio if prim else r.ratio_s, r.ci95_lo if prim else r.ci95_lo_s,
                         r.ci95_hi if prim else r.ci95_hi_s, int(r.n if prim else r.n_s), prim))
    s = pd.read_csv(os.path.join(ROOT, "sweep", "comparisons.csv"))
    for f in ["150", "250", "350"]:
        for inter in ["current", "desired"]:
            r = s[(s.interaction == inter) & (s.scenario == f"uniform_dark_f{f}") & (s.A == "hermite") &
                  (s.B == "tukey") & (s.outcome == "final_t_mm")].iloc[0]
            if np.isfinite(r.ratio):
                rows.append(("Thesis texture, ellipse", f"{int(f) // 10} % occluded", inter,
                             r.ratio, r.ci95_lo, r.ci95_hi, int(r.n), False))
    p = pd.read_csv(os.path.join(ROOT, "replication", "comparisons.csv"))
    for inter in ["current", "desired"]:
        r = p[(p.interaction == inter) & (p.scenario == "nominal") & (p.outcome == "final_t_mm")].iloc[0]
        rows.append(("Second texture", "clean", inter, r.ratio_s, r.ci95_lo_s, r.ci95_hi_s, int(r.n_s), False))
    return rows


def fig_forest(plt):
    rows = forest_rows()
    fig, ax = plt.subplots(figsize=(4.3, 4.3), constrained_layout=True)
    y, ticks, labels, group_y = 0, [], [], []
    last_group = None
    style = {"current": dict(marker="o", mfc=INK, mec=INK, ms=4.5),
             "desired": dict(marker="^", mfc="white", mec=INK, ms=5.0)}
    for g, lab, inter, r, lo, hi, n, prim in rows:
        if g != last_group:
            y -= 0.6 if last_group is not None else 0
            group_y.append((y, g))
            y -= 1.0
            last_group = g
        ax.plot([lo, hi], [y, y], color=INK, lw=1.0, solid_capstyle="butt")
        ax.plot([r], [y], linestyle="none", **style[inter])
        ticks.append(y)
        labels.append(f"{lab}, L {inter}, n={n}" + ("  (P1a)" if prim else ""))
        y -= 1.0
    ax.axvline(1.0, color=INK, lw=0.7)
    ax.axvline(0.8, color=INK2, lw=0.7, ls=":")
    ax.set_yticks(ticks, labels, fontsize=6)
    for t, lab in zip(ax.get_yticklabels(), labels):
        if "(P1a)" in lab:
            t.set_fontweight("bold")
    for gy, g in group_y:
        ax.text(0.205, gy, g, fontsize=6.5, fontweight="bold", va="center", ha="left", color=INK,
                transform=ax.get_yaxis_transform())
    ax.set_xscale("log")
    ax.set_xlim(0.2, 1.3)
    ax.set_xticks([0.25, 0.4, 0.6, 0.8, 1.0, 1.25], ["0.25", "0.4", "0.6", "0.8", "1", "1.25"])
    ax.minorticks_off()
    ax.set_ylim(y + 0.3, 0.6)
    ax.set_xlabel("V3 / V2 camera-centre error\n(Hodges–Lehmann estimate, exact 95 % CI)")
    ax.grid(axis="y", visible=False)
    ax.spines["left"].set_visible(False)
    ax.tick_params(axis="y", length=0)
    fig.savefig(os.path.join(OUT, "fig_forest.pdf"))
    plt.close(fig)


def fig_basin(plt):
    c = pd.read_csv(os.path.join(ROOT, "confirmatory", "runs.csv"))
    p = pd.read_csv(os.path.join(ROOT, "campaign", "poses.csv"))[["pose_id", "bin"]]
    r = c.merge(p, on="pose_id")
    r["basin"] = r.final_r_deg > 5.0
    fig, axes = plt.subplots(1, 2, figsize=(7.0, 2.3), constrained_layout=True, sharey=True)
    bins = [0, 1, 2, 3]
    xs = np.arange(len(bins))
    width = 0.2
    for ax, inter, title in zip(axes, ["current", "desired"],
                                ["L from the current image", "L from the desired image"]):
        for i, w in enumerate(W4):
            g = r[(r.interaction == inter) & (r.weighting == w)]
            cnt = [int(g[g["bin"] == b].basin.sum()) for b in bins]
            x = xs + (i - 1.5) * width
            ax.bar(x, cnt, width * 0.9, color=an.COLORS[w], label=an.LABELS[w], edgecolor="white", linewidth=0.6)
            for xx, cc in zip(x, cnt):
                if cc > 0:
                    ax.text(xx, cc + 0.4, str(cc), ha="center", va="bottom", fontsize=6, color=INK)
        ax.set_xticks(xs, ["20–32", "32–44", "44–56", "56–68"])
        ax.set_xlabel("mean initial image displacement (px)")
        ax.set_title(title)
        ax.grid(axis="x", visible=False)
    axes[0].set_ylabel("basin failures (of 32 runs)")
    axes[0].set_ylim(0, 23)
    axes[1].legend(frameon=False, loc="upper left")
    fig.savefig(os.path.join(OUT, "fig_basin.pdf"))
    plt.close(fig)


def fig_curves(plt, scratch):
    logdir = os.path.join(scratch, "runs", "confirmatory", "logs")
    fig, axes = plt.subplots(1, 2, figsize=(7.0, 2.5), constrained_layout=True, sharey=True)
    for ax, scen, title in zip(axes, ["nominal", "uniform_dark_disc"], ["clean scene", "dark disc (7.5 %)"]):
        for w, wid in zip(W4, ["1", "2", "3", "4"]):
            curves = []
            for f in sorted(glob.glob(os.path.join(logdir, f"L1_{scen}_w{wid}_p*.csv"))):
                d = pd.read_csv(f, comment="#", usecols=["iter", "tx", "ty", "tz", "tux", "tuy", "tuz"])
                r = np.degrees(np.sqrt(d.tux ** 2 + d.tuy ** 2 + d.tuz ** 2))
                if r.iloc[-50:].median() > 5.0:   # basin failures are excluded from the medians
                    continue
                curves.append(1e3 * np.sqrt(d.tx ** 2 + d.ty ** 2 + d.tz ** 2).to_numpy())
            n = min(len(cv) for cv in curves)
            Y = np.array([cv[:n] for cv in curves])
            it = np.arange(1, n + 1)
            q1, med, q3 = np.percentile(Y, [25, 50, 75], axis=0)
            ax.fill_between(it, q1, q3, color=an.COLORS[w], alpha=0.15, linewidth=0)
            ax.plot(it, med, color=an.COLORS[w], linestyle=an.STYLES[w], label=f"{an.LABELS[w]} ({len(curves)})")
        ax.set_yscale("log")
        ax.set_xscale("log")
        ax.set_xlabel("iteration")
        ax.set_title(f"L from the desired image, {title}")
        ax.set_xlim(1, 600)
    axes[0].set_ylabel("camera-centre error (mm)")
    axes[0].legend(frameon=False, loc="lower left", fontsize=6)
    fig.savefig(os.path.join(OUT, "fig_curves.pdf"))
    plt.close(fig)


def fig_budget(plt):
    """Median per-pose V3/V2 ratio at iteration k, confirmatory cells."""
    b = pd.read_csv(os.path.join(ROOT, "confirmatory", "budget_ratio.csv"))
    ks = [100, 200, 300, 400, 500, 550, 600]
    scen = [("nominal", "clean", "-", "o"), ("uniform_dark_disc", "dark disc", "--", "s"),
            ("textured_disc", "textured disc", "-.", "^"), ("low_contrast_disc", "low-contrast disc", ":", "D")]
    fig, axes = plt.subplots(1, 2, figsize=(7.0, 2.5), constrained_layout=True, sharey=True)
    for ax, inter, title in zip(axes, ["current", "desired"],
                                ["L from the current image", "L from the desired image"]):
        for sc, lab, ls, mk in scen:
            r = b[(b.interaction == inter) & (b.scenario == sc)].iloc[0]
            y = [r[f"ratio_k{k}"] for k in ks]
            ax.plot(ks, y, color=INK, ls=ls, marker=mk, ms=4, mfc="white", mec=INK, lw=1.2)
            if sc == "textured_disc":
                continue   # coincides with the dark disc; one label for both
            if sc == "uniform_dark_disc":
                lab = "dark, textured discs"
            ax.text(612, y[-1], lab, fontsize=6, va="center", ha="left", color=INK)
        ax.axhline(1.0, color=INK2, lw=0.6)
        ax.set_xlim(80, 720)
        ax.set_xticks([100, 200, 300, 400, 500, 600])
        ax.set_xlabel("iteration budget k")
        ax.set_title(title)
    axes[0].set_ylabel("median V3 / V2 camera-centre error")
    axes[0].set_ylim(0, 1.05)
    fig.savefig(os.path.join(OUT, "fig_budget.pdf"))
    plt.close(fig)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--scratch", default=None)
    args = ap.parse_args(argv)
    os.makedirs(OUT, exist_ok=True)
    plt = an.setup_mpl()
    shutil.copy(os.path.join(ROOT, "replication", "figures", "diag_maps_pose1.pdf"),
                os.path.join(OUT, "fig_maps.pdf"))
    shutil.copy(os.path.join(ROOT, "sweep", "figures", "sweep_success.pdf"), os.path.join(OUT, "fig_sweep.pdf"))
    fig_forest(plt)
    fig_basin(plt)
    fig_budget(plt)
    if args.scratch:
        fig_scenes(plt, args.scratch)
        fig_curves(plt, args.scratch)


if __name__ == "__main__":
    main()
