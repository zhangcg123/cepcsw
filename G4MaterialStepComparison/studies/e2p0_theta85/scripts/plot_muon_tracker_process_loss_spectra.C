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
  box->SetTextSize(0.030);
  return box;
}
void style(TH1F& h, int color, int lineStyle = 1) {
  h.SetLineColor(color);
  h.SetLineWidth(2);
  h.SetLineStyle(lineStyle);
}
void save(TCanvas& c, const string& dir, const string& stem) {
  c.SaveAs((dir + "/" + stem + ".png").c_str());
  c.SaveAs((dir + "/" + stem + ".pdf").c_str());
}
}

void plot_muon_tracker_process_loss_spectra(const char* outdir = "G4MaterialStepComparison/studies/e2p0_theta85") {
  gSystem->mkdir(outdir, true);
  const string plotdir = string(outdir) + "/plots";
  gSystem->mkdir(plotdir.c_str(), true);
  gStyle->SetOptStat(0);

  TChain ch("g4step_tuple");
  for (int i = 1; i <= 10; ++i) ch.Add(Form("gsf_material_steps-mu--2.0-85-%d.root", i));

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

  TH1F hMuIoni("hMuIoni", "Primary #mu^{-} tracker-volume loss;loss branch [GeV];steps", 160, 0, 0.02);
  TH1F hStepLimiter("hStepLimiter", "Primary #mu^{-} tracker-volume loss;loss branch [GeV];steps", 160, 0, 0.02);
  TH1F hTransportation("hTransportation", "Primary #mu^{-} tracker-volume loss;loss branch [GeV];steps", 160, 0, 0.02);
  TH1F hOther("hOther", "Primary #mu^{-} tracker-volume loss;loss branch [GeV];steps", 160, 0, 0.02);
  TH1F hMuIoniShape("hMuIoniShape", "Primary #mu^{-} tracker-volume loss, shape-normalized;loss branch [GeV];normalized steps", 160, 0, 0.02);
  TH1F hStepLimiterShape("hStepLimiterShape", "Primary #mu^{-} tracker-volume loss, shape-normalized;loss branch [GeV];normalized steps", 160, 0, 0.02);
  TH1F hTransportationShape("hTransportationShape", "Primary #mu^{-} tracker-volume loss, shape-normalized;loss branch [GeV];normalized steps", 160, 0, 0.02);
  TH1F hOtherShape("hOtherShape", "Primary #mu^{-} tracker-volume loss, shape-normalized;loss branch [GeV];normalized steps", 160, 0, 0.02);

  style(hMuIoni, kBlack);
  style(hStepLimiter, kBlue + 1);
  style(hTransportation, kCyan + 2);
  style(hOther, kGray + 2, 7);
  style(hMuIoniShape, kBlack);
  style(hStepLimiterShape, kBlue + 1);
  style(hTransportationShape, kCyan + 2);
  style(hOtherShape, kGray + 2, 7);

  vector<float> vMuIoni, vStepLimiter, vTransportation, vOther;
  auto fill = [](TH1F& h, vector<float>& v, float x) {
    if (!isfinite(x)) return;
    if (x < 0) x = 0;
    h.Fill(x > 0.02f ? 0.01999f : x);
    v.push_back(x);
  };

  for (Long64_t entry = 0; entry < ch.GetEntries(); ++entry) {
    ch.GetEntry(entry);
    for (size_t i = 0; i < process_subtype->size(); ++i) {
      if (track_id->at(i) != 1 || parent_id->at(i) != 0 || pdg->at(i) != 13) continue;
      if (!isTrackerVolume(pre_volume->at(i))) continue;
      const int proc = process_subtype->at(i);
      const float x = loss->at(i);
      if (proc == 2) fill(hMuIoni, vMuIoni, x);              // muIoni in muon sample
      else if (proc == 401) fill(hStepLimiter, vStepLimiter, x);
      else if (proc == 91) fill(hTransportation, vTransportation, x);
      else fill(hOther, vOther, x);
    }
  }

  hMuIoniShape.Add(&hMuIoni);
  hStepLimiterShape.Add(&hStepLimiter);
  hTransportationShape.Add(&hTransportation);
  hOtherShape.Add(&hOther);
  for (TH1F* h : {&hMuIoniShape, &hStepLimiterShape, &hTransportationShape, &hOtherShape}) {
    if (h->Integral() > 0) h->Scale(1.0 / h->Integral());
  }

  TCanvas c1("c_mu_loss_counts", "c_mu_loss_counts", 950, 680);
  c1.SetLogy();
  hTransportation.SetMaximum(max({hTransportation.GetMaximum(), hMuIoni.GetMaximum(), hStepLimiter.GetMaximum(), hOther.GetMaximum()}) * 2.0);
  hTransportation.Draw("hist"); hMuIoni.Draw("hist same"); hStepLimiter.Draw("hist same"); hOther.Draw("hist same");
  TLegend leg1(0.55, 0.70, 0.88, 0.87);
  leg1.AddEntry(&hMuIoni, "muIoni", "l");
  leg1.AddEntry(&hStepLimiter, "StepLimiter", "l");
  leg1.AddEntry(&hTransportation, "Transportation", "l");
  leg1.AddEntry(&hOther, "other", "l");
  leg1.Draw();
  auto* st1 = statsBox(0.50, 0.50, 0.88, 0.67);
  st1->AddText(statsLine("muIoni", vMuIoni).c_str());
  st1->AddText(statsLine("StepLimiter", vStepLimiter).c_str());
  st1->AddText(statsLine("Transportation", vTransportation).c_str());
  st1->AddText(statsLine("other", vOther).c_str());
  st1->Draw();
  save(c1, plotdir, "muon_tracker_primary_loss_by_process_counts");

  TCanvas c2("c_mu_loss_shape", "c_mu_loss_shape", 950, 680);
  c2.SetLogy();
  hTransportationShape.SetMaximum(max({hTransportationShape.GetMaximum(), hMuIoniShape.GetMaximum(), hStepLimiterShape.GetMaximum(), hOtherShape.GetMaximum()}) * 2.0);
  hTransportationShape.Draw("hist"); hMuIoniShape.Draw("hist same"); hStepLimiterShape.Draw("hist same"); hOtherShape.Draw("hist same");
  TLegend leg2(0.55, 0.70, 0.88, 0.87);
  leg2.AddEntry(&hMuIoniShape, "muIoni", "l");
  leg2.AddEntry(&hStepLimiterShape, "StepLimiter", "l");
  leg2.AddEntry(&hTransportationShape, "Transportation", "l");
  leg2.AddEntry(&hOtherShape, "other", "l");
  leg2.Draw();
  auto* st2 = statsBox(0.50, 0.50, 0.88, 0.67);
  st2->AddText(statsLine("muIoni", vMuIoni).c_str());
  st2->AddText(statsLine("StepLimiter", vStepLimiter).c_str());
  st2->AddText(statsLine("Transportation", vTransportation).c_str());
  st2->AddText(statsLine("other", vOther).c_str());
  st2->Draw();
  save(c2, plotdir, "muon_tracker_primary_loss_by_process_shape");

  auto drawOne = [&](TH1F& h, const vector<float>& values, const char* label, const char* stem) {
    TCanvas c(stem, stem, 900, 650);
    c.SetLogy();
    h.Draw("hist");
    auto* st = statsBox(0.55, 0.78, 0.88, 0.87);
    st->AddText(statsLine(label, values).c_str());
    st->Draw();
    save(c, plotdir, stem);
  };
  drawOne(hMuIoni, vMuIoni, "muIoni", "muon_tracker_primary_loss_muIoni");
  drawOne(hStepLimiter, vStepLimiter, "StepLimiter", "muon_tracker_primary_loss_step_limiter");
  drawOne(hTransportation, vTransportation, "Transportation", "muon_tracker_primary_loss_transportation");
  drawOne(hOther, vOther, "other", "muon_tracker_primary_loss_other");

  ofstream out(string(outdir) + "/muon_tracker_process_loss_summary.txt");
  out << fixed << setprecision(8);
  out << "Selection: primary muon steps in tracker-named volumes. Loss variable: branch `loss` in GeV.\n";
  out << "Muon files: gsf_material_steps-mu--2.0-85-{1..10}.root. Primary muon: track_id==1 && parent_id==0 && pdg==13.\n";
  out << "Tracker volume name contains VXD/ITK/TPC/OTK/SIT/SET.\n\n";
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
  write("muIoni", vMuIoni);
  write("StepLimiter", vStepLimiter);
  write("Transportation", vTransportation);
  write("other", vOther);
}
