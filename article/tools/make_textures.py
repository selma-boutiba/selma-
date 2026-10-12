#!/usr/bin/env python3
"""Every texture of the study, from the two thesis textures (any operating
system). Python equivalent of make_textures.sh.

  python tools/make_textures.py [texture_dir]          (default: textures)

Needs, in texture_dir: peppers.jpg (clean texture of Chapter 4) and
peppers_o_b.jpg (the same texture with the Chapter 4 black disc). Writes the
confirmatory scenes, the occlusion-sweep ellipses, the second texture and
its disc, ch4_disc.jpg for the validation, and coverage_*.txt reports.
"""

import contextlib
import os
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)
import make_occluders  # noqa: E402
import make_second_texture  # noqa: E402


def quiet(fn, argv, report):
    with open(report, "w") as fh, contextlib.redirect_stdout(fh):
        fn(argv)


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    T = os.path.abspath(argv[0]) if argv else os.path.join(ROOT, "textures")
    P = os.path.join(ROOT, "campaign", "poses.csv")
    clean, occ = os.path.join(T, "peppers.jpg"), os.path.join(T, "peppers_o_b.jpg")
    for f in (clean, occ):
        if not os.path.isfile(f):
            sys.exit(f"missing {f} (copy it from the thesis files)")

    print("== confirmatory scenes: the Chapter 4 disc, four appearances")
    quiet(make_occluders.main, ["refill", "--clean", clean, "--occluded", occ, "--out-dir", T, "--poses", P],
          os.path.join(T, "coverage_disc.txt"))
    print("== validation scene")
    shutil.copyfile(occ, os.path.join(T, "ch4_disc.jpg"))
    print("== occlusion sweep: dark ellipses")
    for f in ("0.15", "0.25", "0.35", "0.45"):
        quiet(make_occluders.main, ["generate", "--texture", clean, "--kind", "uniform_dark", "--fraction", f,
                                    "--out-dir", T], os.path.join(T, f"coverage_f{f}.txt"))
    print("== replication: second texture and the same disc")
    make_second_texture.main(["--out", os.path.join(T, "coffee.png")])
    quiet(make_occluders.main, ["refill", "--clean", clean, "--occluded", occ, "--onto", os.path.join(T, "coffee.png"),
                                "--kinds", "uniform_dark", "--tag", "coffee", "--out-dir", T],
          os.path.join(T, "coverage_coffee.txt"))
    os.replace(os.path.join(T, "uniform_dark_coffee.png"), os.path.join(T, "coffee_dark_disc.png"))
    print(f"done: textures in {T}")


if __name__ == "__main__":
    main()
