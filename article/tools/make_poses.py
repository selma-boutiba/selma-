#!/usr/bin/env python3
"""Generate the initial poses of the simulation campaign.

Poses are drawn at random (fixed seed) and accepted only if the planar
target covers the whole interior region of the image, so that no measurement
is lost off-target. Each accepted pose is characterised by the image
displacement it induces with respect to the desired view, and poses are
stratified into displacement bins, which serve as the difficulty levels of
the study.

Conventions follow the simulation program:
  * target: rectangle |X| <= 0.30 m, |Y| <= 0.215 m in the plane Z_o = 0;
  * camera: f = 870 px, principal point (160, 120), image 320 x 240;
  * interior region: the 300 x 220 pixels left by a 10-pixel border;
  * pose cMo = (t, theta*u), as vpHomogeneousMatrix::buildFrom(tx, ty, tz,
    tux, tuy, tuz): X_c = R X_o + t;
  * desired pose cdMo: R = I, t = (0, 0, 1.3).

Usage:
  make_poses.py --check-chapter4           # displacement of the Chapter 4 pose
  make_poses.py --out poses.csv            # generate the stratified pose set
"""

import argparse
import csv
import math
import os
import sys

import numpy as np

F_PX = 870.0
U0, V0 = 160.0, 120.0
WIDTH, HEIGHT, BORDER = 320, 240, 10
TARGET_HALF_X, TARGET_HALF_Y = 0.30, 0.215
Z_DESIRED = 1.3

CHAPTER4_POSE = (0.04, -0.04, 1.10, 12.0, -12.0, 6.0)  # m, m, m, deg, deg, deg


def rodrigues(tu):
    """Rotation matrix of the rotation vector theta*u (radians)."""
    tu = np.asarray(tu, dtype=float)
    theta = np.linalg.norm(tu)
    if theta < 1e-12:
        return np.eye(3)
    k = tu / theta
    kx = np.array([[0, -k[2], k[1]], [k[2], 0, -k[0]], [-k[1], k[0], 0]])
    return np.eye(3) + math.sin(theta) * kx + (1 - math.cos(theta)) * kx @ kx


def interior_pixels(step=1):
    """Pixel centres (u, v) of the interior region."""
    u = np.arange(BORDER, WIDTH - BORDER, step, dtype=float)
    v = np.arange(BORDER, HEIGHT - BORDER, step, dtype=float)
    uu, vv = np.meshgrid(u, v)
    return uu.ravel(), vv.ravel()


def backproject_to_target(R, t, u, v):
    """Intersect the rays of pixels (u, v) with the target plane Z_o = 0.

    Returns the object-frame points (N x 3) and a boolean array telling
    whether each ray meets the plane in front of the camera.
    """
    x = (u - U0) / F_PX
    y = (v - V0) / F_PX
    d_c = np.stack([x, y, np.ones_like(x)], axis=1)          # camera frame
    centre_o = -R.T @ t                                       # camera centre
    d_o = d_c @ R                                             # R^T d_c, row-wise
    s = -centre_o[2] / d_o[:, 2]
    pts = centre_o[None, :] + s[:, None] * d_o
    return pts, s > 0


def project(R, t, pts):
    """Project object-frame points into the image of camera (R, t)."""
    pc = pts @ R.T + t[None, :]
    u = U0 + F_PX * pc[:, 0] / pc[:, 2]
    v = V0 + F_PX * pc[:, 1] / pc[:, 2]
    return u, v, pc[:, 2]


def pose_matrix(pose):
    tx, ty, tz, rx, ry, rz = pose
    return rodrigues(np.radians([rx, ry, rz])), np.array([tx, ty, tz])


def coverage_ok(R, t, margin=0.0):
    """True if every interior pixel sees the target (convexity: corners suffice)."""
    u = np.array([BORDER, WIDTH - BORDER - 1, WIDTH - BORDER - 1, BORDER], float)
    v = np.array([BORDER, BORDER, HEIGHT - BORDER - 1, HEIGHT - BORDER - 1], float)
    pts, front = backproject_to_target(R, t, u, v)
    inside = (np.abs(pts[:, 0]) <= TARGET_HALF_X - margin) & \
             (np.abs(pts[:, 1]) <= TARGET_HALF_Y - margin)
    return bool(np.all(front & inside))


def image_displacement(pose, step=1):
    """Mean and maximum displacement (px) of the interior pixels of the image
    at `pose` when the same target points are seen from the desired pose.

    This is the definition behind Section 4.6.3 ("a mean image displacement
    of some 41 pixels, reaching 65 pixels"): for the Chapter 4 pose it gives
    40.0 px and 65.0 px. Mapping the desired image into the initial view
    instead gives 47.0 px and 77.7 px; state which one is used.
    """
    Rd, td = np.eye(3), np.array([0.0, 0.0, Z_DESIRED])
    R, t = pose_matrix(pose)
    u, v = interior_pixels(step)
    pts, _ = backproject_to_target(R, t, u, v)
    u1, v1, _ = project(Rd, td, pts)
    d = np.hypot(u1 - u, v1 - v)
    return float(d.mean()), float(d.max())


def sample_pose(rng, t_lat, z_range, angle_range):
    tx, ty = rng.uniform(-t_lat, t_lat, size=2)
    tz = rng.uniform(*z_range)
    axis = rng.normal(size=3)
    axis /= np.linalg.norm(axis)
    angle = rng.uniform(*angle_range)
    rx, ry, rz = np.degrees(np.radians(angle) * axis)
    return (tx, ty, tz, rx, ry, rz)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--check-chapter4", action="store_true")
    ap.add_argument("--out", default="poses.csv")
    ap.add_argument("--seed", type=int, default=20261011)
    ap.add_argument("--bins", default="20,35,50,65",
                    help="mean-displacement bin edges in pixels")
    ap.add_argument("--per-bin", type=int, default=8)
    ap.add_argument("--t-lat", type=float, default=0.06,
                    help="max |tx|, |ty| of the target origin in the camera frame (m)")
    ap.add_argument("--z-range", default="1.00,1.25")
    ap.add_argument("--angle-range", default="4,26", help="rotation angle range (deg)")
    ap.add_argument("--margin", type=float, default=0.005,
                    help="coverage margin on the target edges (m)")
    ap.add_argument("--max-draws", type=int, default=200000)
    ap.add_argument("--id-offset", type=int, default=0,
                    help="added to the pose ids (use 100 for the pilot set)")
    args = ap.parse_args(argv)

    if args.check_chapter4:
        R, t = pose_matrix(CHAPTER4_POSE)
        mean_d, max_d = image_displacement(CHAPTER4_POSE)
        print(f"Chapter 4 pose {CHAPTER4_POSE}")
        print(f"  full coverage of the interior: {coverage_ok(R, t)}")
        print(f"  image displacement: mean {mean_d:.1f} px, max {max_d:.1f} px")
        return 0

    edges = [float(x) for x in args.bins.split(",")]
    z_range = tuple(float(x) for x in args.z_range.split(","))
    angle_range = tuple(float(x) for x in args.angle_range.split(","))
    nbins = len(edges) - 1
    accepted = [[] for _ in range(nbins)]
    rng = np.random.default_rng(args.seed)

    draws = 0
    while any(len(b) < args.per_bin for b in accepted) and draws < args.max_draws:
        draws += 1
        pose = sample_pose(rng, args.t_lat, z_range, angle_range)
        R, t = pose_matrix(pose)
        if not coverage_ok(R, t, args.margin):
            continue
        mean_d, max_d = image_displacement(pose, step=4)
        for b in range(nbins):
            if edges[b] <= mean_d < edges[b + 1] and len(accepted[b]) < args.per_bin:
                accepted[b].append((pose, b))
                break

    if any(len(b) < args.per_bin for b in accepted):
        print(f"could not fill all bins after {draws} draws: "
              f"{[len(b) for b in accepted]}", file=sys.stderr)
        return 1

    rows = []
    for b in range(nbins):
        for pose, _ in accepted[b]:
            mean_d, max_d = image_displacement(pose)          # full resolution
            rows.append((pose, b, mean_d, max_d))

    os.makedirs(os.path.dirname(os.path.abspath(args.out)), exist_ok=True)
    with open(args.out, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["pose_id", "bin", "tx_m", "ty_m", "tz_m", "tux_deg", "tuy_deg",
                    "tuz_deg", "mean_disp_px", "max_disp_px"])
        for i, (pose, b, mean_d, max_d) in enumerate(rows, start=1 + args.id_offset):
            w.writerow([i, b] + [f"{p:.6f}" for p in pose] + [f"{mean_d:.2f}", f"{max_d:.2f}"])

    print(f"{len(rows)} poses written to {args.out} after {draws} draws (seed {args.seed})")
    for b in range(nbins):
        d = [r[2] for r in rows if r[1] == b]
        print(f"  bin {b} [{edges[b]:.0f}, {edges[b + 1]:.0f}) px: n = {len(d)}, "
              f"mean displacement {np.mean(d):.1f} px")
    return 0


if __name__ == "__main__":
    sys.exit(main())
