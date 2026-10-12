#!/usr/bin/env python3
"""Occluded target textures with a prescribed image coverage, and exact
coverage measurement from the occluder mask.

The texture is mapped by vpImageSimulator onto the target rectangle
|X| <= 0.30 m, |Y| <= 0.215 m (texture row 0 at Y = -0.215). Coverage is the
fraction of the 300 x 220 interior positions whose ray meets the occluder,
computed by back-projecting every interior pixel onto the target plane. This
replaces measuring the occluder as "the region where G vanishes", which is
the occluder eroded by the 4-pixel kernel half-width (item A6).

Subcommands
  generate  write an occluded texture, its mask and a JSON record
  measure   derive the mask of an existing occluded texture from its clean
            counterpart and report coverage at the desired pose, the
            Chapter 4 pose and every pose of a pose file
  refill    keep the geometry of an existing occluder and change its
            appearance (dark, bright, low contrast, textured)

Occluder kinds
  uniform_dark    grey level 0 (the configuration of Chapter 4)
  uniform_bright  grey level 255
  low_contrast    the mean grey level of the texture it covers
  textured        a patch of the same texture taken half a texture away,
                  so local structure is preserved and content is wrong
"""

import argparse
import csv
import json
import math
import os
import sys

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import make_poses as mp  # noqa: E402

KERNEL_HALF_WIDTH = 4  # 9 x 9 Hermite kernels


def load_grey(path):
    return np.asarray(Image.open(path).convert("L"), dtype=np.float64)


def target_to_texture(X, Y, shape):
    h, w = shape
    c = (X + mp.TARGET_HALF_X) / (2 * mp.TARGET_HALF_X) * w - 0.5
    r = (Y + mp.TARGET_HALF_Y) / (2 * mp.TARGET_HALF_Y) * h - 0.5
    return c, r


def texture_grid(shape):
    """Target-plane coordinates of every texture pixel centre."""
    h, w = shape
    c, r = np.meshgrid(np.arange(w), np.arange(h))
    X = (c + 0.5) / w * (2 * mp.TARGET_HALF_X) - mp.TARGET_HALF_X
    Y = (r + 0.5) / h * (2 * mp.TARGET_HALF_Y) - mp.TARGET_HALF_Y
    return X, Y


def ellipse_mask(shape, centre, semi_axes, angle_deg=0.0):
    X, Y = texture_grid(shape)
    ca, sa = math.cos(math.radians(angle_deg)), math.sin(math.radians(angle_deg))
    dx, dy = X - centre[0], Y - centre[1]
    xr, yr = ca * dx + sa * dy, -sa * dx + ca * dy
    return (xr / semi_axes[0]) ** 2 + (yr / semi_axes[1]) ** 2 <= 1.0


def image_mask(mask_tex, pose):
    """Occluder mask over the interior pixels seen from `pose` (2-D, 220 x 300)."""
    R, t = mp.pose_matrix(pose)
    u, v = mp.interior_pixels()
    pts, front = mp.backproject_to_target(R, t, u, v)
    c, r = target_to_texture(pts[:, 0], pts[:, 1], mask_tex.shape)
    ci = np.clip(np.rint(c).astype(int), 0, mask_tex.shape[1] - 1)
    ri = np.clip(np.rint(r).astype(int), 0, mask_tex.shape[0] - 1)
    inside = front & (c >= -0.5) & (c <= mask_tex.shape[1] - 0.5) & \
        (r >= -0.5) & (r <= mask_tex.shape[0] - 0.5)
    m = inside & mask_tex[ri, ci]
    return m.reshape(mp.HEIGHT - 2 * mp.BORDER, mp.WIDTH - 2 * mp.BORDER)


def morph(mask, radius, grow):
    """Binary dilation (grow=True) or erosion by a (2r+1) x (2r+1) square,
    the support of the 9 x 9 kernels."""
    m = mask.copy()
    pad = np.pad(m, radius, constant_values=not grow)
    out = np.zeros_like(m) if grow else np.ones_like(m)
    for di in range(-radius, radius + 1):
        for dj in range(-radius, radius + 1):
            sl = pad[radius + di: radius + di + m.shape[0], radius + dj: radius + dj + m.shape[1]]
            out = (out | sl) if grow else (out & sl)
    return out


def coverage_report(mask_tex, pose):
    m = image_mask(mask_tex, pose)
    n = m.size
    return {
        "occluded": m.sum() / n,
        "affected_dilated": morph(m, KERNEL_HALF_WIDTH, True).sum() / n,
        "G_zero_eroded": morph(m, KERNEL_HALF_WIDTH, False).sum() / n,
        "equiv_radius_px": math.sqrt(m.sum() / math.pi),
    }


DESIRED_POSE = (0.0, 0.0, mp.Z_DESIRED, 0.0, 0.0, 0.0)


def fit_fraction(shape, centre, aspect, angle, fraction):
    """Bisection on the semi-major axis so that the desired-pose coverage
    equals `fraction`."""
    lo, hi = 1e-3, 0.5
    for _ in range(50):
        a = 0.5 * (lo + hi)
        m = ellipse_mask(shape, centre, (a, a / aspect), angle)
        if image_mask(m, DESIRED_POSE).mean() < fraction:
            lo = a
        else:
            hi = a
    a = 0.5 * (lo + hi)
    return a, a / aspect


def fill(tex, mask, kind):
    out = tex.copy()
    if kind == "uniform_dark":
        out[mask] = 0.0
    elif kind == "uniform_bright":
        out[mask] = 255.0
    elif kind == "low_contrast":
        out[mask] = round(float(tex[mask].mean()))
    elif kind == "textured":
        h, w = tex.shape
        shifted = np.roll(np.roll(tex, h // 2, axis=0), w // 2, axis=1)
        out[mask] = shifted[mask]
    else:
        raise ValueError(kind)
    return out


def read_poses(path):
    with open(path) as fh:
        rows = list(csv.DictReader(fh))
    return [(int(r["pose_id"]), tuple(float(r[k]) for k in
            ("tx_m", "ty_m", "tz_m", "tux_deg", "tuy_deg", "tuz_deg"))) for r in rows]


def report(mask_tex, poses_path):
    lines = []
    for name, pose in (("desired", DESIRED_POSE), ("chapter4", mp.CHAPTER4_POSE)):
        c = coverage_report(mask_tex, pose)
        lines.append((name, c))
    if poses_path:
        for pid, pose in read_poses(poses_path):
            lines.append((f"pose{pid}", coverage_report(mask_tex, pose)))
    print(f"{'pose':>9s} {'occluded':>9s} {'dilated':>9s} {'eroded':>9s} {'r_eq(px)':>9s}")
    for name, c in lines:
        print(f"{name:>9s} {100 * c['occluded']:8.2f}% {100 * c['affected_dilated']:8.2f}% "
              f"{100 * c['G_zero_eroded']:8.2f}% {c['equiv_radius_px']:9.1f}")
    return lines


def cmd_generate(args):
    tex = load_grey(args.texture)
    centre = tuple(float(x) for x in args.centre.split(","))
    a, b = fit_fraction(tex.shape, centre, args.aspect, args.angle, args.fraction)
    mask = ellipse_mask(tex.shape, centre, (a, b), args.angle)
    out = fill(tex, mask, args.kind)
    os.makedirs(args.out_dir, exist_ok=True)
    stem = f"{args.kind}_f{int(round(1000 * args.fraction)):03d}"
    Image.fromarray(np.clip(out, 0, 255).astype(np.uint8)).save(os.path.join(args.out_dir, stem + ".png"))
    Image.fromarray((mask * 255).astype(np.uint8)).save(os.path.join(args.out_dir, stem + "_mask.png"))
    rec = {"kind": args.kind, "fraction_desired": args.fraction, "centre_m": centre,
           "semi_axes_m": [a, b], "angle_deg": args.angle, "texture": os.path.basename(args.texture)}
    if args.kind == "low_contrast":
        rec["fill_grey"] = round(float(tex[mask].mean()))
    with open(os.path.join(args.out_dir, stem + ".json"), "w") as fh:
        json.dump(rec, fh, indent=2)
    print(f"{stem}: semi-axes {a:.4f} x {b:.4f} m")
    report(mask, args.poses)


def hull_mask(clean, occ, threshold=10.0):
    """Mask of a convex occluder: filled convex hull of the largest region
    where the two textures differ. Robust where the covered texture is
    itself dark, which a plain threshold on the difference is not."""
    from matplotlib.path import Path
    from scipy import ndimage
    from scipy.spatial import ConvexHull
    d = np.abs(occ - clean) > threshold
    lab, n = ndimage.label(d)
    if n == 0:
        sys.exit("the textures do not differ")
    sizes = ndimage.sum(d, lab, range(1, n + 1))
    ys, xs = np.nonzero(lab == 1 + int(np.argmax(sizes)))
    pts = np.c_[xs, ys]
    hull = Path(pts[ConvexHull(pts).vertices])
    yy, xx = np.mgrid[0:clean.shape[0], 0:clean.shape[1]]
    return hull.contains_points(np.c_[xx.ravel(), yy.ravel()], radius=0.5).reshape(clean.shape)


def transfer_mask(mask, shape):
    """The same occluder, in target coordinates, on a texture of another size."""
    X, Y = texture_grid(shape)
    c, r = target_to_texture(X, Y, mask.shape)
    ci = np.clip(np.rint(c).astype(int), 0, mask.shape[1] - 1)
    ri = np.clip(np.rint(r).astype(int), 0, mask.shape[0] - 1)
    return mask[ri, ci]


def cmd_refill(args):
    clean, occ = load_grey(args.clean), load_grey(args.occluded)
    mask = hull_mask(clean, occ, args.threshold)
    if args.onto:  # keep the geometry, change the texture
        clean = load_grey(args.onto)
        mask = transfer_mask(mask, clean.shape)
    os.makedirs(args.out_dir, exist_ok=True)
    for kind in args.kinds.split(","):
        out = fill(clean, mask, kind)
        stem = f"{kind}_{args.tag}"
        Image.fromarray(np.clip(out, 0, 255).astype(np.uint8)).save(os.path.join(args.out_dir, stem + ".png"))
        print(f"{stem}.png written")
    Image.fromarray((mask * 255).astype(np.uint8)).save(os.path.join(args.out_dir, f"mask_{args.tag}.png"))
    report(mask, args.poses)


def cmd_measure(args):
    clean, occ = load_grey(args.clean), load_grey(args.occluded)
    if clean.shape != occ.shape:
        sys.exit("clean and occluded textures differ in size")
    mask = hull_mask(clean, occ, args.threshold)
    print(f"mask: {mask.sum()} texture pixels ({100 * mask.mean():.2f} % of the texture)")
    report(mask, args.poses)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    g = sub.add_parser("generate")
    g.add_argument("--texture", required=True)
    g.add_argument("--kind", required=True,
                   choices=["uniform_dark", "uniform_bright", "low_contrast", "textured"])
    g.add_argument("--fraction", type=float, default=0.07,
                   help="coverage of the interior at the desired pose")
    g.add_argument("--centre", default="0.04,-0.03", help="ellipse centre on the target (m)")
    g.add_argument("--aspect", type=float, default=1.4)
    g.add_argument("--angle", type=float, default=20.0)
    g.add_argument("--out-dir", default="textures")
    g.add_argument("--poses", default=None)
    m = sub.add_parser("measure")
    m.add_argument("--clean", required=True)
    m.add_argument("--occluded", required=True)
    m.add_argument("--threshold", type=float, default=10.0)
    m.add_argument("--poses", default=None)
    f = sub.add_parser("refill", help="same occluder geometry, other appearances")
    f.add_argument("--clean", required=True)
    f.add_argument("--occluded", required=True)
    f.add_argument("--kinds", default="uniform_dark,uniform_bright,low_contrast,textured")
    f.add_argument("--tag", default="disc")
    f.add_argument("--onto", default=None,
                   help="another clean texture: transfer the occluder there, same target geometry")
    f.add_argument("--threshold", type=float, default=10.0)
    f.add_argument("--out-dir", default="textures")
    f.add_argument("--poses", default=None)
    args = ap.parse_args(argv)
    return {"generate": cmd_generate, "measure": cmd_measure, "refill": cmd_refill}[args.cmd](args)


if __name__ == "__main__":
    sys.exit(main())
