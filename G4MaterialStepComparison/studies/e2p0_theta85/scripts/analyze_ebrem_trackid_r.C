#include <TCanvas.h>
#include <TChain.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TSystem.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

namespace {
string procName(int s) {
  if (s == 3) return "eBrem";
  if (s == 2) return "eIoni";
  if (s == 10) return "msc";
  if (s == 91) return "Transportation";
  if (s == 401) return "StepLimiter";
  if (s == 12) return "phot";
  if (s == 13) return "compt";
  if (s == 14) return "conv";
  if (s == 11) return "Rayl";
  if (s == 5) return "annihil";
  if (s == 121) return "photonNuclear";
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

template <typename K>
vector<pair<K, long long>> sortedCounts(const map<K, long long>& m) {
  vector<pair<K, long long>> v(m.begin(), m.end());
  sort(v.begin(), v.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
  return v;
}

void saveCanvas(TCanvas& c, const string& outdir, const string& stem) {
  c.SaveAs((outdir + "/" + stem + ".png").c_str());
  c.SaveAs((outdir + "/" + stem + ".pdf").c_str());
}
}

void analyze_ebrem_trackid_r(const char* outdir = "G4MaterialStepComparison/studies/e2p0_theta85") {
  gSystem->mkdir(outdir, true);
  const string plotdir = string(outdir) + "/plots";
  gSystem->mkdir(plotdir.c_str(), true);
  gStyle->SetOptStat(0);
  gStyle->SetPalette(kViridis);

  TChain ch("g4step_tuple");
  for (int i = 1; i <= 10; ++i) ch.Add(Form("gsf_material_steps-e--2.0-85-%d.root", i));

  int event_id = 0;
  vector<int>* track_id = nullptr;
  vector<int>* parent_id = nullptr;
  vector<int>* pdg = nullptr;
  vector<int>* process_subtype = nullptr;
  vector<float>* mid_r = nullptr;
  vector<float>* mid_z = nullptr;
  vector<float>* pre_p = nullptr;
  vector<float>* post_p = nullptr;
  vector<float>* step_tX0 = nullptr;
  vector<float>* loss = nullptr;
  vector<string>* material = nullptr;
  vector<string>* pre_volume = nullptr;

  ch.SetBranchAddress("event_id", &event_id);
  ch.SetBranchAddress("track_id", &track_id);
  ch.SetBranchAddress("parent_id", &parent_id);
  ch.SetBranchAddress("pdg", &pdg);
  ch.SetBranchAddress("process_subtype", &process_subtype);
  ch.SetBranchAddress("mid_r", &mid_r);
  ch.SetBranchAddress("mid_z", &mid_z);
  ch.SetBranchAddress("pre_p", &pre_p);
  ch.SetBranchAddress("post_p", &post_p);
  ch.SetBranchAddress("step_tX0", &step_tX0);
  ch.SetBranchAddress("loss", &loss);
  ch.SetBranchAddress("material", &material);
  ch.SetBranchAddress("pre_volume", &pre_volume);

  TH1F hPrimaryR("hPrimaryR", "Primary e^{-} eBrem;mid R [mm];steps", 240, 0, 2400);
  TH1F hPrimaryRZoom("hPrimaryRZoom", "Primary e^{-} eBrem, inner/MDI zoom;mid R [mm];steps / 2 mm", 125, 0, 250);
  TH1F hAllR("hAllR", "All eBrem;mid R [mm];steps", 240, 0, 2400);
  TH2F hPrimaryRZ("hPrimaryRZ", "Primary e^{-} eBrem;mid z [mm];mid R [mm]", 240, 0, 1200, 240, 0, 2400);
  TH2F hR100RZ("hR100RZ", "Primary e^{-} eBrem with 60 < R < 120 mm;mid z [mm];mid R [mm]", 160, 0, 1300, 80, 40, 140);
  TH1F hR100Material("hR100Material", "Primary e^{-} eBrem, 60 < R < 120 mm;material rank;steps", 8, 0.5, 8.5);

  long long events = ch.GetEntries();
  long long steps = 0;
  long long allEBrem = 0;
  long long primaryEBrem = 0;
  long long primarySameTrackAfter = 0;
  long long primaryNoSameTrackAfter = 0;
  long long primaryImmediateSame = 0;
  long long primaryImmediateDifferent = 0;
  long long allSameTrackAfter = 0;
  long long allNoSameTrackAfter = 0;
  long long r100Count = 0;
  long long r100Hard1 = 0;
  long long r100Hard10 = 0;

  map<int, long long> primaryNextProc;
  map<string, long long> r100Materials;
  map<string, long long> r100Volumes;
  vector<float> primaryR;
  vector<float> allR;
  vector<float> r100R;
  vector<float> r100Z;
  vector<float> r100TX0;
  vector<float> r100Loss;

  for (Long64_t entry = 0; entry < events; ++entry) {
    ch.GetEntry(entry);
    const int n = process_subtype->size();
    steps += n;

    vector<int> nextSameIdx(n, -1);
    unordered_map<int, int> lastTrackIndex;
    for (int i = n - 1; i >= 0; --i) {
      const int tid = track_id->at(i);
      const auto it = lastTrackIndex.find(tid);
      if (it != lastTrackIndex.end()) nextSameIdx[i] = it->second;
      lastTrackIndex[tid] = i;
    }

    for (int i = 0; i < n; ++i) {
      if (process_subtype->at(i) != 3) continue;
      ++allEBrem;
      allR.push_back(mid_r->at(i));
      hAllR.Fill(mid_r->at(i));
      if (nextSameIdx[i] >= 0) ++allSameTrackAfter;
      else ++allNoSameTrackAfter;

      const bool primaryElectron = track_id->at(i) == 1 && parent_id->at(i) == 0 && pdg->at(i) == 11;
      if (!primaryElectron) continue;
      ++primaryEBrem;
      primaryR.push_back(mid_r->at(i));
      hPrimaryR.Fill(mid_r->at(i));
      hPrimaryRZoom.Fill(mid_r->at(i));
      hPrimaryRZ.Fill(mid_z->at(i), mid_r->at(i));

      if (i + 1 < n && track_id->at(i + 1) == 1) ++primaryImmediateSame;
      else if (i + 1 < n) ++primaryImmediateDifferent;

      const int js = nextSameIdx[i];
      if (js >= 0) {
        ++primarySameTrackAfter;
        primaryNextProc[process_subtype->at(js)]++;
      } else {
        ++primaryNoSameTrackAfter;
      }

      const float r = mid_r->at(i);
      if (r >= 60.0f && r < 120.0f) {
        ++r100Count;
        r100R.push_back(r);
        r100Z.push_back(mid_z->at(i));
        r100TX0.push_back(step_tX0->at(i));
        r100Loss.push_back(loss->at(i));
        hR100RZ.Fill(mid_z->at(i), r);
        r100Materials[material->at(i)]++;
        r100Volumes[pre_volume->at(i)]++;
        const double retained = pre_p->at(i) > 0.0f ? post_p->at(i) / pre_p->at(i) : 1.0;
        if (1.0 - retained > 0.01) ++r100Hard1;
        if (1.0 - retained > 0.10) ++r100Hard10;
      }
    }
  }

  auto matRank = sortedCounts(r100Materials);
  for (int i = 0; i < 8 && i < (int)matRank.size(); ++i) {
    hR100Material.SetBinContent(i + 1, matRank[i].second);
    hR100Material.GetXaxis()->SetBinLabel(i + 1, matRank[i].first.c_str());
  }

  ofstream out(string(outdir) + "/summary.txt");
  out << fixed << setprecision(6);
  out << "Input files: gsf_material_steps-e--2.0-85-{1..10}.root\n";
  out << "Tree: g4step_tuple\n";
  out << "eBrem selection: process_subtype == 3\n";
  out << "Primary electron selection: track_id == 1 && parent_id == 0 && pdg == 11\n\n";
  out << "events " << events << "\n";
  out << "steps " << steps << "\n";
  out << "all_eBrem_steps " << allEBrem << "\n";
  out << "all_eBrem_same_track_after " << allSameTrackAfter << "\n";
  out << "all_eBrem_no_same_track_after " << allNoSameTrackAfter << "\n";
  out << "primary_eBrem_steps " << primaryEBrem << "\n";
  out << "primary_eBrem_same_track_after " << primarySameTrackAfter << "\n";
  out << "primary_eBrem_no_same_track_after " << primaryNoSameTrackAfter << "\n";
  out << "primary_eBrem_immediate_same_track " << primaryImmediateSame << "\n";
  out << "primary_eBrem_immediate_different_track " << primaryImmediateDifferent << "\n\n";
  out << "primary_eBrem_R_mid_mm mean " << accumulate(primaryR.begin(), primaryR.end(), 0.0) / primaryR.size()
      << " q01 " << quantile(primaryR, 0.01) << " q05 " << quantile(primaryR, 0.05)
      << " q10 " << quantile(primaryR, 0.10) << " q25 " << quantile(primaryR, 0.25)
      << " q50 " << quantile(primaryR, 0.50) << " q75 " << quantile(primaryR, 0.75)
      << " q90 " << quantile(primaryR, 0.90) << " q95 " << quantile(primaryR, 0.95)
      << " q99 " << quantile(primaryR, 0.99) << "\n";
  out << "r100_primary_eBrem_count " << r100Count << "\n";
  out << "r100_midR_q10_q50_q90 " << quantile(r100R, 0.10) << " " << quantile(r100R, 0.50) << " " << quantile(r100R, 0.90) << "\n";
  out << "r100_midZ_q10_q50_q90 " << quantile(r100Z, 0.10) << " " << quantile(r100Z, 0.50) << " " << quantile(r100Z, 0.90) << "\n";
  out << "r100_absZ_q50_q90 ";
  vector<float> r100AbsZ;
  for (float z : r100Z) r100AbsZ.push_back(fabs(z));
  out << quantile(r100AbsZ, 0.50) << " " << quantile(r100AbsZ, 0.90) << "\n";
  out << "r100_step_tX0_mean_q50_q90 " << accumulate(r100TX0.begin(), r100TX0.end(), 0.0) / r100TX0.size()
      << " " << quantile(r100TX0, 0.50) << " " << quantile(r100TX0, 0.90) << "\n";
  out << "r100_loss_GeV_mean_q50_q90 " << accumulate(r100Loss.begin(), r100Loss.end(), 0.0) / r100Loss.size()
      << " " << quantile(r100Loss, 0.50) << " " << quantile(r100Loss, 0.90) << "\n";
  out << "r100_hard_loss_gt_1pct " << r100Hard1 << "\n";
  out << "r100_hard_loss_gt_10pct " << r100Hard10 << "\n\n";
  out << "primary_next_same_track_process_counts\n";
  for (const auto& kv : primaryNextProc) out << procName(kv.first) << " " << kv.second << "\n";
  out << "\nr100_material_counts\n";
  for (const auto& kv : sortedCounts(r100Materials)) out << kv.first << " " << kv.second << "\n";
  out << "\nr100_pre_volume_counts_top40\n";
  int c = 0;
  for (const auto& kv : sortedCounts(r100Volumes)) {
    if (c++ >= 40) break;
    out << kv.first << " " << kv.second << "\n";
  }
  out.close();

  TCanvas c1("c1", "c1", 900, 650);
  c1.SetLogy();
  hAllR.SetLineColor(kGray + 2);
  hAllR.SetLineWidth(2);
  hPrimaryR.SetLineColor(kRed + 1);
  hPrimaryR.SetLineWidth(2);
  hAllR.Draw("hist");
  hPrimaryR.Draw("hist same");
  TLegend leg1(0.58, 0.72, 0.88, 0.86);
  leg1.AddEntry(&hAllR, "all eBrem", "l");
  leg1.AddEntry(&hPrimaryR, "primary e^{-} eBrem", "l");
  leg1.Draw();
  saveCanvas(c1, plotdir, "ebrem_midR_all_vs_primary");

  TCanvas c2("c2", "c2", 900, 650);
  hPrimaryRZoom.SetLineColor(kRed + 1);
  hPrimaryRZoom.SetLineWidth(2);
  hPrimaryRZoom.Draw("hist");
  saveCanvas(c2, plotdir, "primary_ebrem_midR_zoom_inner");

  TCanvas c3("c3", "c3", 900, 650);
  c3.SetLogz();
  hPrimaryRZ.Draw("colz");
  saveCanvas(c3, plotdir, "primary_ebrem_midR_vs_midZ");

  TCanvas c4("c4", "c4", 900, 650);
  hR100RZ.Draw("colz");
  saveCanvas(c4, plotdir, "primary_ebrem_R100_midR_vs_midZ");

  TCanvas c5("c5", "c5", 950, 650);
  c5.SetBottomMargin(0.22);
  hR100Material.SetFillColor(kAzure + 1);
  hR100Material.SetLineColor(kAzure + 2);
  hR100Material.Draw("hist");
  hR100Material.GetXaxis()->LabelsOption("v");
  saveCanvas(c5, plotdir, "primary_ebrem_R100_materials");
}
