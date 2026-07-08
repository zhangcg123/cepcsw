#include "GsfSimHitTuple.h"

#include "GaudiKernel/SmartDataPtr.h"
#include "k4FWCore/DataWrapper.h"
#include "edm4hep/MCParticle.h"
#include "edm4hep/SimTrackerHitCollection.h"

#include <TFile.h>
#include <TTree.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numeric>

DECLARE_COMPONENT(RecGsfSimHitTuple)

namespace {
float mag3(float x, float y, float z) {
  return std::sqrt(x*x + y*y + z*z);
}

double mag3d(double x, double y, double z) {
  return std::sqrt(x*x + y*y + z*z);
}
}

RecGsfSimHitTuple::RecGsfSimHitTuple(const std::string& name, ISvcLocator* svc)
  : Algorithm(name, svc) {}

StatusCode RecGsfSimHitTuple::initialize() {
  m_iev = 0;
  m_file = TFile::Open(m_outFileName.value().c_str(), "RECREATE");
  if (!m_file || m_file->IsZombie()) {
    error() << "Cannot create output file " << m_outFileName << endmsg;
    return StatusCode::FAILURE;
  }

  m_tree = new TTree("simhit_tuple", "GSF SimTrackerHit Truth Tuple");
  m_tree->SetDirectory(m_file);

  m_tree->Branch("iev", &m_iev);
  m_tree->Branch("mc_pdg", &m_mc_pdg);
  m_tree->Branch("mc_gen_status", &m_mc_gen_status);
  m_tree->Branch("mc_sim_status", &m_mc_sim_status);
  m_tree->Branch("mc_vx", &m_mc_vx);
  m_tree->Branch("mc_vy", &m_mc_vy);
  m_tree->Branch("mc_vz", &m_mc_vz);
  m_tree->Branch("mc_ex", &m_mc_ex);
  m_tree->Branch("mc_ey", &m_mc_ey);
  m_tree->Branch("mc_ez", &m_mc_ez);
  m_tree->Branch("mc_px", &m_mc_px);
  m_tree->Branch("mc_py", &m_mc_py);
  m_tree->Branch("mc_pz", &m_mc_pz);
  m_tree->Branch("mc_p", &m_mc_p);
  m_tree->Branch("mc_pT", &m_mc_pT);
  m_tree->Branch("mc_end_px", &m_mc_end_px);
  m_tree->Branch("mc_end_py", &m_mc_end_py);
  m_tree->Branch("mc_end_pz", &m_mc_end_pz);
  m_tree->Branch("mc_end_p", &m_mc_end_p);
  m_tree->Branch("mc_end_pT", &m_mc_end_pT);
  m_tree->Branch("mc_retained_p", &m_mc_retained_p);

  m_tree->Branch("hit_n", &m_hit_n);
  m_tree->Branch("hit_det", &m_hit_det);
  m_tree->Branch("hit_col_index", &m_hit_col_index);
  m_tree->Branch("hit_mc_index", &m_hit_mc_index);
  m_tree->Branch("hit_mc_col", &m_hit_mc_col);
  m_tree->Branch("hit_pdg", &m_hit_pdg);
  m_tree->Branch("hit_quality", &m_hit_quality);
  m_tree->Branch("hit_cellid", &m_hit_cellid);
  m_tree->Branch("hit_x", &m_hit_x);
  m_tree->Branch("hit_y", &m_hit_y);
  m_tree->Branch("hit_z", &m_hit_z);
  m_tree->Branch("hit_r", &m_hit_r);
  m_tree->Branch("hit_px", &m_hit_px);
  m_tree->Branch("hit_py", &m_hit_py);
  m_tree->Branch("hit_pz", &m_hit_pz);
  m_tree->Branch("hit_p", &m_hit_p);
  m_tree->Branch("hit_pT", &m_hit_pT);
  m_tree->Branch("hit_time", &m_hit_time);
  m_tree->Branch("hit_edep", &m_hit_edep);
  m_tree->Branch("hit_path_length", &m_hit_path_length);
  m_tree->Branch("hit_retained_vs_primary", &m_hit_retained_vs_primary);
  m_tree->Branch("hit_loss_vs_primary", &m_hit_loss_vs_primary);
  m_tree->Branch("hit_has_prev", &m_hit_has_prev);
  m_tree->Branch("hit_prev_index_time", &m_hit_prev_index_time);
  m_tree->Branch("hit_order_time", &m_hit_order_time);
  m_tree->Branch("hit_prev_p", &m_hit_prev_p);
  m_tree->Branch("hit_prev_pT", &m_hit_prev_pT);
  m_tree->Branch("hit_prev_x", &m_hit_prev_x);
  m_tree->Branch("hit_prev_y", &m_hit_prev_y);
  m_tree->Branch("hit_prev_z", &m_hit_prev_z);
  m_tree->Branch("hit_prev_r", &m_hit_prev_r);
  m_tree->Branch("hit_prev_time", &m_hit_prev_time);
  m_tree->Branch("hit_step_retained_vs_prev", &m_hit_step_retained_vs_prev);
  m_tree->Branch("hit_step_loss_vs_prev", &m_hit_step_loss_vs_prev);
  m_tree->Branch("hit_step_dp", &m_hit_step_dp);
  m_tree->Branch("hit_step_dr", &m_hit_step_dr);
  m_tree->Branch("hit_step_ds", &m_hit_step_ds);
  m_tree->Branch("hit_step_dt", &m_hit_step_dt);

  info() << "Output: " << m_outFileName << endmsg;
  info() << "SimTrackerHit collections to dump: "
         << m_simHitCollectionNames.value().size() << endmsg;
  return StatusCode::SUCCESS;
}

void RecGsfSimHitTuple::clearVectors() {
  m_hit_n = 0;
  m_hit_det.clear();
  m_hit_col_index.clear();
  m_hit_mc_index.clear();
  m_hit_mc_col.clear();
  m_hit_pdg.clear();
  m_hit_quality.clear();
  m_hit_cellid.clear();
  m_hit_x.clear(); m_hit_y.clear(); m_hit_z.clear(); m_hit_r.clear();
  m_hit_px.clear(); m_hit_py.clear(); m_hit_pz.clear();
  m_hit_p.clear(); m_hit_pT.clear();
  m_hit_time.clear();
  m_hit_edep.clear();
  m_hit_path_length.clear();
  m_hit_retained_vs_primary.clear();
  m_hit_loss_vs_primary.clear();
  m_hit_has_prev.clear();
  m_hit_prev_index_time.clear();
  m_hit_order_time.clear();
  m_hit_prev_p.clear(); m_hit_prev_pT.clear();
  m_hit_prev_x.clear(); m_hit_prev_y.clear(); m_hit_prev_z.clear(); m_hit_prev_r.clear();
  m_hit_prev_time.clear();
  m_hit_step_retained_vs_prev.clear();
  m_hit_step_loss_vs_prev.clear();
  m_hit_step_dp.clear();
  m_hit_step_dr.clear();
  m_hit_step_ds.clear();
  m_hit_step_dt.clear();
}

void RecGsfSimHitTuple::fillHitToHitDiagnostics() {
  const int n = (int)m_hit_p.size();
  m_hit_has_prev.assign(n, 0);
  m_hit_prev_index_time.assign(n, -1);
  m_hit_order_time.assign(n, -1);
  m_hit_prev_p.assign(n, 0.0f);
  m_hit_prev_pT.assign(n, 0.0f);
  m_hit_prev_x.assign(n, 0.0f);
  m_hit_prev_y.assign(n, 0.0f);
  m_hit_prev_z.assign(n, 0.0f);
  m_hit_prev_r.assign(n, 0.0f);
  m_hit_prev_time.assign(n, 0.0f);
  m_hit_step_retained_vs_prev.assign(n, 1.0f);
  m_hit_step_loss_vs_prev.assign(n, 0.0f);
  m_hit_step_dp.assign(n, 0.0f);
  m_hit_step_dr.assign(n, 0.0f);
  m_hit_step_ds.assign(n, 0.0f);
  m_hit_step_dt.assign(n, 0.0f);

  std::vector<int> order(n);
  std::iota(order.begin(), order.end(), 0);
  std::stable_sort(order.begin(), order.end(), [this](int a, int b) {
    if (m_hit_time[a] != m_hit_time[b]) return m_hit_time[a] < m_hit_time[b];
    if (m_hit_r[a] != m_hit_r[b]) return m_hit_r[a] < m_hit_r[b];
    if (m_hit_det[a] != m_hit_det[b]) return m_hit_det[a] < m_hit_det[b];
    return m_hit_col_index[a] < m_hit_col_index[b];
  });

  for (int iord = 0; iord < n; ++iord) {
    const int idx = order[iord];
    m_hit_order_time[idx] = iord;
    if (iord == 0) continue;

    const int prev = order[iord - 1];
    const float dx = m_hit_x[idx] - m_hit_x[prev];
    const float dy = m_hit_y[idx] - m_hit_y[prev];
    const float dz = m_hit_z[idx] - m_hit_z[prev];
    const float retained = (m_hit_p[prev] > 0.0f) ? m_hit_p[idx] / m_hit_p[prev] : 0.0f;

    m_hit_has_prev[idx] = 1;
    m_hit_prev_index_time[idx] = prev;
    m_hit_prev_p[idx] = m_hit_p[prev];
    m_hit_prev_pT[idx] = m_hit_pT[prev];
    m_hit_prev_x[idx] = m_hit_x[prev];
    m_hit_prev_y[idx] = m_hit_y[prev];
    m_hit_prev_z[idx] = m_hit_z[prev];
    m_hit_prev_r[idx] = m_hit_r[prev];
    m_hit_prev_time[idx] = m_hit_time[prev];
    m_hit_step_retained_vs_prev[idx] = retained;
    m_hit_step_loss_vs_prev[idx] = 1.0f - retained;
    m_hit_step_dp[idx] = m_hit_p[idx] - m_hit_p[prev];
    m_hit_step_dr[idx] = m_hit_r[idx] - m_hit_r[prev];
    m_hit_step_ds[idx] = std::sqrt(dx*dx + dy*dy + dz*dz);
    m_hit_step_dt[idx] = m_hit_time[idx] - m_hit_time[prev];
  }
}

StatusCode RecGsfSimHitTuple::execute() {
  m_iev++;
  clearVectors();

  const auto* mcCol = m_inMCParticles.get();
  edm4hep::MCParticle primary;
  bool hasPrimary = false;

  m_mc_pdg = 0; m_mc_gen_status = 0; m_mc_sim_status = 0;
  m_mc_vx = m_mc_vy = m_mc_vz = 0;
  m_mc_ex = m_mc_ey = m_mc_ez = 0;
  m_mc_px = m_mc_py = m_mc_pz = m_mc_p = m_mc_pT = 0;
  m_mc_end_px = m_mc_end_py = m_mc_end_pz = 0;
  m_mc_end_p = m_mc_end_pT = m_mc_retained_p = 0;

  if (mcCol && mcCol->size() > 0) {
    primary = (*mcCol)[0];
    hasPrimary = primary.isAvailable();
    const auto& v = primary.getVertex();
    const auto& e = primary.getEndpoint();
    const auto& p = primary.getMomentum();
    const auto& pe = primary.getMomentumAtEndpoint();
    m_mc_pdg = primary.getPDG();
    m_mc_gen_status = primary.getGeneratorStatus();
    m_mc_sim_status = primary.getSimulatorStatus();
    m_mc_vx = v.x; m_mc_vy = v.y; m_mc_vz = v.z;
    m_mc_ex = e.x; m_mc_ey = e.y; m_mc_ez = e.z;
    m_mc_px = p.x; m_mc_py = p.y; m_mc_pz = p.z;
    m_mc_p = mag3d(m_mc_px, m_mc_py, m_mc_pz);
    m_mc_pT = std::hypot(m_mc_px, m_mc_py);
    m_mc_end_px = pe.x; m_mc_end_py = pe.y; m_mc_end_pz = pe.z;
    m_mc_end_p = mag3d(m_mc_end_px, m_mc_end_py, m_mc_end_pz);
    m_mc_end_pT = std::hypot(m_mc_end_px, m_mc_end_py);
    m_mc_retained_p = (m_mc_p > 0) ? m_mc_end_p / m_mc_p : 0;
  }

  int detIdx = 0;
  for (const auto& colName : m_simHitCollectionNames.value()) {
    SmartDataPtr<DataWrapper<edm4hep::SimTrackerHitCollection>> dw(eventSvc(), colName);
    const auto* hits = dw ? dw->getData() : nullptr;
    if (!hits) {
      warning() << "Missing SimTrackerHit collection: " << colName << endmsg;
      detIdx++;
      continue;
    }

    int localIdx = 0;
    for (const auto& hit : *hits) {
      if (!hit.isAvailable()) { localIdx++; continue; }
      auto mcp = hit.getMCParticle();
      if (m_primaryOnly && hasPrimary && !(mcp == primary)) { localIdx++; continue; }
      if (m_electronOnly && mcp.isAvailable() && std::abs(mcp.getPDG()) != 11) {
        localIdx++;
        continue;
      }

      const auto& pos = hit.getPosition();
      const auto& mom = hit.getMomentum();
      const float p = mag3(mom.x, mom.y, mom.z);
      const float pT = std::hypot(mom.x, mom.y);
      int mcIndex = -1;
      int mcCollection = -1;
      if (mcp.isAvailable()) {
        const auto oid = mcp.getObjectID();
        mcIndex = oid.index;
        mcCollection = oid.collectionID;
      }

      m_hit_det.push_back(detIdx);
      m_hit_col_index.push_back(localIdx);
      m_hit_mc_index.push_back(mcIndex);
      m_hit_mc_col.push_back(mcCollection);
      m_hit_pdg.push_back(mcp.isAvailable() ? mcp.getPDG() : 0);
      m_hit_quality.push_back(hit.getQuality());
      m_hit_cellid.push_back(hit.getCellID());
      m_hit_x.push_back((float)pos.x);
      m_hit_y.push_back((float)pos.y);
      m_hit_z.push_back((float)pos.z);
      m_hit_r.push_back((float)std::hypot(pos.x, pos.y));
      m_hit_px.push_back(mom.x);
      m_hit_py.push_back(mom.y);
      m_hit_pz.push_back(mom.z);
      m_hit_p.push_back(p);
      m_hit_pT.push_back(pT);
      m_hit_time.push_back(hit.getTime());
      m_hit_edep.push_back(hit.getEDep());
      m_hit_path_length.push_back(hit.getPathLength());
      const float retained = (m_mc_p > 0) ? (float)(p / m_mc_p) : 0.0f;
      m_hit_retained_vs_primary.push_back(retained);
      m_hit_loss_vs_primary.push_back(1.0f - retained);
      localIdx++;
    }
    detIdx++;
  }

  m_hit_n = (int)m_hit_p.size();
  fillHitToHitDiagnostics();
  m_tree->Fill();
  return StatusCode::SUCCESS;
}

StatusCode RecGsfSimHitTuple::finalize() {
  if (m_file) {
    m_file->cd();
    if (m_tree) m_tree->Write();
    m_file->Close();
    delete m_file;
    m_file = nullptr;
  }
  info() << "Wrote " << m_outFileName << " with " << m_iev << " events" << endmsg;
  return StatusCode::SUCCESS;
}
