#include <TCanvas.h>
#include <TChain.h>
#include <TH1F.h>
#include <TLegend.h>
#include <TPaveText.h>
#include <TStyle.h>
#include <TSystem.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <string>
#include <vector>
using namespace std;

namespace {
bool contains(const string& s, const string& needle) { return s.find(needle) != string::npos; }
bool isTrackerVolume(const string& volume) {
  return contains(volume, "VXD") || contains(volume, "ITK") || contains(volume, "TPC") ||
         contains(volume, "OTK") || contains(volume, "SIT") || contains(volume, "SET");
}
float quantile(vector<float> v, double p) {
  if (v.empty()) return NAN;
  sort(v.begin(), v.end());
  const double pos = p * (v.size() - 1);
  const int lo = floor(pos), hi = ceil(pos);
  if (lo == hi) return v[lo];
  return v[lo] + (v[hi] - v[lo]) * (pos - lo);
}
string statsLine(const char* name, const vector<float>& values) {
  const double mean = values.empty() ? 0.0 : accumulate(values.begin(), values.end(), 0.0) / values.size();
  return Form("%s: N=%zu, mean=%.2g GeV", name, values.size(), mean);
}
TPaveText* statsBox(double x1, double y1, double x2, double y2) {
  auto* box = new TPaveText(x1, y1, x2, y2, "NDC");
  box->SetFillColor(kWhite);
  box->SetFillStyle(1001);
  box->SetBorderSize(1);
  box->SetTextAlign(12);
  box->SetTextFont(42);
  box->SetTextSize(0.032);
  return box;
}
void save(TCanvas& c, const string& dir, const string& stem) {
  c.SaveAs((dir + "/" + stem + ".png").c_str());
  c.SaveAs((dir + "/" + stem + ".pdf").c_str());
}
}

void plot_electron_muon_ioni_tracker_loss(const char* outdir = "G4MaterialStepComparison/studies/e1p0_theta85") {
  gSystem->mkdir(outdir, true);
  const string plotdir = string(outdir) + "/plots";
  gSystem->mkdir(plotdir.c_str(), true);
  gStyle->SetOptStat(0);

  auto fillSample = [&](const char* pattern, int primaryPdg, TH1F& h, vector<float>& values) {
    TChain ch("g4step_tuple");
    for (int i = 1; i <= 10; ++i) ch.Add(Form(pattern, i));
    vector<int>* track_id = nullptr;
    vector<int>* parent_id = nullptr;
    vector<int>* pdg = nullptr;
    vector<int>* process_subtype = nullptr;
    vector<float>* loss = nullptr;
    vector<string>* pre_volume = nullptr;
    ch.SetBranchAddress("track_id", &track_id);
    ch.SetBranchAddress("parent_id", &parent_id);
    ch.SetBranchAddress("pdg", &pdg);
    ch.SetBranchAddress("process_subtype", &process_subtype);
    ch.SetBranchAddress("loss", &loss);
    ch.SetBranchAddress("pre_volume", &pre_volume);
    for (Long64_t entry = 0; entry < ch.GetEntries(); ++entry) {
      ch.GetEntry(entry);
      for (size_t i = 0; i < process_subtype->size(); ++i) {
        if (track_id->at(i) != 1 || parent_id->at(i) != 0 || pdg->at(i) != primaryPdg) continue;
        if (process_subtype->at(i) != 2) continue; // eIoni or muIoni, depending on particle sample
        if (!isTrackerVolume(pre_volume->at(i))) continue;
        float x = loss->at(i);
        if (!isfinite(x)) continue;
        if (x < 0) x = 0;
        values.push_back(x);
        h.Fill(x > 0.005f ? 0.00499f : x);
      }
    }
  };

  TH1F hE("hE", "Tracker ionization loss: e^{-} vs #mu^{-};loss branch [GeV];steps", 120, 0, 0.005);
  TH1F hMu("hMu", "Tracker ionization loss: e^{-} vs #mu^{-};loss branch [GeV];steps", 120, 0, 0.005);
  TH1F hEShape("hEShape", "Tracker ionization loss, shape-normalized;loss branch [GeV];normalized steps", 120, 0, 0.005);
  TH1F hMuShape("hMuShape", "Tracker ionization loss, shape-normalized;loss branch [GeV];normalized steps", 120, 0, 0.005);
  hE.SetLineColor(kBlack); hE.SetLineWidth(2);
  hMu.SetLineColor(kBlue + 1); hMu.SetLineWidth(2);
  hEShape.SetLineColor(kBlack); hEShape.SetLineWidth(2);
  hMuShape.SetLineColor(kBlue + 1); hMuShape.SetLineWidth(2);

  vector<float> eLoss, muLoss;
  fillSample("gsf_material_steps-e--1.0-85-%d.root", 11, hE, eLoss);
  fillSample("gsf_material_steps-mu--1.0-85-%d.root", 13, hMu, muLoss);
  hEShape.Add(&hE); hMuShape.Add(&hMu);
  if (hEShape.Integral() > 0) hEShape.Scale(1.0 / hEShape.Integral());
  if (hMuShape.Integral() > 0) hMuShape.Scale(1.0 / hMuShape.Integral());

  TCanvas c1("c_e_mu_ioni_counts", "c_e_mu_ioni_counts", 900, 650);
  c1.SetLogy();
  hE.SetMaximum(max(hE.GetMaximum(), hMu.GetMaximum()) * 2.0);
  hE.Draw("hist"); hMu.Draw("hist same");
  TLegend leg1(0.58, 0.74, 0.88, 0.86);
  leg1.AddEntry(&hE, "e^{-} eIoni", "l");
  leg1.AddEntry(&hMu, "#mu^{-} muIoni", "l");
  leg1.Draw();
  auto* st1 = statsBox(0.50, 0.58, 0.88, 0.71);
  st1->AddText(statsLine("eIoni", eLoss).c_str());
  st1->AddText(statsLine("muIoni", muLoss).c_str());
  st1->Draw();
  save(c1, plotdir, "tracker_electron_muon_ioni_loss_counts");

  TCanvas c2("c_e_mu_ioni_shape", "c_e_mu_ioni_shape", 900, 650);
  c2.SetLogy();
  hEShape.SetMaximum(max(hEShape.GetMaximum(), hMuShape.GetMaximum()) * 2.0);
  hEShape.Draw("hist"); hMuShape.Draw("hist same");
  TLegend leg2(0.58, 0.74, 0.88, 0.86);
  leg2.AddEntry(&hEShape, "e^{-} eIoni", "l");
  leg2.AddEntry(&hMuShape, "#mu^{-} muIoni", "l");
  leg2.Draw();
  auto* st2 = statsBox(0.50, 0.58, 0.88, 0.71);
  st2->AddText(statsLine("eIoni", eLoss).c_str());
  st2->AddText(statsLine("muIoni", muLoss).c_str());
  st2->Draw();
  save(c2, plotdir, "tracker_electron_muon_ioni_loss_shape");

  ofstream out(string(outdir) + "/tracker_electron_muon_ioni_summary.txt");
  out << fixed << setprecision(8);
  out << "Selection: primary charged particle ionization steps in tracker-named volumes. Loss variable: branch `loss` in GeV.\n";
  out << "electron: gsf_material_steps-e--1.0-85-{1..10}.root, pdg==11, process_subtype==2 (eIoni).\n";
  out << "muon: gsf_material_steps-mu--1.0-85-{1..10}.root, pdg==13, process_subtype==2 (muIoni).\n\n";
  auto write = [&](const char* name, const vector<float>& v) {
    long long nonzero = 0;
    for (float x : v) if (x > 0) ++nonzero;
    const double mean = v.empty() ? 0.0 : accumulate(v.begin(), v.end(), 0.0) / v.size();
    out << name << " count " << v.size()
        << " nonzero " << nonzero
        << " mean " << mean
        << " q50 " << quantile(v, 0.50)
        << " q90 " << quantile(v, 0.90)
        << " q99 " << quantile(v, 0.99)
        << " max " << (v.empty() ? 0.0f : *max_element(v.begin(), v.end())) << "\n";
  };
  write("eIoni", eLoss);
  write("muIoni", muLoss);
}
