#!/usr/bin/env python3
"""Compare LCIO CompleteTracks vs GSF vs MC truth pT."""
import ROOT, numpy as np, sys

fname = sys.argv[1] if len(sys.argv) > 1 else "rec_gsf_v01.root"
f = ROOT.TFile.Open(fname)
if not f or f.IsZombie():
    print(f"ERROR: cannot open {fname}"); sys.exit(1)
t = f.Get("events"); n = t.GetEntries()
print(f"Events: {n}")

Bz = 3.0; alpha = Bz * 2.99792458e-4
gen_pt, lcio_pt, gsf_pt = [], [], []

for i in range(n):
    t.GetEntry(i)
    mcp = getattr(t, 'MCParticle', None)
    if not mcp or mcp.size() == 0: continue
    p = mcp[0]; px, py = p.momentum.x, p.momentum.y
    gen_pt.append(np.sqrt(px*px + py*py))

    cts = getattr(t, 'CompleteTracks', None)
    lv = None
    if cts and cts.size() > 0:
        for ts in cts[0].getTrackStates():
            if ts.location == 4:  # AtIP
                lv = abs(alpha / ts.omega) if abs(ts.omega) > 1e-12 else 0; break
    lcio_pt.append(lv)

    gsf_c = getattr(t, 'GSFTracks', None)
    gv = None
    if gsf_c and gsf_c.size() > 0:
        for ts in gsf_c[0].getTrackStates():
            if ts.location == 4:
                gv = abs(alpha / ts.omega) if abs(ts.omega) > 1e-12 else 0; break
    gsf_pt.append(gv)

f.Close()
valid = [(g,l,s) for g,l,s in zip(gen_pt, lcio_pt, gsf_pt) if l is not None and s is not None]
g = np.array([v[0] for v in valid])
l = np.array([v[1] for v in valid])
s = np.array([v[2] for v in valid])
print(f"Tracks: {len(valid)}")

ld = (l - g) / g * 100; sd = (s - g) / g * 100
print(f"\nLCIO  mean={np.mean(ld):.4f}% rms={np.std(ld):.4f}%")
print(f"GSF   mean={np.mean(sd):.4f}% rms={np.std(sd):.4f}%")
print(f"Corr(LCIO,GSF) = {np.corrcoef(l, s)[0,1]:.4f}")

c = ROOT.TCanvas("c","pT",1200,500); c.Divide(2,1)
hL = ROOT.TH1F("hL","LCIO (rec-gen)/gen [%]",50,-2,2)
hS = ROOT.TH1F("hS","GSF (rec-gen)/gen [%]",50,-2,2)
for x in ld: hL.Fill(x)
for x in sd: hS.Fill(x)
hL.SetLineColor(ROOT.kBlue); hS.SetLineColor(ROOT.kRed)
c.cd(1); hL.Draw(); hL.Fit("gaus","Q")
c.cd(2); hS.Draw(); hS.Fit("gaus","Q")
oname = fname.replace(".root","") + "_pt.png"
c.SaveAs(oname)
print(f"Saved {oname}")
