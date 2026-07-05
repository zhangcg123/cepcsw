#!/usr/bin/env python3
"""Summarize RecGsfSimHitTuple momentum-retention diagnostics.

The tuple stores SimTrackerHit momentum at hit position. This script orders hits by
radius and reports retained p/p_primary ranges and tail probabilities. It is a
first diagnostic for CEPC electron energy-loss model tuning, not a replacement
for full Geant4 pre/post-step truth.
"""
import sys
import math
import ROOT
import numpy as np

fname = sys.argv[1] if len(sys.argv) > 1 else "gsf_simhit_tuple_test.root"
f = ROOT.TFile.Open(fname)
if not f or f.IsZombie():
    raise SystemExit(f"ERROR: cannot open {fname}")
t = f.Get("simhit_tuple")
if not t:
    raise SystemExit(f"ERROR: no simhit_tuple tree in {fname}")

all_ret = []
last_ret = []
min_ret = []
print(f"File: {fname}  entries={t.GetEntries()}")
print("evt hit_n mc_p first_p last_p min_p first_ret last_ret min_ret p<0.99 p<0.95 p<0.90")
for iev in range(t.GetEntries()):
    t.GetEntry(iev)
    n = int(t.hit_n)
    hits = []
    for i in range(n):
        hits.append((float(t.hit_r[i]), float(t.hit_time[i]), float(t.hit_p[i]), float(t.hit_retained_vs_primary[i])))
    hits.sort(key=lambda x: (x[0], x[1]))
    ret = np.array([h[3] for h in hits], dtype=float)
    p = np.array([h[2] for h in hits], dtype=float)
    if len(ret) == 0:
        print(f"{iev+1:3d} {n:5d} {float(t.mc_p):.4f} no hits")
        continue
    all_ret.extend(ret.tolist())
    last_ret.append(float(ret[-1]))
    min_ret.append(float(ret.min()))
    print(f"{iev+1:3d} {n:5d} {float(t.mc_p):.4f} {p[0]:.4f} {p[-1]:.4f} {p.min():.4f} "
          f"{ret[0]:.4f} {ret[-1]:.4f} {ret.min():.4f} "
          f"{np.mean(ret < 0.99):.3f} {np.mean(ret < 0.95):.3f} {np.mean(ret < 0.90):.3f}")

all_ret = np.array(all_ret, dtype=float)
last_ret = np.array(last_ret, dtype=float)
min_ret = np.array(min_ret, dtype=float)
print("\nGlobal retained p/p_primary over all dumped sim hits:")
if len(all_ret):
    qs = np.quantile(all_ret, [0, 0.01, 0.05, 0.10, 0.50, 0.90, 0.99, 1.0])
    print("quantiles 0/1/5/10/50/90/99/100%:", " ".join(f"{q:.5f}" for q in qs))
    print(f"tail P(ret<0.99)={np.mean(all_ret < 0.99):.4f} P(ret<0.95)={np.mean(all_ret < 0.95):.4f} P(ret<0.90)={np.mean(all_ret < 0.90):.4f}")
if len(last_ret):
    print("\nPer-event last-hit retained p/p_primary:", " ".join(f"{x:.5f}" for x in last_ret))
    print("Per-event min-hit retained p/p_primary:", " ".join(f"{x:.5f}" for x in min_ret))
f.Close()
