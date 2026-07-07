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
#include <iostream>
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
string procName(int s) {
  if (s == 3) return "eBrem";
  if (s == 2) return "eIoni";
  if (s == 10) return "msc";
  if (s == 91) return "Transportation";
  if (s == 401) return "StepLimiter";
  return to_string(s);
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
void style(TH1F& h, int color, int style = 1) {
  h.SetLineColor(color);
  h.SetLineWidth(2);
  h.SetLineStyle(style);
}
void save(TCanvas& c, const string& dir, const string& stem) {
  c.SaveAs((dir + "/" + stem + ".png").c_str());
  c.SaveAs((dir + "/" + stem + ".pdf").c_str());
}

string statsLine(const char* name, const vector<float>& values) {
  const double mean = values.empty() ? 0.0 : accumulate(values.begin(), values.end(), 0.0) / values.size();
  return Form("%s: N=%zu, mean=%.2g GeV", name, values.size(), mean);
}

TPaveText* makeStatsBox(double x1, double y1, double x2, double y2) {
  auto* box = new TPaveText(x1, y1, x2, y2, "NDC");
  box->SetFillColor(kWhite);
  box->SetFillStyle(1001);
  box->SetBorderSize(1);
  box->SetTextAlign(12);
  box->SetTextFont(42);
  box->SetTextSize(0.030);
  return box;
}
}

void plot_tracker_process_loss_spectra(const char* outdir = "G4MaterialStepComparison/studies/e2p0_theta85") {
  gSystem->mkdir(outdir, true);
  const string plotdir = string(outdir) + "/plots";
  gSystem->mkdir(plotdir.c_str(), true);
  gStyle->SetOptStat(0);

  TChain ch("g4step_tuple");
  for (int i = 1; i <= 10; ++i) ch.Add(Form("gsf_material_steps-e--2.0-85-%d.root", i));

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

  TH1F hEBrem("hEBrem", "Primary electron tracker-volume loss;loss branch [GeV];steps", 160, 0, 0.20);
  TH1F hEIoni("hEIoni", "Primary electron tracker-volume loss;loss branch [GeV];steps", 160, 0, 0.20);
  TH1F hMSC("hMSC", "Primary electron tracker-volume loss;loss branch [GeV];steps", 160, 0, 0.20);
  TH1F hStepLimiter("hStepLimiter", "Primary electron tracker-volume loss;loss branch [GeV];steps", 160, 0, 0.20);
  TH1F hTransportation("hTransportation", "Primary electron tracker-volume loss;loss branch [GeV];steps", 160, 0, 0.20);

  TH1F hEBremShape("hEBremShape", "Primary electron tracker-volume loss, shape-normalized;loss branch [GeV];normalized steps", 160, 0, 0.20);
  TH1F hEIoniShape("hEIoniShape", "Primary electron tracker-volume loss, shape-normalized;loss branch [GeV];normalized steps", 160, 0, 0.20);
  TH1F hMSCShape("hMSCShape", "Primary electron tracker-volume loss, shape-normalized;loss branch [GeV];normalized steps", 160, 0, 0.20);
  TH1F hStepLimiterShape("hStepLimiterShape", "Primary electron tracker-volume loss, shape-normalized;loss branch [GeV];normalized steps", 160, 0, 0.20);
  TH1F hTransportationShape("hTransportationShape", "Primary electron tracker-volume loss, shape-normalized;loss branch [GeV];normalized steps", 160, 0, 0.20);

  style(hEBrem, kBlack);
  style(hEIoni, kBlue + 1);
  style(hMSC, kCyan + 2);
  style(hStepLimiter, kMagenta + 2, 2);
  style(hTransportation, kGray + 2, 7);
  style(hEBremShape, kBlack);
  style(hEIoniShape, kBlue + 1);
  style(hMSCShape, kCyan + 2);
  style(hStepLimiterShape, kMagenta + 2, 2);
  style(hTransportationShape, kGray + 2, 7);

  vector<float> vEBrem, vEIoni, vMSC, vStepLimiter, vTransportation;
  auto fill = [](TH1F& h, vector<float>& v, float x) {
    if (!isfinite(x)) return;
    if (x < 0) x = 0;
    h.Fill(x > 0.20f ? 0.1999f : x);
    v.push_back(x);
  };

  for (Long64_t entry = 0; entry < ch.GetEntries(); ++entry) {
    ch.GetEntry(entry);
    for (size_t i = 0; i < process_subtype->size(); ++i) {
      if (track_id->at(i) != 1 || parent_id->at(i) != 0 || pdg->at(i) != 11) continue;
      if (!isTrackerVolume(pre_volume->at(i))) continue;
      const int proc = process_subtype->at(i);
      const float x = loss->at(i);
      if (proc == 3) fill(hEBrem, vEBrem, x);
      else if (proc == 2) fill(hEIoni, vEIoni, x);
      else if (proc == 10) fill(hMSC, vMSC, x);
      else if (proc == 401) fill(hStepLimiter, vStepLimiter, x);
      else if (proc == 91) fill(hTransportation, vTransportation, x);
    }
  }

  hEBremShape.Add(&hEBrem); hEIoniShape.Add(&hEIoni); hMSCShape.Add(&hMSC); hStepLimiterShape.Add(&hStepLimiter); hTransportationShape.Add(&hTransportation);
  for (TH1F* h : {&hEBremShape, &hEIoniShape, &hMSCShape, &hStepLimiterShape, &hTransportationShape}) {
    if (h->Integral() > 0) h->Scale(1.0 / h->Integral());
  }

  TCanvas c1("c_tracker_loss_counts", "c_tracker_loss_counts", 950, 680);
  c1.SetLogy();
  hTransportation.SetMaximum(max({hTransportation.GetMaximum(), hEIoni.GetMaximum(), hMSC.GetMaximum(), hStepLimiter.GetMaximum(), hEBrem.GetMaximum()}) * 2.0);
  hTransportation.Draw("hist"); hEIoni.Draw("hist same"); hMSC.Draw("hist same"); hStepLimiter.Draw("hist same"); hEBrem.Draw("hist same");
  TLegend leg1(0.52, 0.66, 0.88, 0.88);
  leg1.AddEntry(&hEBrem, "eBrem", "l");
  leg1.AddEntry(&hEIoni, "eIoni", "l");
  leg1.AddEntry(&hMSC, "msc", "l");
  leg1.AddEntry(&hStepLimiter, "StepLimiter", "l");
  leg1.AddEntry(&hTransportation, "Transportation", "l");
  leg1.Draw();
  auto* stats1 = makeStatsBox(0.50, 0.43, 0.88, 0.63);
  stats1->AddText(statsLine("eBrem", vEBrem).c_str());
  stats1->AddText(statsLine("eIoni", vEIoni).c_str());
  stats1->AddText(statsLine("msc", vMSC).c_str());
  stats1->AddText(statsLine("StepLimiter", vStepLimiter).c_str());
  stats1->AddText(statsLine("Transportation", vTransportation).c_str());
  stats1->Draw();
  save(c1, plotdir, "tracker_primary_loss_by_process_counts");

  TCanvas c2("c_tracker_loss_shape", "c_tracker_loss_shape", 950, 680);
  c2.SetLogy();
  hTransportationShape.SetMaximum(max({hTransportationShape.GetMaximum(), hEIoniShape.GetMaximum(), hMSCShape.GetMaximum(), hStepLimiterShape.GetMaximum(), hEBremShape.GetMaximum()}) * 2.0);
  hTransportationShape.Draw("hist"); hEIoniShape.Draw("hist same"); hMSCShape.Draw("hist same"); hStepLimiterShape.Draw("hist same"); hEBremShape.Draw("hist same");
  TLegend leg2(0.52, 0.66, 0.88, 0.88);
  leg2.AddEntry(&hEBremShape, "eBrem", "l");
  leg2.AddEntry(&hEIoniShape, "eIoni", "l");
  leg2.AddEntry(&hMSCShape, "msc", "l");
  leg2.AddEntry(&hStepLimiterShape, "StepLimiter", "l");
  leg2.AddEntry(&hTransportationShape, "Transportation", "l");
  leg2.Draw();
  auto* stats2 = makeStatsBox(0.50, 0.43, 0.88, 0.63);
  stats2->AddText(statsLine("eBrem", vEBrem).c_str());
  stats2->AddText(statsLine("eIoni", vEIoni).c_str());
  stats2->AddText(statsLine("msc", vMSC).c_str());
  stats2->AddText(statsLine("StepLimiter", vStepLimiter).c_str());
  stats2->AddText(statsLine("Transportation", vTransportation).c_str());
  stats2->Draw();
  save(c2, plotdir, "tracker_primary_loss_by_process_shape");

  auto drawOne = [&](TH1F& h, const vector<float>& values, const char* label, const char* stem) {
    TCanvas c(stem, stem, 900, 650);
    c.SetLogy();
    h.Draw("hist");
    auto* stats = makeStatsBox(0.55, 0.78, 0.88, 0.87);
    stats->AddText(statsLine(label, values).c_str());
    stats->Draw();
    save(c, plotdir, stem);
  };
  drawOne(hEIoni, vEIoni, "eIoni", "tracker_primary_loss_eIoni");
  drawOne(hMSC, vMSC, "msc", "tracker_primary_loss_msc");
  drawOne(hStepLimiter, vStepLimiter, "StepLimiter", "tracker_primary_loss_step_limiter");
  drawOne(hTransportation, vTransportation, "Transportation", "tracker_primary_loss_transportation");
  drawOne(hEBrem, vEBrem, "eBrem", "tracker_primary_loss_eBrem_reference");

  ofstream out(string(outdir) + "/tracker_process_loss_summary.txt");
  out << fixed << setprecision(8);
  out << "Selection: primary electron steps in tracker-named volumes. Loss variable: branch `loss` in GeV.\n";
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
  write("eBrem", vEBrem);
  write("eIoni", vEIoni);
  write("msc", vMSC);
  write("StepLimiter", vStepLimiter);
  write("Transportation", vTransportation);
}
