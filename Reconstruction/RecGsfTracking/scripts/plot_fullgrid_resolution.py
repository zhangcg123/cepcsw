#!/usr/bin/env python3
"""Full-grid pT resolution: (rec - truth)/truth for LCIO vs GSF across all energies/thetas/seeds.

Groups by pT energy, produces:
  1. Resolution comparison plots per (energy, theta) group
  2. Summary overlay: LCIO vs GSF rms vs energy (both theta bands)
  3. Scatter: GSF pT vs LCIO pT
"""

import ROOT, numpy as np, sys, os, glob
from collections import defaultdict

ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptStat(1111)
ROOT.gStyle.SetOptFit(111)

Bz = 3.0
alpha = Bz * 2.99792458e-4  # GeV/mm

# ── gather all files ──
files = sorted(glob.glob("/cefs/higgs/zhangcg/cepc/20May2026/CEPCSW/gsf-e--*.root"))
print(f"Found {len(files)} GSF output files")

# ── parse filename: gsf-e--{pT}-{theta}-{seed}.root ──
def parse(fname):
    base = os.path.basename(fname)
    # gsf-e--0.2-135-1.root  ->  ['gsf','e','','0.2','135','1']
    parts = base.replace(".root","").split("-")
    pT = float(parts[3])
    theta = int(parts[4])
    seed = int(parts[5])
    return pT, theta, seed

# ── accumulate ──
# key = (pT, theta) -> {lcio: [], gsf: [], truth: [], lcio_d0: [], gsf_d0: [], lcio_z0: [], gsf_z0: [], nhits: []}
data = defaultdict(lambda: {
    "lcio_pt": [], "gsf_pt": [], "truth_pt": [],
    "lcio_d0": [], "gsf_d0": [], "lcio_z0": [], "gsf_z0": [],
    "nHits": [], "nComps": [], "chi2_lcio": [], "chi2_gsf": [],
})

for fname in files:
    pT_key, theta_key, seed_key = parse(fname)
    key = (pT_key, theta_key)
    f = ROOT.TFile.Open(fname)
    if not f or f.IsZombie():
        print(f"  SKIP {fname}: zombie")
        continue
    t = f.Get("events")
    if not t:
        print(f"  SKIP {fname}: no events tree")
        f.Close()
        continue
    nev = t.GetEntries()
    for iev in range(nev):
        t.GetEntry(iev)
        mcp = getattr(t, 'MCParticle', None)
        if not mcp or mcp.size() == 0: continue
        p0 = mcp[0]; px, py = p0.momentum.x, p0.momentum.y
        gpt = np.sqrt(px*px + py*py)

        # LCIO
        cts = getattr(t, 'CompleteTracks', None)
        lv = None; l_d0 = None; l_z0 = None; l_chi2 = None
        if cts and cts.size() > 0:
            ts_vec = getattr(t, '_CompleteTracks_trackStates', None)
            trk0 = cts[0]
            beg, end = trk0.trackStates_begin, trk0.trackStates_end
            if ts_vec:
                for j in range(beg, min(end, ts_vec.size())):
                    ts = ts_vec[j]
                    if ts.location == 4:  # AtIP
                        lv = abs(alpha / ts.omega) if abs(ts.omega) > 1e-12 else 0
                        l_d0 = ts.D0; l_z0 = ts.Z0
                        break
            l_chi2 = trk0.getChi2() if hasattr(trk0, 'getChi2') else 0.0

        # GSF
        gsf_c = getattr(t, 'GSFTracks', None)
        gv = None; g_d0 = None; g_z0 = None; g_chi2 = None; nComps = 0; nHits = 0
        if gsf_c and gsf_c.size() > 0:
            ts_vec = getattr(t, '_GSFTracks_trackStates', None)
            trk0 = gsf_c[0]
            beg, end = trk0.trackStates_begin, trk0.trackStates_end
            if ts_vec and beg < ts_vec.size():
                ts = ts_vec[beg]
                gv = abs(alpha / ts.omega) if abs(ts.omega) > 1e-12 else 0
                g_d0 = ts.D0; g_z0 = ts.Z0
            g_chi2 = trk0.getChi2() if hasattr(trk0, 'getChi2') else 0.0
            nHits = trk0.trackerHits_size() if hasattr(trk0, 'trackerHits_size') else 0

        if lv is None or gv is None: continue

        data[key]["lcio_pt"].append(lv)
        data[key]["gsf_pt"].append(gv)
        data[key]["truth_pt"].append(gpt)
        data[key]["lcio_d0"].append(l_d0 if l_d0 else 0)
        data[key]["gsf_d0"].append(g_d0 if g_d0 else 0)
        data[key]["lcio_z0"].append(l_z0 if l_z0 else 0)
        data[key]["gsf_z0"].append(g_z0 if g_z0 else 0)
        data[key]["nHits"].append(nHits)
        data[key]["nComps"].append(nComps)
        data[key]["chi2_lcio"].append(l_chi2 if l_chi2 else 0)
        data[key]["chi2_gsf"].append(g_chi2 if g_chi2 else 0)
    f.Close()

# ── print summary ──
print(f"\n{'='*80}")
print(f"{'Energy':>8s} {'Theta':>6s} {'Ntracks':>8s} {'LCIO mean%':>10s} {'LCIO rms%':>10s} {'GSF mean%':>10s} {'GSF rms%':>10s}")
print(f"{'-'*80}")
for key in sorted(data.keys()):
    d = data[key]
    pT, theta = key
    g = np.array(d["truth_pt"])
    l = np.array(d["lcio_pt"])
    s = np.array(d["gsf_pt"])
    lr = (l - g) / g * 100
    sr = (s - g) / g * 100
    print(f"{pT:8.1f} {theta:6d} {len(g):8d} {np.mean(lr):10.4f} {np.std(lr):10.4f} {np.mean(sr):10.4f} {np.std(sr):10.4f}")
print(f"{'='*80}")

# ── per-(pT,theta) resolution histograms ──
colors = [ROOT.kRed, ROOT.kBlue, ROOT.kGreen+2, ROOT.kMagenta+2, ROOT.kOrange+2,
          ROOT.kCyan+2, ROOT.kYellow+2, ROOT.kSpring+2]

print("\nMaking per-group resolution plots...")
for key in sorted(data.keys()):
    d = data[key]
    pT, theta = key
    g = np.array(d["truth_pt"])
    l = np.array(d["lcio_pt"])
    s = np.array(d["gsf_pt"])
    lr = (l - g) / g * 100
    sr = (s - g) / g * 100

    # auto-range
    pmax = max(abs(np.percentile(lr, [1, 99])).max(),
               abs(np.percentile(sr, [1, 99])).max(), 5.0)
    nbins = 60
    xlo, xhi = -pmax * 1.3, pmax * 1.3

    hL = ROOT.TH1F("hL", "; (p_{T}^{rec} - p_{T}^{truth}) / p_{T}^{truth}  [%]; Tracks",
                    nbins, xlo, xhi)
    hS = ROOT.TH1F("hS", "; (p_{T}^{rec} - p_{T}^{truth}) / p_{T}^{truth}  [%]; Tracks",
                    nbins, xlo, xhi)
    for v in lr: hL.Fill(v)
    for v in sr: hS.Fill(v)
    hL.SetLineColor(ROOT.kBlue); hL.SetLineWidth(2)
    hS.SetLineColor(ROOT.kRed);  hS.SetLineWidth(2)

    c = ROOT.TCanvas("c", f"pT={pT} theta={theta}", 1200, 500)
    c.Divide(2, 1)

    c.cd(1); ROOT.gPad.SetLeftMargin(0.14)
    hL.Draw("HIST"); hL.Fit("gaus", "Q")
    tl = ROOT.TLatex(); tl.SetNDC(); tl.SetTextSize(0.045)
    tl.DrawLatex(0.15, 0.91, f"LCIO  pT={pT} GeV #theta={theta}#circ  n={len(lr)}")
    tl.DrawLatex(0.15, 0.84, f"#mu={np.mean(lr):.2f}%  #sigma={np.std(lr):.2f}%")

    c.cd(2); ROOT.gPad.SetLeftMargin(0.14)
    hS.Draw("HIST"); hS.Fit("gaus", "Q")
    tl.DrawLatex(0.15, 0.91, f"GSF  pT={pT} GeV #theta={theta}#circ  n={len(sr)}")
    tl.DrawLatex(0.15, 0.84, f"#mu={np.mean(sr):.2f}%  #sigma={np.std(sr):.2f}%")

    outname = f"ptres_pT{pT}_theta{theta}.png"
    c.SaveAs(outname)
    c.Close()
    print(f"  {outname}")

# ── summary: RMS vs pT ──
print("\nMaking summary RMS vs energy plot...")
c_sum = ROOT.TCanvas("c_sum", "RMS vs pT", 900, 600)
c_sum.SetGrid()

# Build TMultiGraph
mg = ROOT.TMultiGraph("mg", "; p_{T}^{truth} [GeV]; RMS(p_{T}^{rec} - p_{T}^{truth}) / p_{T}^{truth}  [%]")

graphs = {}
for key in sorted(data.keys()):
    d = data[key]
    pT_val, theta_val = key
    g = np.array(d["truth_pt"])
    l = np.array(d["lcio_pt"])
    s = np.array(d["gsf_pt"])
    if len(g) < 10: continue
    lr = (l - g) / g * 100
    sr = (s - g) / g * 100

    if theta_val not in graphs:
        graphs[theta_val] = {"l_x": [], "l_y": [], "g_x": [], "g_y": [], "n": []}
    graphs[theta_val]["l_x"].append(pT_val)
    graphs[theta_val]["l_y"].append(np.std(lr))
    graphs[theta_val]["g_x"].append(pT_val)
    graphs[theta_val]["g_y"].append(np.std(sr))
    graphs[theta_val]["n"].append(len(g))

colors_m = {85: ROOT.kBlue, 135: ROOT.kRed}
markers_m = {85: 21, 135: 22}

for theta_val in [85, 135]:
    if theta_val not in graphs: continue
    gr = graphs[theta_val]
    # sort by pT
    order = np.argsort(gr["l_x"])

    gL = ROOT.TGraph()
    gG = ROOT.TGraph()
    for i in range(len(order)):
        idx = order[i]
        gL.SetPoint(gL.GetN(), gr["l_x"][idx], gr["l_y"][idx])
        gG.SetPoint(gG.GetN(), gr["g_x"][idx], gr["g_y"][idx])

    color = colors_m[theta_val]
    gL.SetMarkerStyle(markers_m[theta_val])
    gL.SetMarkerColor(color); gL.SetLineColor(color); gL.SetLineWidth(2)
    gL.SetName(f"lcio_{theta_val}")
    gG.SetMarkerStyle(markers_m[theta_val] + 2)
    gG.SetMarkerColor(color + 1); gG.SetLineColor(color + 1); gG.SetLineStyle(2); gG.SetLineWidth(2)
    gG.SetName(f"gsf_{theta_val}")

    mg.Add(gL, "LP")
    mg.Add(gG, "LP")

mg.Draw("A")
mg.GetYaxis().SetRangeUser(0, 18)
mg.GetXaxis().SetRangeUser(0, 2.2)

leg2 = c_sum.BuildLegend(0.62, 0.65, 0.88, 0.88)
leg2.SetBorderSize(0)

# Update legend entries to be more descriptive
leg_entries = leg2.GetListOfPrimitives()
for i, obj in enumerate(leg_entries):
    if hasattr(obj, 'SetLabel'):
        pass  # Keep auto-generated labels

# Instead, add text labels manually
tl2 = ROOT.TLatex(); tl2.SetNDC(); tl2.SetTextSize(0.035)
tl2.DrawLatex(0.62, 0.88, "LCIO #theta=85#circ (#bullet)")
tl2.DrawLatex(0.62, 0.85, "GSF  #theta=85#circ (#square)")
tl2.DrawLatex(0.62, 0.82, "LCIO #theta=135#circ (#bullet)")
tl2.DrawLatex(0.62, 0.79, "GSF  #theta=135#circ (#square)")

c_sum.SaveAs("ptres_summary_rms.png")
c_sum.Close()
print("  ptres_summary_rms.png")

# ── overlay: all energies side by side ──
print("\nMaking overlay histograms...")
for theta_val in [85, 135]:
    c_ov = ROOT.TCanvas("c_ov", f"overlay_theta{theta_val}", 1400, 700)
    c_ov.Divide(4, 1)

    pad_idx = 0
    for pT_val in [0.2, 0.5, 1.0, 2.0]:
        pad_idx += 1
        c_ov.cd(pad_idx); ROOT.gPad.SetLeftMargin(0.15)

        key = (pT_val, theta_val)
        if key not in data: continue
        d = data[key]
        g = np.array(d["truth_pt"])
        l = np.array(d["lcio_pt"])
        s = np.array(d["gsf_pt"])
        lr = (l - g) / g * 100
        sr = (s - g) / g * 100

        pmax = max(abs(np.percentile(lr, [1, 99])).max(),
                   abs(np.percentile(sr, [1, 99])).max(), 5.0)
        nbins = 50
        xlo, xhi = -pmax * 1.3, pmax * 1.3

        hL = ROOT.TH1F(f"hL_{pT_val}_{theta_val}", f"pT={pT_val} GeV #theta={theta_val}#circ",
                        nbins, xlo, xhi)
        hS = ROOT.TH1F(f"hS_{pT_val}_{theta_val}", f"pT={pT_val} GeV #theta={theta_val}#circ",
                        nbins, xlo, xhi)
        for v in lr: hL.Fill(v); hS.Fill(v)
        hL.SetLineColor(ROOT.kBlue); hL.SetLineWidth(2)
        hS.SetLineColor(ROOT.kRed);  hS.SetLineWidth(2)

        hL.Draw("HIST")
        hS.Draw("HIST SAMES")
        if pad_idx == 4:
            leg = ROOT.TLegend(0.55, 0.75, 0.88, 0.88)
            leg.AddEntry(hL, "LCIO", "l")
            leg.AddEntry(hS, "GSF", "l")
            leg.Draw()

    outname = f"ptres_overlay_theta{theta_val}.png"
    c_ov.SaveAs(outname)
    c_ov.Close()
    print(f"  {outname}")

# ── final: scatter plot GSF vs LCIO pT ──
print("\nMaking GSF vs LCIO scatter plot...")
c_sc = ROOT.TCanvas("c_sc", "GSF vs LCIO", 1000, 700)
c_sc.Divide(2, 2)
all_l = []; all_s = []; all_g = []
pad = 0
for pT_val in [0.2, 0.5, 1.0, 2.0]:
    pad += 1
    c_sc.cd(pad); ROOT.gPad.SetLeftMargin(0.14)
    h2 = ROOT.TH2F(f"h2_{pT_val}", "; LCIO p_{T} [GeV]; GSF p_{T} [GeV]",
                    100, pT_val*0.5, pT_val*1.5, 100, pT_val*0.5, pT_val*1.5)
    for theta_val in [85, 135]:
        key = (pT_val, theta_val)
        if key not in data: continue
        d = data[key]
        for lv, gv in zip(d["lcio_pt"], d["gsf_pt"]):
            h2.Fill(lv, gv)
            all_l.append(lv); all_s.append(gv)
            all_g.append(0)  # placeholder
    h2.Draw("COLZ")
    # Draw diagonal
    l_diag = ROOT.TLine(pT_val*0.5, pT_val*0.5, pT_val*1.5, pT_val*1.5)
    l_diag.SetLineColor(ROOT.kRed); l_diag.SetLineWidth(1)
    l_diag.Draw()
    tl = ROOT.TLatex(); tl.SetNDC(); tl.SetTextSize(0.06)
    tl.DrawLatex(0.18, 0.91, f"pT={pT_val} GeV")

c_sc.SaveAs("ptres_gsf_vs_lcio.png")
c_sc.Close()
print("  ptres_gsf_vs_lcio.png")

# ── stats dump ──
print(f"\n{'='*80}")
print("Global statistics (all energies, all thetas combined)")
all_l_arr = np.array(all_l); all_s_arr = np.array(all_s)
print(f"  Total tracks: {len(all_l_arr)}")
print(f"  LCIO pT mean: {np.mean(all_l_arr):.4f} GeV")
print(f"  GSF  pT mean: {np.mean(all_s_arr):.4f} GeV")
print(f"  Correlation LCIO-GSF: r={np.corrcoef(all_l_arr, all_s_arr)[0,1]:.6f}")
print(f"  Fractional difference (GSF-LCIO)/LCIO mean: {np.mean((all_s_arr - all_l_arr)/all_l_arr)*100:.4f}%")
print(f"{'='*80}")

print("\nDone! Files created:")
for f in sorted(glob.glob("ptres_*.png")):
    print(f"  {f}")
