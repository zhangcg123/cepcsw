#include <TCanvas.h>
#include <TChain.h>
#include <TFile.h>
#include <TH1F.h>
#include <TLine.h>
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
double quantile(vector<double> v, double p) {
  if (v.empty()) return NAN;
  sort(v.begin(), v.end());
  const double pos = p * (v.size() - 1);
  const int lo = floor(pos);
  const int hi = ceil(pos);
  if (lo == hi) return v[lo];
  return v[lo] + (v[hi] - v[lo]) * (pos - lo);
}
}

void analyze_tracker_ebrem_efei(const char* outdir = "BHModelComparisonStudies/globalBHmodelfromSim@2GeV85Degree") {
  gSystem->mkdir(outdir, true);
  const string plotdir = string(outdir) + "/plots";
  gSystem->mkdir(plotdir.c_str(), true);

  TChain ch("g4step_tuple");
  for (int seed = 1; seed <= 15; ++seed) ch.Add(Form("gsf_material_steps-e--2.0-85-%d.root", seed));

  vector<int>* track_id = nullptr;
  vector<int>* parent_id = nullptr;
  vector<int>* pdg = nullptr;
  vector<int>* process_subtype = nullptr;
  vector<float>* pre_p = nullptr;
  vector<float>* post_p = nullptr;
  vector<string>* pre_volume = nullptr;

  ch.SetBranchAddress("track_id", &track_id);
  ch.SetBranchAddress("parent_id", &parent_id);
  ch.SetBranchAddress("pdg", &pdg);
  ch.SetBranchAddress("process_subtype", &process_subtype);
  ch.SetBranchAddress("pre_p", &pre_p);
  ch.SetBranchAddress("post_p", &post_p);
  ch.SetBranchAddress("pre_volume", &pre_volume);

  TH1F hCounts("h_tracker_ebrem_Ef_over_Ei_counts", "Primary tracker eBrem;E_{f}/E_{i};steps", 120, 0.0, 1.0);
  TH1F hShape("h_tracker_ebrem_Ef_over_Ei_shape", "Primary tracker eBrem;E_{f}/E_{i};normalized steps", 120, 0.0, 1.0);
  vector<double> values;

  for (Long64_t entry = 0; entry < ch.GetEntries(); ++entry) {
    ch.GetEntry(entry);
    for (size_t i = 0; i < process_subtype->size(); ++i) {
      if (track_id->at(i) != 1 || parent_id->at(i) != 0 || pdg->at(i) != 11) continue;
      if (process_subtype->at(i) != 3) continue;
      if (!isTrackerVolume(pre_volume->at(i))) continue;
      const double p0 = pre_p->at(i);
      const double p1 = post_p->at(i);
      if (p0 <= 0.0) continue;
      const double z = p1 / p0;
      if (!isfinite(z)) continue;
      values.push_back(z);
      const double zfill = max(0.0, min(0.999999, z));
      hCounts.Fill(zfill);
      hShape.Fill(zfill);
    }
  }

  if (hShape.Integral() > 0) hShape.Scale(1.0 / hShape.Integral());

  const long long total = values.size();
  long long lt09 = 0, eq09 = 0, gt09 = 0, ge09 = 0;
  for (double z : values) {
    if (z < 0.9) ++lt09;
    if (z == 0.9) ++eq09;
    if (z > 0.9) ++gt09;
    if (z >= 0.9) ++ge09;
  }
  const double mean = values.empty() ? NAN : accumulate(values.begin(), values.end(), 0.0) / values.size();
  const double fracLt = total ? double(lt09) / total : NAN;
  const double fracGt = total ? double(gt09) / total : NAN;
  const double fracGe = total ? double(ge09) / total : NAN;

  ofstream out(string(outdir) + "/tracker_ebrem_efei_fraction_summary.txt");
  out << fixed << setprecision(9);
  out << "Tracker eBrem retained-fraction split for electron 2 GeV, theta=85 deg\n";
  out << "selection primary electron eBrem steps in tracker-named pre_volume VXD/ITK/TPC/OTK/SIT/SET\n";
  out << "input gsf_material_steps-e--2.0-85-{1..15}.root\n";
  out << "variable E_f/E_i = post_p/pre_p\n";
  out << "total " << total << "\n";
  out << "count_lt_0p9 " << lt09 << "\n";
  out << "count_eq_0p9 " << eq09 << "\n";
  out << "count_gt_0p9 " << gt09 << "\n";
  out << "count_ge_0p9 " << ge09 << "\n";
  out << "fraction_lt_0p9 " << fracLt << "\n";
  out << "fraction_gt_0p9 " << fracGt << "\n";
  out << "fraction_ge_0p9 " << fracGe << "\n";
  out << "mean " << mean << "\n";
  out << "q10 " << quantile(values, 0.10) << "\n";
  out << "q50 " << quantile(values, 0.50) << "\n";
  out << "q90 " << quantile(values, 0.90) << "\n";

  ofstream csv(string(outdir) + "/tracker_ebrem_efei_values.csv");
  csv << "Ef_over_Ei\n";
  csv << setprecision(9);
  for (double z : values) csv << z << "\n";

  gStyle->SetOptStat(0);
  TCanvas c("c_tracker_ebrem_efei_fraction", "c_tracker_ebrem_efei_fraction", 950, 680);
  c.SetLogy();
  hShape.SetLineColor(kBlack);
  hShape.SetLineWidth(2);
  hShape.SetMinimum(1e-5);
  hShape.SetMaximum(hShape.GetMaximum() * 2.4);
  hShape.Draw("hist");
  TLine line(0.9, hShape.GetMinimum(), 0.9, hShape.GetMaximum() / 1.15);
  line.SetLineColor(kRed + 1);
  line.SetLineWidth(2);
  line.SetLineStyle(2);
  line.Draw();
  TPaveText box(0.16, 0.70, 0.50, 0.88, "NDC");
  box.SetFillColor(kWhite);
  box.SetFillStyle(1001);
  box.SetBorderSize(1);
  box.SetTextAlign(12);
  box.SetTextFont(42);
  box.SetTextSize(0.030);
  box.AddText(Form("N = %lld", total));
  box.AddText(Form("z < 0.9: %lld (%.3f%%)", lt09, 100.0 * fracLt));
  box.AddText(Form("z > 0.9: %lld (%.3f%%)", gt09, 100.0 * fracGt));
  box.AddText(Form("mean = %.4f", mean));
  box.Draw();
  c.SaveAs((plotdir + "/tracker_ebrem_Ef_over_Ei_fraction_split.png").c_str());
  c.SaveAs((plotdir + "/tracker_ebrem_Ef_over_Ei_fraction_split.pdf").c_str());

  TFile fout((string(outdir) + "/tracker_ebrem_Ef_over_Ei_fraction_split.root").c_str(), "RECREATE");
  hCounts.Write();
  hShape.Write();
  fout.Close();

  cout << "total=" << total << " lt0.9=" << lt09 << " frac_lt0.9=" << fracLt
       << " gt0.9=" << gt09 << " frac_gt0.9=" << fracGt << endl;
}
