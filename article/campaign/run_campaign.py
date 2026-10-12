#!/usr/bin/env python3
"""Run one design of the simulation campaign (any operating system).

Python equivalent of run_campaign.sh, for Windows without bash: the same
designs, run lists, file names, program arguments and resume rule.

  python campaign/run_campaign.py DESIGN [--jobs 4] [--bin PATH] [--tex DIR]
                                         [--ref FILE] [--out DIR] [--niter 600]
  DESIGN: validate | pilot | confirmatory | replication | sweep

Defaults (relative to the directory above campaign/):
  --bin    build/herpvs_campaign, or build/Release/herpvs_campaign.exe
  --tex    textures/            (made by tools/make_textures.py)
  --ref    textures/peppers.jpg; textures/coffee.png for the replication
  --out    runs/                logs in runs/<design>/logs, console output in
                                runs/<design>/console
Resumable: a run whose log already has more than NITER lines of data is
skipped, so the script can be interrupted (Ctrl-C) and restarted. Each run
writes its log under a temporary name and renames it only on completion.
"""

import argparse
import concurrent.futures as cf
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

ALL_SCENARIOS = ["nominal", "uniform_dark_disc", "textured_disc", "low_contrast_disc"]


def design(name):
    """(pose file, ids, weightings, interactions, scenarios, pixel convention)"""
    poses = os.path.join(HERE, "poses.csv")
    if name == "validate":   # Chapter 4 pose, thesis pixel order
        return None, [0], [1, 2, 3], [0], ["nominal", "ch4_disc"], 0
    if name == "pilot":
        return os.path.join(HERE, "poses_pilot.csv"), [101, 102, 103, 104], [1, 2, 3, 4, 5], [0, 1], ALL_SCENARIOS, 1
    if name == "confirmatory":   # V1-V4, as pre-registered
        return poses, list(range(1, 33)), [1, 2, 3, 4], [0, 1], ALL_SCENARIOS, 1
    if name == "replication":
        return poses, list(range(1, 33)), [2, 3], [0, 1], ["nominal", "coffee_dark_disc"], 1
    if name == "sweep":
        return poses, list(range(1, 33)), [2, 3], [0, 1], \
            ["uniform_dark_f150", "uniform_dark_f250", "uniform_dark_f350", "uniform_dark_f450"], 1
    sys.exit(f"unknown design {name}")


def default_bin():
    for cand in (os.path.join(ROOT, "build", "herpvs_campaign"),
                 os.path.join(ROOT, "build", "Release", "herpvs_campaign.exe"),
                 os.path.join(ROOT, "build", "herpvs_campaign.exe")):
        if os.path.isfile(cand):
            return cand
    return os.path.join(ROOT, "build", "herpvs_campaign")


def servo_texture(scen, ref, tex):
    if scen == "nominal":
        return ref
    for ext in (".png", ".jpg"):
        p = os.path.join(tex, scen + ext)
        if os.path.isfile(p):
            return p
    sys.exit(f"missing texture for scenario {scen} in {tex} (run tools/make_textures.py)")


def data_lines(path):
    with open(path) as fh:
        return sum(1 for line in fh if not line.startswith("#"))


def run_one(job, args, pose_file, pix, logdir, condir):
    L, scen, tex, W, P = job
    name = f"L{L}_{scen}_w{W}_p{P}"
    log = os.path.join(logdir, name + ".csv")
    if os.path.isfile(log) and data_lines(log) > args.niter:
        return name, "skipped"
    cmd = [args.bin, "-d", "-c", "-n", str(args.niter), "-w", str(W), "-L", str(L), "-x", str(pix),
           "-r", args.ref, "-t", tex]
    if pose_file:
        cmd += ["-P", pose_file]
    cmd += ["-p", str(P), "-o", log + ".part"]
    with open(os.path.join(condir, name + ".txt"), "w") as out:
        rc = subprocess.call(cmd, stdout=out, stderr=subprocess.STDOUT)
    if rc == 0 and os.path.isfile(log + ".part"):
        os.replace(log + ".part", log)
        return name, "done"
    return name, f"FAILED (exit {rc}, see {os.path.join(condir, name + '.txt')})"


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("design")
    ap.add_argument("--jobs", type=int, default=int(os.environ.get("JOBS", 3)))
    ap.add_argument("--bin", default=os.environ.get("BIN") or default_bin())
    ap.add_argument("--tex", default=os.environ.get("TEX") or os.path.join(ROOT, "textures"))
    ap.add_argument("--ref", default=os.environ.get("REF"))
    ap.add_argument("--out", default=os.environ.get("OUT") or os.path.join(ROOT, "runs"))
    ap.add_argument("--niter", type=int, default=int(os.environ.get("NITER", 600)))
    ap.add_argument("--dry-run", action="store_true", help="list the runs without running them")
    args = ap.parse_args(argv)
    if not args.ref:
        args.ref = os.path.join(args.tex, "coffee.png" if args.design == "replication" else "peppers.jpg")
    pose_file, ids, weightings, inters, scens, pix = design(args.design)
    if args.design != "validate":   # validate always uses the thesis convention
        pix = int(os.environ.get("PIX", pix))
    if not args.dry_run:
        if not os.path.isfile(args.bin):
            sys.exit(f"program not found: {args.bin} (build it first, or give --bin)")
        if not os.path.isfile(args.ref):
            sys.exit(f"reference texture not found: {args.ref} (run tools/make_textures.py, or give --ref)")
    jobs = [(L, s, servo_texture(s, args.ref, args.tex), W, P)
            for L in inters for s in scens for W in weightings for P in ids]
    d = os.path.join(args.out, args.design)
    logdir, condir = os.path.join(d, "logs"), os.path.join(d, "console")
    print(f"{args.design}: {len(jobs)} runs, {args.jobs} in parallel, logs in {logdir}", flush=True)
    if args.dry_run:
        for j in jobs:
            print(f"L{j[0]}_{j[1]}_w{j[3]}_p{j[4]}")
        return 0
    os.makedirs(logdir, exist_ok=True)
    os.makedirs(condir, exist_ok=True)
    done = 0
    with cf.ThreadPoolExecutor(max_workers=args.jobs) as ex:
        futures = [ex.submit(run_one, j, args, pose_file, pix, logdir, condir) for j in jobs]
        for f in cf.as_completed(futures):
            name, status = f.result()
            done += 1
            if status != "skipped":
                print(f"[{done}/{len(jobs)}] {name}: {status}", flush=True)
    n = len([f for f in os.listdir(logdir) if f.endswith(".csv")])
    print(f"done: {n} of {len(jobs)} logs complete")
    return 0


if __name__ == "__main__":
    sys.exit(main())
