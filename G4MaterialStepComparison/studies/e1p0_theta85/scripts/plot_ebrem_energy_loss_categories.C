#include <TCanvas.h>
#include <TChain.h>
#include <TFile.h>
#include <TH1F.h>
#include <TLegend.h>
#include <TPaveText.h>
#include <TStyle.h>
#include <TSystem.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using namespace std;

namespace {
bool contains(const string& s, const string& needle) { return s.find(needle) != string::npos; }

bool isCrystalBar(const string& material, const string& volume) {
  return material == "G4_BGO" && (volume.find("bar_s") == 0 || contains(volume, "crystal_s"));
}

bool isWBeamPipeShell(const string& material, const string& volume) {
  return material == "G4_W" && contains(volume, "BeamPipe_BeforeCryoW");
}

bool isTrackerVolume(const string& volume) {
  return contains(volume, "VXD") || contains(volume, "ITK") || contains(volume, "TPC") ||
         contains(volume, "OTK") || contains(volume, "SIT") || contains(volume, "SET");
}

float quantile(vector<float> v, double p) {
  if (v.empty()) return NAN;
  sort(v.begin(), v.end());
  const double pos = p * (v.size() - 1);
  const int lo = floor(pos);
  const int hi = ceil(pos);
  if (lo == hi) return v[lo];
  return v[lo] + (v[hi] - v[lo]) * (pos - lo);
}

void saveCanvas(TCanvas& c, const string& outdir, const string& stem) {
  c.SaveAs((outdir + "/" + stem + ".png").c_str());
  c.SaveAs((outdir + "/" + stem + ".pdf").c_str());
}

void styleHist(TH1F& h, int color) {
  h.SetLineColor(color);
  h.SetLineWidth(2);
}

void drawOne(TH1F& h, const string& outdir, const string& stem, bool logx = false) {
  TCanvas c((stem + "_c").c_str(), stem.c_str(), 900, 650);
  c.SetLogy();
  if (logx) c.SetLogx();
  h.Draw("hist");
  saveCanvas(c, outdir, stem);
}
}

void plot_ebrem_energy_loss_categories(const char* outdir = "G4MaterialStepComparison/studies/e1p0_theta85") {
  gSystem->mkdir(outdir, true);
  const string plotdir = string(outdir) + "/plots";
  gSystem->mkdir(plotdir.c_str(), true);
  gStyle->SetOptStat(0);

  TChain ch("g4step_tuple");
  for (int i = 1; i <= 10; ++i) ch.Add(Form("gsf_material_steps-e--1.0-85-%d.root", i));

  vector<int>* track_id = nullptr;
  vector<int>* parent_id = nullptr;
  vector<int>* pdg = nullptr;
  vector<int>* process_subtype = nullptr;
  vector<float>* pre_p = nullptr;
  vector<float>* post_p = nullptr;
  vector<float>* loss = nullptr;
  vector<string>* material = nullptr;
  vector<string>* pre_volume = nullptr;

  ch.SetBranchAddress("track_id", &track_id);
  ch.SetBranchAddress("parent_id", &parent_id);
  ch.SetBranchAddress("pdg", &pdg);
  ch.SetBranchAddress("process_subtype", &process_subtype);
  ch.SetBranchAddress("pre_p", &pre_p);
  ch.SetBranchAddress("post_p", &post_p);
  ch.SetBranchAddress("loss", &loss);
  ch.SetBranchAddress("material", &material);
  ch.SetBranchAddress("pre_volume", &pre_volume);

  TH1F hFracCrystal("hFracCrystal", "Primary eBrem fractional energy loss;1 - p_{post}/p_{pre};steps", 120, 0, 1.0);
  TH1F hFracW("hFracW", "Primary eBrem fractional energy loss;1 - p_{post}/p_{pre};steps", 120, 0, 1.0);
  TH1F hFracTracker("hFracTracker", "Primary eBrem fractional energy loss;1 - p_{post}/p_{pre};steps", 120, 0, 1.0);
  TH1F hAbsCrystal("hAbsCrystal", "Primary eBrem absolute momentum loss;pre_p - post_p [GeV];steps", 120, 0, 1.0);
  TH1F hAbsW("hAbsW", "Primary eBrem absolute momentum loss;pre_p - post_p [GeV];steps", 120, 0, 1.0);
  TH1F hAbsTracker("hAbsTracker", "Primary eBrem absolute momentum loss;pre_p - post_p [GeV];steps", 120, 0, 1.0);

  TH1F hFracCrystalN("hFracCrystalN", "Primary eBrem fractional energy loss, shape-normalized;1 - p_{post}/p_{pre};normalized steps", 120, 0, 1.0);
  TH1F hEfEiTrackerN("hEfEiTrackerN", "Primary eBrem in tracker volumes;E_{f}/E_{i};normalized steps", 120, 0, 1.0);
  TH1F hFracWN("hFracWN", "Primary eBrem fractional energy loss, shape-normalized;1 - p_{post}/p_{pre};normalized steps", 120, 0, 1.0);
  TH1F hFracTrackerN("hFracTrackerN", "Primary eBrem fractional energy loss, shape-normalized;1 - p_{post}/p_{pre};normalized steps", 120, 0, 1.0);

  styleHist(hFracCrystal, kBlack); styleHist(hFracW, kBlue + 1); styleHist(hFracTracker, kCyan + 2);
  styleHist(hAbsCrystal, kBlack); styleHist(hAbsW, kBlue + 1); styleHist(hAbsTracker, kCyan + 2);
  styleHist(hFracCrystalN, kBlack); styleHist(hFracWN, kBlue + 1); styleHist(hFracTrackerN, kCyan + 2);
  styleHist(hEfEiTrackerN, kBlack);

  vector<float> fracCrystal, fracW, fracTracker;
  vector<float> absCrystal, absW, absTracker;
  vector<float> efEiTracker;

  auto fill = [](TH1F& hf, TH1F& ha, vector<float>& vf, vector<float>& va, float frac, float absLoss) {
    if (!isfinite(frac) || !isfinite(absLoss)) return;
    if (frac < 0) frac = 0;
    if (frac > 1) frac = 1;
    if (absLoss < 0) absLoss = 0;
    if (absLoss > 1) absLoss = 1;
    hf.Fill(frac);
    ha.Fill(absLoss);
    vf.push_back(frac);
    va.push_back(absLoss);
  };

  for (Long64_t entry = 0; entry < ch.GetEntries(); ++entry) {
    ch.GetEntry(entry);
    for (size_t i = 0; i < process_subtype->size(); ++i) {
      if (process_subtype->at(i) != 3) continue;
      if (track_id->at(i) != 1 || parent_id->at(i) != 0 || pdg->at(i) != 11) continue;

      const string& mat = material->at(i);
      const string& vol = pre_volume->at(i);
      const float p0 = pre_p->at(i);
      const float p1 = post_p->at(i);
      const float efEi = p0 > 0 ? p1 / p0 : 0.0f;
      const float frac = p0 > 0 ? 1.0f - efEi : 0.0f;
      const float absLoss = loss->at(i);

      if (isTrackerVolume(vol) && isfinite(efEi)) {
        hEfEiTrackerN.Fill(max(0.0f, min(1.0f, efEi)));
        efEiTracker.push_back(efEi);
      }

      if (isCrystalBar(mat, vol)) fill(hFracCrystal, hAbsCrystal, fracCrystal, absCrystal, frac, absLoss);
      else if (isWBeamPipeShell(mat, vol)) fill(hFracW, hAbsW, fracW, absW, frac, absLoss);
      else if (isTrackerVolume(vol)) fill(hFracTracker, hAbsTracker, fracTracker, absTracker, frac, absLoss);
    }
  }

  hFracCrystalN.Add(&hFracCrystal); hFracWN.Add(&hFracW); hFracTrackerN.Add(&hFracTracker);
  if (hFracCrystalN.Integral() > 0) hFracCrystalN.Scale(1.0 / hFracCrystalN.Integral());
  if (hFracWN.Integral() > 0) hFracWN.Scale(1.0 / hFracWN.Integral());
  if (hFracTrackerN.Integral() > 0) hFracTrackerN.Scale(1.0 / hFracTrackerN.Integral());
  if (hEfEiTrackerN.Integral() > 0) hEfEiTrackerN.Scale(1.0 / hEfEiTrackerN.Integral());

  TCanvas c1("c_loss_frac", "c_loss_frac", 900, 650);
  c1.SetLogy();
  hFracCrystal.SetMaximum(max({hFracCrystal.GetMaximum(), hFracW.GetMaximum(), hFracTracker.GetMaximum()}) * 2.0);
  hFracCrystal.Draw("hist"); hFracW.Draw("hist same"); hFracTracker.Draw("hist same");
  TLegend leg1(0.50, 0.70, 0.88, 0.86);
  leg1.AddEntry(&hFracCrystal, "BGO crystal bars", "l");
  leg1.AddEntry(&hFracW, "W beam-pipe shell", "l");
  leg1.AddEntry(&hFracTracker, "other tracker volumes", "l");
  leg1.Draw();
  saveCanvas(c1, plotdir, "primary_ebrem_loss_fraction_by_category");

  TCanvas c2("c_loss_abs", "c_loss_abs", 900, 650);
  c2.SetLogy();
  hAbsCrystal.SetMaximum(max({hAbsCrystal.GetMaximum(), hAbsW.GetMaximum(), hAbsTracker.GetMaximum()}) * 2.0);
  hAbsCrystal.Draw("hist"); hAbsW.Draw("hist same"); hAbsTracker.Draw("hist same");
  TLegend leg2(0.50, 0.70, 0.88, 0.86);
  leg2.AddEntry(&hAbsCrystal, "BGO crystal bars", "l");
  leg2.AddEntry(&hAbsW, "W beam-pipe shell", "l");
  leg2.AddEntry(&hAbsTracker, "other tracker volumes", "l");
  leg2.Draw();
  saveCanvas(c2, plotdir, "primary_ebrem_abs_loss_by_category");

  TCanvas c3("c_loss_frac_shape", "c_loss_frac_shape", 900, 650);
  c3.SetLogy();
  hFracCrystalN.SetMaximum(max({hFracCrystalN.GetMaximum(), hFracWN.GetMaximum(), hFracTrackerN.GetMaximum()}) * 2.0);
  hFracCrystalN.Draw("hist"); hFracWN.Draw("hist same"); hFracTrackerN.Draw("hist same");
  TLegend leg3(0.50, 0.70, 0.88, 0.86);
  leg3.AddEntry(&hFracCrystalN, "BGO crystal bars", "l");
  leg3.AddEntry(&hFracWN, "W beam-pipe shell", "l");
  leg3.AddEntry(&hFracTrackerN, "other tracker volumes", "l");
  leg3.Draw();
  saveCanvas(c3, plotdir, "primary_ebrem_loss_fraction_shape_by_category");

  drawOne(hFracCrystal, plotdir, "primary_ebrem_loss_fraction_crystal_bar");
  drawOne(hFracW, plotdir, "primary_ebrem_loss_fraction_w_beampipe_shell");
  drawOne(hFracTracker, plotdir, "primary_ebrem_loss_fraction_other_tracker");

  const double efEiMean = efEiTracker.empty() ? 0.0 : accumulate(efEiTracker.begin(), efEiTracker.end(), 0.0) / efEiTracker.size();
  TCanvas c4("c_tracker_efei", "c_tracker_efei", 900, 650);
  c4.SetLogy();
  hEfEiTrackerN.SetMaximum(hEfEiTrackerN.GetMaximum() * 2.0);
  hEfEiTrackerN.SetMinimum(1e-5);
  hEfEiTrackerN.Draw("hist");
  TPaveText box(0.16, 0.74, 0.45, 0.86, "NDC");
  box.SetFillColor(0);
  box.SetBorderSize(1);
  box.SetTextAlign(12);
  box.AddText(Form("N = %zu", efEiTracker.size()));
  box.AddText(Form("mean = %.4f", efEiMean));
  box.Draw();
  saveCanvas(c4, plotdir, "primary_tracker_ebrem_Ef_over_Ei_shape");
  TFile histFile((plotdir + "/primary_tracker_ebrem_Ef_over_Ei_shape.root").c_str(), "RECREATE");
  hEfEiTrackerN.SetName("h_primary_tracker_ebrem_Ef_over_Ei_shape");
  hEfEiTrackerN.Write();
  histFile.Close();

  ofstream out(string(outdir) + "/category_energy_loss_summary.txt");
  out << fixed << setprecision(6);
  auto write = [&](const char* name, const vector<float>& vf, const vector<float>& va) {
    const double meanF = vf.empty() ? 0.0 : accumulate(vf.begin(), vf.end(), 0.0) / vf.size();
    const double meanA = va.empty() ? 0.0 : accumulate(va.begin(), va.end(), 0.0) / va.size();
    long long hard1 = 0, hard10 = 0;
    for (float x : vf) { if (x > 0.01) ++hard1; if (x > 0.10) ++hard10; }
    out << name << " count " << vf.size()
        << " frac_loss_mean " << meanF
        << " frac_loss_q50 " << quantile(vf, 0.50)
        << " frac_loss_q90 " << quantile(vf, 0.90)
        << " frac_loss_q99 " << quantile(vf, 0.99)
        << " abs_loss_GeV_mean " << meanA
        << " abs_loss_GeV_q50 " << quantile(va, 0.50)
        << " abs_loss_GeV_q90 " << quantile(va, 0.90)
        << " hard_frac_gt_1pct " << hard1
        << " hard_frac_gt_10pct " << hard10 << "\n";
  };
  out << "Categories are mutually exclusive and restricted to primary electron eBrem.\n";
  out << "crystal_bar: material == G4_BGO and pre_volume starts with bar_s or contains crystal_s.\n";
  out << "w_beampipe_shell: material == G4_W and pre_volume contains BeamPipe_BeforeCryoW.\n";
  out << "other_tracker: pre_volume contains VXD/ITK/TPC/OTK/SIT/SET, excluding earlier categories.\n";
  out << "tracker_Ef_over_Ei_shape: primary electron eBrem with tracker-named pre_volume; histogram normalized to unit area.\n";
  out << "tracker_Ef_over_Ei_count " << efEiTracker.size()
      << " mean " << efEiMean
      << " q10 " << quantile(efEiTracker, 0.10)
      << " q50 " << quantile(efEiTracker, 0.50)
      << " q90 " << quantile(efEiTracker, 0.90) << "\n\n";
  write("crystal_bar", fracCrystal, absCrystal);
  write("w_beampipe_shell", fracW, absW);
  write("other_tracker", fracTracker, absTracker);
}
