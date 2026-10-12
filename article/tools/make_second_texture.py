#!/usr/bin/env python3
"""Second texture of the replication (PREREGISTRATION_ADDENDUM.md, R.1).

skimage.data.coffee() (600 x 400 RGB, CC0, photograph by Rachel Michetti),
centre-cropped to the aspect ratio of the target (0.60 : 0.43 m) and resized
to 386 x 277 with Lanczos interpolation. It is kept in colour, as in the
study: the servoing program converts it to grey levels when it reads it, and
make_occluders.py converts it with PIL.

  python3 tools/make_second_texture.py --out textures/coffee.png
"""

import argparse
import os

from PIL import Image
from skimage import data

TARGET_ASPECT = 0.60 / 0.43
WIDTH = 386


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", required=True)
    args = ap.parse_args(argv)
    im = Image.fromarray(data.coffee())                       # 600 x 400, RGB
    w = round(im.height * TARGET_ASPECT)                       # 558
    x0 = (im.width - w) // 2
    im = im.crop((x0, 0, x0 + w, im.height))
    im = im.resize((WIDTH, round(WIDTH / TARGET_ASPECT)), Image.LANCZOS)   # 386 x 277
    os.makedirs(os.path.dirname(os.path.abspath(args.out)), exist_ok=True)
    im.save(args.out)
    print(f"{args.out}: {im.size[0]} x {im.size[1]}")


if __name__ == "__main__":
    main()
