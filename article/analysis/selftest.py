#!/usr/bin/env python3
"""End-to-end check of analyze.py on synthetic logs with planted effects.

Trajectories follow the two regimes of the convergence analysis:
  nominal   ||dr_k|| ~ a (k + k0)^(-1/2)   (gain vanishing with the error)
  occluded  geometric decay to an equilibrium bias b_w
Planted effects, which the analysis must recover:
  * nominal tail slope -1/2;
  * occluded: V1 bias ~400 x V2;
  * L from the current image: V3 equivalent to V2 (ratio 1);
  * L from the desired image: V3 = 0.6 x V2, V4 = 1.5 x V2.

Run:  python3 selftest.py   (exit status 0 on success)
"""

import os
import subprocess
import sys
import tempfile

import numpy as np
import pandas as pd

HERE = os.path.dirname(os.path.abspath(__file__))
COLS = ("iter,tx,ty,tz,tux,tuy,tuz,ox,oy,oz,vx,vy,vz,wx,wy,wz,e2,eI2,lambda,mu,h_min,h_trace,scale,gamma,"
        "mean_w,frac_w01,frac_w25,frac_w25_c0,frac_w25_c1,frac_w25_c2,frac_w25_c3,dW,t_weight_ms,t_iter_ms")


def trajectory(rng, scenario, bias_mm, n=600):
    k = np.arange(1, n + 1, dtype=float)
    a = rng.uniform(5.0, 9.0)                       # mm
    if scenario == "nominal":
        t = a * ((k + 20.0) / 21.0) ** -0.5
    else:
        t = bias_mm + a * 0.92 ** k
    t = t * np.exp(0.01 * rng.normal(size=n))      # small terminal jitter
    r = t * 0.6                                    # deg, proportional for simplicity
    return k, t, r


def write_run(path, meta, k, t_mm, r_deg, rng):
    n = len(k)
    d = rng.normal(size=(n, 3))
    d /= np.linalg.norm(d, axis=1, keepdims=True)
    tt = d * (t_mm[:, None] * 1e-3)
    rr = d[:, ::-1] * np.radians(r_deg)[:, None]
    v = np.gradient(tt, axis=0) / 0.04
    data = np.zeros((n, len(COLS.split(","))))
    data[:, 0] = k
    c = {name: i for i, name in enumerate(COLS.split(","))}
    data[:, 1:4], data[:, 4:7], data[:, 7:10], data[:, 10:13] = tt, rr, tt * 0.1, v
    data[:, c["e2"]] = 1e7 * (1 + t_mm)            # e2: not monotone with accuracy
    data[:, c["h_min"]], data[:, c["h_trace"]] = 1e3, 6e3
    data[:, c["frac_w25"]] = 0.1
    data[:, c["dW"]] = 1e-4
    with open(path, "w") as fh:
        for kk, vv in meta.items():
            fh.write(f"# {kk}={vv}\n")
        fh.write(COLS + "\n")
        np.savetxt(fh, data, delimiter=",", fmt="%.10g")


def main():
    rng = np.random.default_rng(1)
    tmp = tempfile.mkdtemp(prefix="selftest_")
    logdir, out = os.path.join(tmp, "logs"), os.path.join(tmp, "results")
    os.makedirs(logdir)
    poses = pd.DataFrame({"pose_id": np.arange(1, 25), "bin": np.repeat([0, 1, 2], 8),
                          "mean_disp_px": np.repeat([28.0, 40.0, 52.0], 8)})
    poses.to_csv(os.path.join(tmp, "poses.csv"), index=False)
    factor = {("current", "hermite"): 1.0, ("current", "penalised"): 1.0, ("current", "adaptive"): 1.0,
              ("desired", "hermite"): 0.6, ("desired", "penalised"): 1.5, ("desired", "adaptive"): 0.8}
    for pose in range(1, 25):
        base = 3e-3 * np.exp(0.5 * rng.normal())   # V2 bias for this pose, mm
        for inter in ("current", "desired"):
            for scen, servo in (("nominal", "peppers.jpg"), ("uniform_dark_f070", "uniform_dark_f070.png")):
                for w in ("none", "tukey", "hermite", "penalised", "adaptive"):
                    if w == "none":
                        b = base * 400 * np.exp(0.2 * rng.normal())
                    elif w == "tukey":
                        b = base
                    else:
                        b = base * factor[(inter, w)] * np.exp(0.03 * rng.normal())
                    k, t, r = trajectory(rng, scen, b)
                    meta = {"weighting": w, "interaction": inter, "ref_texture": "peppers.jpg",
                            "servo_texture": servo, "pose_id": pose}
                    write_run(os.path.join(logdir, f"{inter}_{scen}_{w}_p{pose:02d}.csv"), meta, k, t, r, rng)

    subprocess.run([sys.executable, os.path.join(HERE, "analyze.py"), logdir, "--poses",
                    os.path.join(tmp, "poses.csv"), "--out", out], check=True)
    comps = pd.read_csv(os.path.join(out, "comparisons.csv"))
    summ = pd.read_csv(os.path.join(out, "summary.csv"))

    def get(inter, scen, a, b, outcome="final_t_mm"):
        m = comps[(comps.interaction == inter) & (comps.scenario == scen) & (comps.A == a) &
                  (comps.B == b) & (comps.outcome == outcome)]
        return m.iloc[0]

    checks = []
    occ = "uniform_dark_f070"
    c = get("desired", occ, "hermite", "tukey")
    checks.append(("V3/V2 with desired L recovered as 0.6, significant after Holm",
                   abs(c.ratio - 0.6) < 0.05 and c.p_holm < 0.01))
    c = get("current", occ, "hermite", "tukey")
    checks.append(("V3/V2 with current L declared equivalent within 25 %", bool(c.equivalent)))
    c = get("desired", occ, "penalised", "tukey")
    checks.append(("V4/V2 with desired L recovered as 1.5", abs(c.ratio - 1.5) < 0.1 and c.p_holm < 0.01))
    c = get("current", occ, "tukey", "none")
    checks.append(("V2/V1 under occlusion recovered as 1/400", abs(np.log(c.ratio * 400)) < 0.2))
    s = summ[(summ.scenario == "nominal")]
    checks.append(("nominal tail slope recovered as -1/2", bool(np.all(np.abs(s.slope_t + 0.5) < 0.05))))
    s = summ[(summ.scenario == occ) & (summ.weighting == "none")]
    checks.append(("V1 under occlusion mostly fails the success criterion", bool(np.all(s.success < 0.5))))
    figs = os.listdir(os.path.join(out, "figures"))
    checks.append(("figures written", len(figs) == 12))

    ok = True
    for name, passed in checks:
        print(("ok    " if passed else "FAIL  ") + name)
        ok &= bool(passed)
    print(f"results in {out}")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
