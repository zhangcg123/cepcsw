#include "GsfFlatTuple.h"

#include "GaudiKernel/SmartDataPtr.h"
#include "k4FWCore/DataWrapper.h"

#include <TFile.h>
#include <TTree.h>
#include <algorithm>
#include <cmath>
#include <vector>

DECLARE_COMPONENT(RecGsfFlatTuple)

using DH = edm4hep::TrackState;

// ── helpers ───────────────────────────────────────────────────────────

/// Convert helical parameters to pT, eta, theta at the reference point.
/// omega, tanLambda follow the LCIO / edm4hep convention.
/// alpha = B·c·10⁻⁴ [GeV/(mm·T)]
static void helixToKinematics(double omega, double tanLambda,
                              double alpha,
                              double& pT, double& p, double& eta, double& theta) {
  pT = (std::abs(omega) > 1e-15) ? std::abs(alpha / omega) : 0.0;
  theta = std::atan2(1.0, tanLambda);  // tanLambda = cot(theta)
  double sinTheta = std::sin(theta);
  if (sinTheta < 1e-15) sinTheta = 1e-15;
  p = pT / sinTheta;
  eta = std::asinh(tanLambda);
}

// ── algorithm ─────────────────────────────────────────────────────────

RecGsfFlatTuple::RecGsfFlatTuple(const std::string& name, ISvcLocator* svc)
  : Algorithm(name, svc) {}

StatusCode RecGsfFlatTuple::initialize() {
  m_iev = 0;

  // ── open output ROOT file ──
  m_file = TFile::Open(m_outFileName.value().c_str(), "RECREATE");
  if (!m_file || m_file->IsZombie()) {
    error() << "Cannot create output file " << m_outFileName << endmsg;
    return StatusCode::FAILURE;
  }

  // ── create tree ──
  m_tree = new TTree("gsf_tuple", "GSF Tracking Flat Tuple");
  m_tree->SetDirectory(m_file);

  m_tree->Branch("iev",        &m_iev);
  // MC truth
  m_tree->Branch("mc_pdg",     &m_mc_pdg);
  m_tree->Branch("mc_px",      &m_mc_px);
  m_tree->Branch("mc_py",      &m_mc_py);
  m_tree->Branch("mc_pz",      &m_mc_pz);
  m_tree->Branch("mc_pT",      &m_mc_pT);
  m_tree->Branch("mc_p",       &m_mc_p);
  m_tree->Branch("mc_eta",     &m_mc_eta);
  m_tree->Branch("mc_theta",   &m_mc_theta);
  m_tree->Branch("mc_phi",     &m_mc_phi);
  m_tree->Branch("mc_vx",      &m_mc_vx);
  m_tree->Branch("mc_vy",      &m_mc_vy);
  m_tree->Branch("mc_vz",      &m_mc_vz);
  // LCIO per-hit
  m_tree->Branch("lcio_hit_n",     &m_lcio_hit_n);
  m_tree->Branch("lcio_hit_x",     &m_lcio_hit_x);
  m_tree->Branch("lcio_hit_y",     &m_lcio_hit_y);
  m_tree->Branch("lcio_hit_z",     &m_lcio_hit_z);
  m_tree->Branch("lcio_hit_r",     &m_lcio_hit_r);
  m_tree->Branch("lcio_hit_edep",  &m_lcio_hit_edep);
  m_tree->Branch("lcio_hit_cellid",&m_lcio_hit_cellid);
  // GSF per-hit
  m_tree->Branch("gsf_hit_n",     &m_gsf_hit_n);
  m_tree->Branch("gsf_hit_x",     &m_gsf_hit_x);
  m_tree->Branch("gsf_hit_y",     &m_gsf_hit_y);
  m_tree->Branch("gsf_hit_z",     &m_gsf_hit_z);
  m_tree->Branch("gsf_hit_r",     &m_gsf_hit_r);
  m_tree->Branch("gsf_hit_edep",  &m_gsf_hit_edep);
  m_tree->Branch("gsf_hit_cellid",&m_gsf_hit_cellid);
  // all hits from original collections
  m_tree->Branch("all_hit_n",     &m_all_hit_n);
  m_tree->Branch("all_hit_x",     &m_all_hit_x);
  m_tree->Branch("all_hit_y",     &m_all_hit_y);
  m_tree->Branch("all_hit_z",     &m_all_hit_z);
  m_tree->Branch("all_hit_r",     &m_all_hit_r);
  m_tree->Branch("all_hit_edep",  &m_all_hit_edep);
  m_tree->Branch("all_hit_cellid",&m_all_hit_cellid);
  m_tree->Branch("all_hit_det",   &m_all_hit_det);
  // LCIO CompleteTracks
  m_tree->Branch("lcio_pT",    &m_lcio_pT);
  m_tree->Branch("lcio_p",     &m_lcio_p);
  m_tree->Branch("lcio_eta",   &m_lcio_eta);
  m_tree->Branch("lcio_theta", &m_lcio_theta);
  m_tree->Branch("lcio_phi",   &m_lcio_phi);
  m_tree->Branch("lcio_d0",    &m_lcio_d0);
  m_tree->Branch("lcio_z0",    &m_lcio_z0);
  m_tree->Branch("lcio_omega", &m_lcio_omega);
  m_tree->Branch("lcio_tanl",  &m_lcio_tanl);
  m_tree->Branch("lcio_chi2",  &m_lcio_chi2);
  m_tree->Branch("lcio_ndf",   &m_lcio_ndf);
  m_tree->Branch("lcio_nhits", &m_lcio_nhits);
  m_tree->Branch("lcio_type",  &m_lcio_type);
  // GSF
  m_tree->Branch("gsf_pT",    &m_gsf_pT);
  m_tree->Branch("gsf_p",     &m_gsf_p);
  m_tree->Branch("gsf_eta",   &m_gsf_eta);
  m_tree->Branch("gsf_theta", &m_gsf_theta);
  m_tree->Branch("gsf_phi",   &m_gsf_phi);
  m_tree->Branch("gsf_d0",    &m_gsf_d0);
  m_tree->Branch("gsf_z0",    &m_gsf_z0);
  m_tree->Branch("gsf_omega", &m_gsf_omega);
  m_tree->Branch("gsf_tanl",  &m_gsf_tanl);
  m_tree->Branch("gsf_chi2",  &m_gsf_chi2);
  m_tree->Branch("gsf_ndf",   &m_gsf_ndf);
  m_tree->Branch("gsf_nhits", &m_gsf_nhits);
  m_tree->Branch("gsf_type",  &m_gsf_type);
  // paired smoother/reverse WeightedMean GSF
  m_tree->Branch("weighted_gsf_available", &m_weighted_gsf_available);
  m_tree->Branch("weighted_gsf_changed",   &m_weighted_gsf_changed);
  m_tree->Branch("weighted_gsf_pT",        &m_weighted_gsf_pT);
  m_tree->Branch("weighted_gsf_p",         &m_weighted_gsf_p);
  m_tree->Branch("weighted_gsf_eta",       &m_weighted_gsf_eta);
  m_tree->Branch("weighted_gsf_theta",     &m_weighted_gsf_theta);
  m_tree->Branch("weighted_gsf_phi",       &m_weighted_gsf_phi);
  m_tree->Branch("weighted_gsf_d0",        &m_weighted_gsf_d0);
  m_tree->Branch("weighted_gsf_z0",        &m_weighted_gsf_z0);
  m_tree->Branch("weighted_gsf_omega",     &m_weighted_gsf_omega);
  m_tree->Branch("weighted_gsf_tanl",      &m_weighted_gsf_tanl);
  m_tree->Branch("weighted_gsf_chi2",      &m_weighted_gsf_chi2);
  m_tree->Branch("weighted_gsf_ndf",       &m_weighted_gsf_ndf);
  m_tree->Branch("weighted_gsf_nhits",     &m_weighted_gsf_nhits);
  m_tree->Branch("weighted_gsf_type",      &m_weighted_gsf_type);
  // truth BH-loss oracle validity for CompleteTracks index 0
  m_tree->Branch("truth_bh_scope_status", &m_truth_bh_scope_status);
  m_tree->Branch("truth_bh_scope_valid",  &m_truth_bh_scope_valid);
  // passive truth/DD4hep/runtime material intervals
  m_tree->Branch("truth_material_scope_status",
                 &m_truth_material_scope_status);
  m_tree->Branch("truth_material_scope_valid",
                 &m_truth_material_scope_valid);
  m_tree->Branch("truth_material_interval_n",
                 &m_truth_material_interval_n);
  m_tree->Branch("truth_material_input_track_index",
                 &m_truth_material_input_track_index);
  m_tree->Branch("truth_material_output_track_index",
                 &m_truth_material_output_track_index);
  m_tree->Branch("truth_material_hit_from_index",
                 &m_truth_material_hit_from_index);
  m_tree->Branch("truth_material_hit_to_index",
                 &m_truth_material_hit_to_index);
  m_tree->Branch("truth_material_surface_from_index",
                 &m_truth_material_surface_from_index);
  m_tree->Branch("truth_material_surface_to_index",
                 &m_truth_material_surface_to_index);
  m_tree->Branch("truth_material_cell_from", &m_truth_material_cell_from);
  m_tree->Branch("truth_material_cell_to", &m_truth_material_cell_to);
  m_tree->Branch("truth_material_track_id", &m_truth_material_track_id);
  m_tree->Branch("truth_material_first_step", &m_truth_material_first_step);
  m_tree->Branch("truth_material_last_step", &m_truth_material_last_step);
  m_tree->Branch("truth_material_start_hook_fraction",
                 &m_truth_material_start_hook_fraction);
  m_tree->Branch("truth_material_end_hook_fraction",
                 &m_truth_material_end_hook_fraction);
  m_tree->Branch("truth_material_start_x", &m_truth_material_start_x);
  m_tree->Branch("truth_material_start_y", &m_truth_material_start_y);
  m_tree->Branch("truth_material_start_z", &m_truth_material_start_z);
  m_tree->Branch("truth_material_end_x", &m_truth_material_end_x);
  m_tree->Branch("truth_material_end_y", &m_truth_material_end_y);
  m_tree->Branch("truth_material_end_z", &m_truth_material_end_z);
  m_tree->Branch("truth_material_step_count",
                 &m_truth_material_step_count);
  m_tree->Branch("truth_material_g4_tx0", &m_truth_material_g4_tx0);
  m_tree->Branch("truth_material_p_before", &m_truth_material_p_before);
  m_tree->Branch("truth_material_ebrem_loss", &m_truth_material_ebrem_loss);
  m_tree->Branch("truth_material_retained_fraction",
                 &m_truth_material_retained_fraction);
  m_tree->Branch("truth_material_dd4hep_hook_valid",
                 &m_truth_material_dd4hep_hook_valid);
  m_tree->Branch("truth_material_dd4hep_hook_layer_count",
                 &m_truth_material_dd4hep_hook_layer_count);
  m_tree->Branch("truth_material_dd4hep_hook_tx0",
                 &m_truth_material_dd4hep_hook_tx0);
  m_tree->Branch("truth_material_runtime_mode",
                 &m_truth_material_runtime_mode);
  m_tree->Branch("truth_material_split_threshold",
                 &m_truth_material_split_threshold);
  m_tree->Branch("truth_material_forward_candidate_count",
                 &m_truth_material_forward_candidate_count);
  m_tree->Branch("truth_material_forward_valid_count",
                 &m_truth_material_forward_valid_count);
  m_tree->Branch("truth_material_forward_above_threshold_count",
                 &m_truth_material_forward_above_threshold_count);
  m_tree->Branch("truth_material_forward_weighted_tx0",
                 &m_truth_material_forward_weighted_tx0);
  m_tree->Branch("truth_material_forward_min_tx0",
                 &m_truth_material_forward_min_tx0);
  m_tree->Branch("truth_material_forward_max_tx0",
                 &m_truth_material_forward_max_tx0);
  m_tree->Branch("truth_material_forward_leading_component_id",
                 &m_truth_material_forward_leading_component_id);
  m_tree->Branch("truth_material_forward_leading_component_weight",
                 &m_truth_material_forward_leading_component_weight);
  m_tree->Branch("truth_material_forward_leading_tx0",
                 &m_truth_material_forward_leading_tx0);
  m_tree->Branch("truth_material_reverse_candidate_count",
                 &m_truth_material_reverse_candidate_count);
  m_tree->Branch("truth_material_reverse_valid_count",
                 &m_truth_material_reverse_valid_count);
  m_tree->Branch("truth_material_reverse_above_threshold_count",
                 &m_truth_material_reverse_above_threshold_count);
  m_tree->Branch("truth_material_reverse_weighted_tx0",
                 &m_truth_material_reverse_weighted_tx0);
  m_tree->Branch("truth_material_reverse_min_tx0",
                 &m_truth_material_reverse_min_tx0);
  m_tree->Branch("truth_material_reverse_max_tx0",
                 &m_truth_material_reverse_max_tx0);
  m_tree->Branch("truth_material_reverse_leading_component_id",
                 &m_truth_material_reverse_leading_component_id);
  m_tree->Branch("truth_material_reverse_leading_component_weight",
                 &m_truth_material_reverse_leading_component_weight);
  m_tree->Branch("truth_material_reverse_leading_tx0",
                 &m_truth_material_reverse_leading_tx0);
  // paired ECAL-constrained GSF
  m_tree->Branch("ecal_gsf_available", &m_ecal_gsf_available);
  m_tree->Branch("ecal_gsf_changed",   &m_ecal_gsf_changed);
  m_tree->Branch("ecal_gsf_pT",        &m_ecal_gsf_pT);
  m_tree->Branch("ecal_gsf_p",         &m_ecal_gsf_p);
  m_tree->Branch("ecal_gsf_eta",       &m_ecal_gsf_eta);
  m_tree->Branch("ecal_gsf_theta",     &m_ecal_gsf_theta);
  m_tree->Branch("ecal_gsf_phi",       &m_ecal_gsf_phi);
  m_tree->Branch("ecal_gsf_d0",        &m_ecal_gsf_d0);
  m_tree->Branch("ecal_gsf_z0",        &m_ecal_gsf_z0);
  m_tree->Branch("ecal_gsf_omega",     &m_ecal_gsf_omega);
  m_tree->Branch("ecal_gsf_tanl",      &m_ecal_gsf_tanl);
  m_tree->Branch("ecal_gsf_chi2",      &m_ecal_gsf_chi2);
  m_tree->Branch("ecal_gsf_ndf",       &m_ecal_gsf_ndf);
  m_tree->Branch("ecal_gsf_nhits",     &m_ecal_gsf_nhits);
  m_tree->Branch("ecal_gsf_type",      &m_ecal_gsf_type);
  // resolution
  m_tree->Branch("res_pT_gsf",      &m_res_pT_gsf);
  m_tree->Branch("res_pT_weighted_gsf", &m_res_pT_weighted_gsf);
  m_tree->Branch("res_pT_ecal_gsf", &m_res_pT_ecal_gsf);
  m_tree->Branch("res_pT_lcio",     &m_res_pT_lcio);

  info() << "Output: " << m_outFileName
         << " trackSource="
         << (m_useGlobalLossTracks.value() ? "GlobalLossTracks"
                                           : "GSFTracks")
         << endmsg;
  if (!m_hitCollectionNames.value().empty())
    info() << "Hit collections to dump: " << m_hitCollectionNames.value().size()
           << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode RecGsfFlatTuple::execute() {
  m_iev++;

  double alpha = m_bField * 2.99792458e-4;  // GeV/(mm·T)

  const auto* mcCol = m_inMCParticles.get();
  const auto* lcioCol = m_inCompleteTracks.get();
  const auto* gsfCol = m_useGlobalLossTracks.value()
                           ? m_inGlobalLossTracks.get()
                           : m_inGsfTracks.get();
  m_truth_bh_scope_status =
      truthBHLossStatusValue(TruthBHLossScopeStatus::Disabled);
  m_truth_bh_scope_valid = 0;
  try {
    const auto* truthStatus = m_inTruthBHLossStatus.get();
    if (truthStatus && !truthStatus->empty()) {
      m_truth_bh_scope_status = (*truthStatus)[0];
      m_truth_bh_scope_valid =
          m_truth_bh_scope_status ==
                  truthBHLossStatusValue(TruthBHLossScopeStatus::Valid)
              ? 1
              : 0;
    }
  } catch (...) {
    // Older GSF producers do not provide this optional diagnostic collection.
    // Keep the disabled/invalid defaults without changing their tuple flow.
  }

  m_truth_material_scope_status =
      truthBHLossStatusValue(TruthBHLossScopeStatus::Disabled);
  m_truth_material_scope_valid = 0;
  m_truth_material_interval_n = 0;
  m_truth_material_input_track_index.clear();
  m_truth_material_output_track_index.clear();
  m_truth_material_hit_from_index.clear();
  m_truth_material_hit_to_index.clear();
  m_truth_material_surface_from_index.clear();
  m_truth_material_surface_to_index.clear();
  m_truth_material_cell_from.clear();
  m_truth_material_cell_to.clear();
  m_truth_material_track_id.clear();
  m_truth_material_first_step.clear();
  m_truth_material_last_step.clear();
  m_truth_material_start_hook_fraction.clear();
  m_truth_material_end_hook_fraction.clear();
  m_truth_material_start_x.clear();
  m_truth_material_start_y.clear();
  m_truth_material_start_z.clear();
  m_truth_material_end_x.clear();
  m_truth_material_end_y.clear();
  m_truth_material_end_z.clear();
  m_truth_material_step_count.clear();
  m_truth_material_g4_tx0.clear();
  m_truth_material_p_before.clear();
  m_truth_material_ebrem_loss.clear();
  m_truth_material_retained_fraction.clear();
  m_truth_material_dd4hep_hook_valid.clear();
  m_truth_material_dd4hep_hook_layer_count.clear();
  m_truth_material_dd4hep_hook_tx0.clear();
  m_truth_material_runtime_mode.clear();
  m_truth_material_split_threshold.clear();
  m_truth_material_forward_candidate_count.clear();
  m_truth_material_forward_valid_count.clear();
  m_truth_material_forward_above_threshold_count.clear();
  m_truth_material_forward_weighted_tx0.clear();
  m_truth_material_forward_min_tx0.clear();
  m_truth_material_forward_max_tx0.clear();
  m_truth_material_forward_leading_component_id.clear();
  m_truth_material_forward_leading_component_weight.clear();
  m_truth_material_forward_leading_tx0.clear();
  m_truth_material_reverse_candidate_count.clear();
  m_truth_material_reverse_valid_count.clear();
  m_truth_material_reverse_above_threshold_count.clear();
  m_truth_material_reverse_weighted_tx0.clear();
  m_truth_material_reverse_min_tx0.clear();
  m_truth_material_reverse_max_tx0.clear();
  m_truth_material_reverse_leading_component_id.clear();
  m_truth_material_reverse_leading_component_weight.clear();
  m_truth_material_reverse_leading_tx0.clear();

  try {
    const auto* materialStatus = m_inTruthMaterialStatus.get();
    if (materialStatus && !materialStatus->empty()) {
      const auto notSelected =
          truthBHLossStatusValue(TruthBHLossScopeStatus::NotSelected);
      const auto selectedStatus = std::find_if(
          materialStatus->begin(), materialStatus->end(),
          [notSelected](std::int32_t status) {
            return status != notSelected;
          });
      if (selectedStatus != materialStatus->end())
        m_truth_material_scope_status = *selectedStatus;
      m_truth_material_scope_valid =
          m_truth_material_scope_status ==
                  truthBHLossStatusValue(TruthBHLossScopeStatus::Valid)
              ? 1
              : 0;
    }
    const auto* materialIntervals = m_inTruthMaterialIntervals.get();
    if (materialIntervals) {
      for (const auto& interval : *materialIntervals) {
        const auto start = interval.getTruthStartPosition();
        const auto end = interval.getTruthEndPosition();
        m_truth_material_input_track_index.push_back(
            interval.getInputTrackIndex());
        m_truth_material_output_track_index.push_back(
            interval.getOutputTrackIndex());
        m_truth_material_hit_from_index.push_back(
            interval.getHitFromIndex());
        m_truth_material_hit_to_index.push_back(interval.getHitToIndex());
        m_truth_material_surface_from_index.push_back(
            interval.getSurfaceFromIndex());
        m_truth_material_surface_to_index.push_back(
            interval.getSurfaceToIndex());
        m_truth_material_cell_from.push_back(interval.getCellFrom());
        m_truth_material_cell_to.push_back(interval.getCellTo());
        m_truth_material_track_id.push_back(interval.getTruthTrackID());
        m_truth_material_first_step.push_back(
            interval.getTruthFirstStepNumber());
        m_truth_material_last_step.push_back(
            interval.getTruthLastStepNumber());
        m_truth_material_start_hook_fraction.push_back(
            interval.getTruthStartHookFraction());
        m_truth_material_end_hook_fraction.push_back(
            interval.getTruthEndHookFraction());
        m_truth_material_start_x.push_back(start.x);
        m_truth_material_start_y.push_back(start.y);
        m_truth_material_start_z.push_back(start.z);
        m_truth_material_end_x.push_back(end.x);
        m_truth_material_end_y.push_back(end.y);
        m_truth_material_end_z.push_back(end.z);
        m_truth_material_step_count.push_back(interval.getTruthStepCount());
        m_truth_material_g4_tx0.push_back(interval.getTruthG4TX0());
        m_truth_material_p_before.push_back(
            interval.getTruthMomentumBefore());
        m_truth_material_ebrem_loss.push_back(interval.getTruthEbremLoss());
        m_truth_material_retained_fraction.push_back(
            interval.getTruthRetainedMomentumFraction());
        m_truth_material_dd4hep_hook_valid.push_back(
            interval.getDd4hepTruthHookValid());
        m_truth_material_dd4hep_hook_layer_count.push_back(
            interval.getDd4hepTruthHookLayerCount());
        m_truth_material_dd4hep_hook_tx0.push_back(
            interval.getDd4hepTruthHookTX0());
        m_truth_material_runtime_mode.push_back(
            interval.getRuntimeMaterialMode());
        m_truth_material_split_threshold.push_back(
            interval.getSplitThreshold());
        m_truth_material_forward_candidate_count.push_back(
            interval.getForwardCandidateCount());
        m_truth_material_forward_valid_count.push_back(
            interval.getForwardValidCount());
        m_truth_material_forward_above_threshold_count.push_back(
            interval.getForwardAboveThresholdCount());
        m_truth_material_forward_weighted_tx0.push_back(
            interval.getForwardWeightedTX0());
        m_truth_material_forward_min_tx0.push_back(
            interval.getForwardMinTX0());
        m_truth_material_forward_max_tx0.push_back(
            interval.getForwardMaxTX0());
        m_truth_material_forward_leading_component_id.push_back(
            interval.getForwardLeadingComponentID());
        m_truth_material_forward_leading_component_weight.push_back(
            interval.getForwardLeadingComponentWeight());
        m_truth_material_forward_leading_tx0.push_back(
            interval.getForwardLeadingTX0());
        m_truth_material_reverse_candidate_count.push_back(
            interval.getReverseCandidateCount());
        m_truth_material_reverse_valid_count.push_back(
            interval.getReverseValidCount());
        m_truth_material_reverse_above_threshold_count.push_back(
            interval.getReverseAboveThresholdCount());
        m_truth_material_reverse_weighted_tx0.push_back(
            interval.getReverseWeightedTX0());
        m_truth_material_reverse_min_tx0.push_back(
            interval.getReverseMinTX0());
        m_truth_material_reverse_max_tx0.push_back(
            interval.getReverseMaxTX0());
        m_truth_material_reverse_leading_component_id.push_back(
            interval.getReverseLeadingComponentID());
        m_truth_material_reverse_leading_component_weight.push_back(
            interval.getReverseLeadingComponentWeight());
        m_truth_material_reverse_leading_tx0.push_back(
            interval.getReverseLeadingTX0());
      }
      m_truth_material_interval_n =
          static_cast<int>(m_truth_material_g4_tx0.size());
    }
  } catch (...) {
    // Older GSF producers do not provide the passive material collections.
    // Keep disabled/empty values and preserve their flat-tuple flow.
  }
  SmartDataPtr<DataWrapper<edm4hep::TrackCollection>> ecalGsfWrapper(
      eventSvc(), "GSFTracksEcalConstrained");
  const auto* ecalGsfCol = ecalGsfWrapper
      ? ecalGsfWrapper->getData() : nullptr;
  SmartDataPtr<DataWrapper<edm4hep::TrackCollection>> weightedGsfWrapper(
      eventSvc(), "GSFTracksWeightedMean");
  const auto* weightedGsfCol = !m_useGlobalLossTracks.value() &&
          weightedGsfWrapper
      ? weightedGsfWrapper->getData() : nullptr;

  // ── MC truth (first particle = primary) ──
  if (mcCol && mcCol->size() > 0) {
    auto mcp = (*mcCol)[0];
    auto mom = mcp.getMomentum();
    m_mc_pdg   = mcp.getPDG();
    m_mc_px    = mom.x;
    m_mc_py    = mom.y;
    m_mc_pz    = mom.z;
    m_mc_pT    = std::hypot(mom.x, mom.y);
    m_mc_p     = std::hypot(m_mc_pT, mom.z);
    m_mc_eta   = (m_mc_p > 0 && m_mc_pT < m_mc_p)
                 ? std::atanh(mom.z / m_mc_p) : 0.0;
    m_mc_theta = (m_mc_pT > 1e-15) ? std::atan2(m_mc_pT, mom.z) : 0.0;
    m_mc_phi   = std::atan2(mom.y, mom.x);
    auto& vtx  = mcp.getVertex();
    m_mc_vx    = vtx.x;
    m_mc_vy    = vtx.y;
    m_mc_vz    = vtx.z;
  } else {
    m_mc_pdg = 0; m_mc_px = 0; m_mc_py = 0; m_mc_pz = 0;
    m_mc_pT = 0; m_mc_p = 0; m_mc_eta = 0; m_mc_theta = 0; m_mc_phi = 0;
    m_mc_vx = 0; m_mc_vy = 0; m_mc_vz = 0;
  }

  // ── helper: fill LCIO / GSF branches from a track ──
  auto fillTrack = [&](const edm4hep::TrackCollection* col,
                       double& pT, double& p, double& eta, double& theta,
                       double& phi, double& d0, double& z0,
                       double& omega, double& tanl,
                       double& chi2, int& ndf, int& nhits, int& type) {
    pT = 0; p = 0; eta = 0; theta = 0;
    phi = 0; d0 = 0; z0 = 0; omega = 0; tanl = 0;
    chi2 = 0; ndf = 0; nhits = 0; type = 0;

    if (!col || col->size() == 0) return;

    const auto& trk = (*col)[0];
    type   = trk.getType();
    chi2   = trk.getChi2();
    ndf    = trk.getNdf();
    nhits  = trk.trackerHits_size();

    // Look for AtIP (location=4) track state
    for (const auto& ts : trk.getTrackStates()) {
      if (ts.location == DH::AtIP) {
        omega = ts.omega;
        d0    = ts.D0;
        z0    = ts.Z0;
        phi   = ts.phi;
        tanl  = ts.tanLambda;
        helixToKinematics(omega, tanl, alpha, pT, p, eta, theta);
        return;
      }
    }
    // fallback: first state
    if (!trk.getTrackStates().empty()) {
      const auto& ts = trk.getTrackStates()[0];
      omega = ts.omega;
      d0    = ts.D0;
      z0    = ts.Z0;
      phi   = ts.phi;
      tanl  = ts.tanLambda;
      helixToKinematics(omega, tanl, alpha, pT, p, eta, theta);
    }
  };

  // ── clear per-hit vectors ──
  m_lcio_hit_x.clear(); m_lcio_hit_y.clear(); m_lcio_hit_z.clear();
  m_lcio_hit_r.clear(); m_lcio_hit_edep.clear(); m_lcio_hit_cellid.clear();
  m_lcio_hit_n = 0;
  m_gsf_hit_x.clear(); m_gsf_hit_y.clear(); m_gsf_hit_z.clear();
  m_gsf_hit_r.clear(); m_gsf_hit_edep.clear(); m_gsf_hit_cellid.clear();
  m_gsf_hit_n = 0;

  fillTrack(lcioCol,
            m_lcio_pT, m_lcio_p, m_lcio_eta, m_lcio_theta,
            m_lcio_phi, m_lcio_d0, m_lcio_z0,
            m_lcio_omega, m_lcio_tanl,
            m_lcio_chi2, m_lcio_ndf, m_lcio_nhits, m_lcio_type);
  if (lcioCol && lcioCol->size() > 0) {
    // per-hit data from CompleteTracks
    const auto& trk = (*lcioCol)[0];
    for (const auto& th : trk.getTrackerHits()) {
      if (!th.isAvailable()) continue;
      auto& pos = th.getPosition();
      m_lcio_hit_x.push_back((float)pos.x);
      m_lcio_hit_y.push_back((float)pos.y);
      m_lcio_hit_z.push_back((float)pos.z);
      m_lcio_hit_r.push_back((float)std::hypot(pos.x, pos.y));
      m_lcio_hit_edep.push_back(th.getEDep());
      m_lcio_hit_cellid.push_back(th.getCellID());
    }
    m_lcio_hit_n = (int)m_lcio_hit_x.size();
  }

  fillTrack(nullptr,
            m_gsf_pT, m_gsf_p, m_gsf_eta, m_gsf_theta,
            m_gsf_phi, m_gsf_d0, m_gsf_z0,
            m_gsf_omega, m_gsf_tanl,
            m_gsf_chi2, m_gsf_ndf, m_gsf_nhits, m_gsf_type);
  if (gsfCol && gsfCol->size() > 0) {
    try {
      fillTrack(gsfCol,
                m_gsf_pT, m_gsf_p, m_gsf_eta, m_gsf_theta,
                m_gsf_phi, m_gsf_d0, m_gsf_z0,
                m_gsf_omega, m_gsf_tanl,
                m_gsf_chi2, m_gsf_ndf, m_gsf_nhits, m_gsf_type);
      // per-hit data from GSFTracks
      const auto& trk = (*gsfCol)[0];
      auto trackerHits = trk.getTrackerHits();
      if (trackerHits.size() > 0) {
        for (const auto& th : trackerHits) {
          if (!th.isAvailable()) continue;
          auto& pos = th.getPosition();
          m_gsf_hit_x.push_back((float)pos.x);
          m_gsf_hit_y.push_back((float)pos.y);
          m_gsf_hit_z.push_back((float)pos.z);
          m_gsf_hit_r.push_back((float)std::hypot(pos.x, pos.y));
          m_gsf_hit_edep.push_back(th.getEDep());
          m_gsf_hit_cellid.push_back(th.getCellID());
        }
      }
      m_gsf_hit_n = (int)m_gsf_hit_x.size();
    } catch (const std::exception& e) {
      warning() << "Event " << m_iev << ": GSF track access failed — " << e.what() << " — skipping GSF per-hit data" << endmsg;
      m_gsf_hit_n = 0;
    } catch (...) {
      warning() << "Event " << m_iev << ": GSF track access failed (unknown exception) — skipping GSF per-hit data" << endmsg;
      m_gsf_hit_n = 0;
    }
  }

  m_weighted_gsf_available =
      weightedGsfCol && weightedGsfCol->size() > 0 ? 1 : 0;
  try {
    fillTrack(weightedGsfCol,
              m_weighted_gsf_pT, m_weighted_gsf_p,
              m_weighted_gsf_eta, m_weighted_gsf_theta,
              m_weighted_gsf_phi, m_weighted_gsf_d0, m_weighted_gsf_z0,
              m_weighted_gsf_omega, m_weighted_gsf_tanl,
              m_weighted_gsf_chi2, m_weighted_gsf_ndf,
              m_weighted_gsf_nhits, m_weighted_gsf_type);
  } catch (const std::exception& e) {
    warning() << "Event " << m_iev
              << ": WeightedMean GSF track access failed — " << e.what()
              << " — writing unavailable weighted fields" << endmsg;
    m_weighted_gsf_available = 0;
    fillTrack(nullptr,
              m_weighted_gsf_pT, m_weighted_gsf_p,
              m_weighted_gsf_eta, m_weighted_gsf_theta,
              m_weighted_gsf_phi, m_weighted_gsf_d0, m_weighted_gsf_z0,
              m_weighted_gsf_omega, m_weighted_gsf_tanl,
              m_weighted_gsf_chi2, m_weighted_gsf_ndf,
              m_weighted_gsf_nhits, m_weighted_gsf_type);
  } catch (...) {
    warning() << "Event " << m_iev
              << ": WeightedMean GSF track access failed (unknown "
                 "exception) — writing unavailable weighted fields"
              << endmsg;
    m_weighted_gsf_available = 0;
    fillTrack(nullptr,
              m_weighted_gsf_pT, m_weighted_gsf_p,
              m_weighted_gsf_eta, m_weighted_gsf_theta,
              m_weighted_gsf_phi, m_weighted_gsf_d0, m_weighted_gsf_z0,
              m_weighted_gsf_omega, m_weighted_gsf_tanl,
              m_weighted_gsf_chi2, m_weighted_gsf_ndf,
              m_weighted_gsf_nhits, m_weighted_gsf_type);
  }
  m_weighted_gsf_changed =
      (m_weighted_gsf_available && gsfCol && gsfCol->size() > 0 &&
       (m_weighted_gsf_omega != m_gsf_omega ||
        m_weighted_gsf_d0 != m_gsf_d0 ||
        m_weighted_gsf_z0 != m_gsf_z0 ||
        m_weighted_gsf_phi != m_gsf_phi ||
        m_weighted_gsf_tanl != m_gsf_tanl ||
        m_weighted_gsf_chi2 != m_gsf_chi2 ||
        m_weighted_gsf_ndf != m_gsf_ndf)) ? 1 : 0;

  m_ecal_gsf_available = ecalGsfCol && ecalGsfCol->size() > 0 ? 1 : 0;
  try {
    fillTrack(ecalGsfCol,
              m_ecal_gsf_pT, m_ecal_gsf_p,
              m_ecal_gsf_eta, m_ecal_gsf_theta,
              m_ecal_gsf_phi, m_ecal_gsf_d0, m_ecal_gsf_z0,
              m_ecal_gsf_omega, m_ecal_gsf_tanl,
              m_ecal_gsf_chi2, m_ecal_gsf_ndf,
              m_ecal_gsf_nhits, m_ecal_gsf_type);
  } catch (const std::exception& e) {
    warning() << "Event " << m_iev
              << ": ECAL-constrained GSF track access failed — " << e.what()
              << " — writing unavailable constrained fields" << endmsg;
    m_ecal_gsf_available = 0;
    fillTrack(nullptr,
              m_ecal_gsf_pT, m_ecal_gsf_p,
              m_ecal_gsf_eta, m_ecal_gsf_theta,
              m_ecal_gsf_phi, m_ecal_gsf_d0, m_ecal_gsf_z0,
              m_ecal_gsf_omega, m_ecal_gsf_tanl,
              m_ecal_gsf_chi2, m_ecal_gsf_ndf,
              m_ecal_gsf_nhits, m_ecal_gsf_type);
  } catch (...) {
    warning() << "Event " << m_iev
              << ": ECAL-constrained GSF track access failed (unknown "
                 "exception) — writing unavailable constrained fields"
              << endmsg;
    m_ecal_gsf_available = 0;
    fillTrack(nullptr,
              m_ecal_gsf_pT, m_ecal_gsf_p,
              m_ecal_gsf_eta, m_ecal_gsf_theta,
              m_ecal_gsf_phi, m_ecal_gsf_d0, m_ecal_gsf_z0,
              m_ecal_gsf_omega, m_ecal_gsf_tanl,
              m_ecal_gsf_chi2, m_ecal_gsf_ndf,
              m_ecal_gsf_nhits, m_ecal_gsf_type);
  }
  m_ecal_gsf_changed = (m_ecal_gsf_available && gsfCol && gsfCol->size() > 0 &&
      (m_ecal_gsf_omega != m_gsf_omega ||
       m_ecal_gsf_d0 != m_gsf_d0 ||
       m_ecal_gsf_z0 != m_gsf_z0 ||
       m_ecal_gsf_phi != m_gsf_phi ||
       m_ecal_gsf_tanl != m_gsf_tanl ||
       m_ecal_gsf_chi2 != m_gsf_chi2 ||
       m_ecal_gsf_ndf != m_gsf_ndf)) ? 1 : 0;

  // ── all hits from original collections ──
  m_all_hit_x.clear(); m_all_hit_y.clear(); m_all_hit_z.clear();
  m_all_hit_r.clear(); m_all_hit_edep.clear(); m_all_hit_cellid.clear();
  m_all_hit_det.clear();
  int detIdx = 0;
  for (const auto& colName : m_hitCollectionNames.value()) {
    SmartDataPtr<DataWrapper<edm4hep::TrackerHitCollection>> dw(eventSvc(), colName);
    const auto* ptr = dw ? dw->getData() : nullptr;
    if (ptr && ptr->size() > 0) {
      for (const auto& h : *ptr) {
        if (!h.isAvailable()) continue;
        auto& pos = h.getPosition();
        m_all_hit_x.push_back((float)pos.x);
        m_all_hit_y.push_back((float)pos.y);
        m_all_hit_z.push_back((float)pos.z);
        m_all_hit_r.push_back((float)std::hypot(pos.x, pos.y));
        m_all_hit_edep.push_back(h.getEDep());
        m_all_hit_cellid.push_back(h.getCellID());
        m_all_hit_det.push_back(detIdx);
      }
    }
    detIdx++;
  }
  m_all_hit_n = (int)m_all_hit_x.size();

  // ── resolution ──
  m_res_pT_gsf =
      (m_mc_pT > 0) ? (m_gsf_pT - m_mc_pT) / m_mc_pT : 0;
  m_res_pT_weighted_gsf = (m_mc_pT > 0 && m_weighted_gsf_available)
      ? (m_weighted_gsf_pT - m_mc_pT) / m_mc_pT : 0;
  m_res_pT_ecal_gsf = (m_mc_pT > 0 && m_ecal_gsf_available)
      ? (m_ecal_gsf_pT - m_mc_pT) / m_mc_pT : 0;
  m_res_pT_lcio =
      (m_mc_pT > 0) ? (m_lcio_pT - m_mc_pT) / m_mc_pT : 0;

  m_tree->Fill();
  return StatusCode::SUCCESS;
}

StatusCode RecGsfFlatTuple::finalize() {
  info() << "Writing " << m_tree->GetEntries() << " entries to "
         << m_outFileName << endmsg;
  m_file->cd();
  m_tree->Write("", TObject::kOverwrite);
  m_file->Close();
  delete m_file;
  m_file = nullptr;
  m_tree = nullptr;
  return StatusCode::SUCCESS;
}
