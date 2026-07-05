#!/usr/bin/env python3
"""Plot (rec - truth)/truth fractional pT resolution for LCIO and GSF."""
import ROOT, numpy as np, sys

# ── inputs ──────────────────────────────────────────────────────────────
fname = sys.argv[1] if len(sys.argv) > 1 else "rec_e_gsf.root"
f = ROOT.TFile.Open(fname)
if not f or f.IsZombie():
    print(f"ERROR: cannot open {fname}"); sys.exit(1)
t = f.Get("events")
nev = t.GetEntries()
print(f"File: {fname}  |  Events: {nev}")

Bz = 3.0
alpha = Bz * 2.99792458e-4  # B·c·10⁻⁴  [GeV/mm]

# ── accumulate ──────────────────────────────────────────────────────────
gen_pt, lcio_pt, gsf_pt = [], [], []

for iev in range(nev):
    t.GetEntry(iev)

    # truth pT ───────────────────────────────────────────────────────────
    mcp = getattr(t, 'MCParticle', None)
    if not mcp or mcp.size() == 0:
        continue
    p0 = mcp[0]
    px, py = p0.momentum.x, p0.momentum.y
    gpt = np.sqrt(px*px + py*py)
    gen_pt.append(gpt)

    # LCIO pT from CompleteTracks (location=4 is AtIP) ──────────────────
    cts = getattr(t, 'CompleteTracks', None)
    lv = None
    if cts and cts.size() > 0:
        ts_vec = getattr(t, '_CompleteTracks_trackStates', None)
        trk0 = cts[0]
        beg, end = trk0.trackStates_begin, trk0.trackStates_end
        if ts_vec:
            for j in range(beg, min(end, ts_vec.size())):
                ts = ts_vec[j]
                if ts.location == 4:  # AtIP
                    lv = abs(alpha / ts.omega) if abs(ts.omega) > 1e-12 else 0
                    break
    lcio_pt.append(lv)

    # GSF pT from GSFTracks (use the only per-track state) ──────────────
    gsf = getattr(t, 'GSFTracks', None)
    gv = None
    if gsf and gsf.size() > 0:
        ts_vec = getattr(t, '_GSFTracks_trackStates', None)
        trk0 = gsf[0]
        beg, end = trk0.trackStates_begin, trk0.trackStates_end
        if ts_vec and beg < ts_vec.size():
            # use the first state belonging to this track
            ts = ts_vec[beg]
            gv = abs(alpha / ts.omega) if abs(ts.omega) > 1e-12 else 0
    gsf_pt.append(gv)

f.Close()

# ── filter valid ────────────────────────────────────────────────────────
valid = [(g, l, s) for g, l, s in zip(gen_pt, lcio_pt, gsf_pt)
         if l is not None and s is not None]
print(f"Valid tracks (both LCIO and GSF): {len(valid)} / {nev}")

g = np.array([v[0] for v in valid])
l = np.array([v[1] for v in valid])
s = np.array([v[2] for v in valid])

# fractional resolution in %
ld_pct = (l - g) / g * 100
sd_pct = (s - g) / g * 100

print(f"\ngen_pT  mean = {np.mean(g):.4f} GeV  rms = {np.std(g):.4f}")
print(f"LCIO pT mean = {np.mean(l):.4f} GeV")
print(f"GSF  pT mean = {np.mean(s):.4f} GeV")
print(f"\nLCIO  (rec-gen)/gen [%]  mean = {np.mean(ld_pct):.4f}  rms = {np.std(ld_pct):.4f}")
print(f"GSF   (rec-gen)/gen [%]  mean = {np.mean(sd_pct):.4f}  rms = {np.std(sd_pct):.4f}")

# ── histograms ──────────────────────────────────────────────────────────
# auto-range: symmetric around 0, at least [-5, 5]%
pmax = max(abs(np.percentile(ld_pct, [1, 99])).max(),
           abs(np.percentile(sd_pct, [1, 99])).max(),
           5.0)
nbins = 50
xlo, xhi = -pmax * 1.2, pmax * 1.2

hL = ROOT.TH1F("hL", "; (p_{T}^{rec} - p_{T}^{truth}) / p_{T}^{truth}  [%]; Tracks",
                nbins, xlo, xhi)
hS = ROOT.TH1F("hS", "; (p_{T}^{rec} - p_{T}^{truth}) / p_{T}^{truth}  [%]; Tracks",
                nbins, xlo, xhi)
for x in ld_pct: hL.Fill(x)
for x in sd_pct: hS.Fill(x)

hL.SetLineColor(ROOT.kBlue);   hL.SetLineWidth(2)
hS.SetLineColor(ROOT.kRed);    hS.SetLineWidth(2)

# ── canvas ──────────────────────────────────────────────────────────────
ROOT.gStyle.SetOptStat(1111)
ROOT.gStyle.SetOptFit(111)
c = ROOT.TCanvas("c", "pT resolution", 1200, 600)
c.Divide(2, 1)

c.cd(1)
ROOT.gPad.SetLeftMargin(0.14)
hL.Draw("HIST")
hL.Fit("gaus", "Q")
leg = ROOT.TLegend(0.60, 0.75, 0.88, 0.88)
leg.AddEntry(hL, "LCIO CompleteTracks", "l")
leg.Draw()

c.cd(2)
ROOT.gPad.SetLeftMargin(0.14)
hS.Draw("HIST")
hS.Fit("gaus", "Q")
leg = ROOT.TLegend(0.60, 0.75, 0.88, 0.88)
leg.AddEntry(hS, "GSF (Bethe-Heitler)", "l")
leg.Draw()

outname = fname.replace(".root", "") + "_ptres.png"
c.SaveAs(outname)
print(f"\nSaved {outname}")

# ── overlay canvas ─────────────────────────────────────────────────────
c2 = ROOT.TCanvas("c2", "overlay", 800, 600)
hL.Draw("HIST")
hS.Draw("HIST SAMES")
leg2 = ROOT.TLegend(0.62, 0.75, 0.88, 0.88)
leg2.AddEntry(hL, "LCIO CompleteTracks", "l")
leg2.AddEntry(hS, "GSF (Bethe-Heitler)", "l")
leg2.Draw()
c2.SaveAs(outname.replace("_ptres", "_ptres_overlay"))
print(f"Saved {outname.replace('_ptres','_ptres_overlay')}")
