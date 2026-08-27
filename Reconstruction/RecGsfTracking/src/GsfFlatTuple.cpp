#include "GsfFlatTuple.h"

#include "GaudiKernel/SmartDataPtr.h"
#include "k4FWCore/DataWrapper.h"

#include <TFile.h>
#include <TTree.h>
#include <algorithm>
#include <cmath>
#include <limits>
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
  // smoother/reverse BestBranch per-hit
  m_tree->Branch("bestbranch_gsf_hit_n", &m_bestbranch_gsf_hit_n);
  m_tree->Branch("bestbranch_gsf_hit_x", &m_bestbranch_gsf_hit_x);
  m_tree->Branch("bestbranch_gsf_hit_y", &m_bestbranch_gsf_hit_y);
  m_tree->Branch("bestbranch_gsf_hit_z", &m_bestbranch_gsf_hit_z);
  m_tree->Branch("bestbranch_gsf_hit_r", &m_bestbranch_gsf_hit_r);
  m_tree->Branch("bestbranch_gsf_hit_edep", &m_bestbranch_gsf_hit_edep);
  m_tree->Branch("bestbranch_gsf_hit_cellid",
                 &m_bestbranch_gsf_hit_cellid);
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
  // paired smoother/reverse BestBranch GSF
  m_tree->Branch("bestbranch_gsf_available", &m_bestbranch_gsf_available);
  m_tree->Branch("bestbranch_gsf_pT",        &m_bestbranch_gsf_pT);
  m_tree->Branch("bestbranch_gsf_p",         &m_bestbranch_gsf_p);
  m_tree->Branch("bestbranch_gsf_eta",       &m_bestbranch_gsf_eta);
  m_tree->Branch("bestbranch_gsf_theta",     &m_bestbranch_gsf_theta);
  m_tree->Branch("bestbranch_gsf_phi",       &m_bestbranch_gsf_phi);
  m_tree->Branch("bestbranch_gsf_d0",        &m_bestbranch_gsf_d0);
  m_tree->Branch("bestbranch_gsf_z0",        &m_bestbranch_gsf_z0);
  m_tree->Branch("bestbranch_gsf_omega",     &m_bestbranch_gsf_omega);
  m_tree->Branch("bestbranch_gsf_tanl",      &m_bestbranch_gsf_tanl);
  m_tree->Branch("bestbranch_gsf_chi2",      &m_bestbranch_gsf_chi2);
  m_tree->Branch("bestbranch_gsf_ndf",       &m_bestbranch_gsf_ndf);
  m_tree->Branch("bestbranch_gsf_nhits",     &m_bestbranch_gsf_nhits);
  m_tree->Branch("bestbranch_gsf_type",      &m_bestbranch_gsf_type);
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
  // paired smoother/reverse full five-dimensional mixture-density mode GSF
  m_tree->Branch("fullmixture_gsf_available",
                 &m_fullmixture_gsf_available);
  m_tree->Branch("fullmixture_gsf_changed", &m_fullmixture_gsf_changed);
  m_tree->Branch("fullmixture_gsf_status",  &m_fullmixture_gsf_status);
  m_tree->Branch("fullmixture_gsf_pT",      &m_fullmixture_gsf_pT);
  m_tree->Branch("fullmixture_gsf_p",       &m_fullmixture_gsf_p);
  m_tree->Branch("fullmixture_gsf_eta",     &m_fullmixture_gsf_eta);
  m_tree->Branch("fullmixture_gsf_theta",   &m_fullmixture_gsf_theta);
  m_tree->Branch("fullmixture_gsf_phi",     &m_fullmixture_gsf_phi);
  m_tree->Branch("fullmixture_gsf_d0",      &m_fullmixture_gsf_d0);
  m_tree->Branch("fullmixture_gsf_z0",      &m_fullmixture_gsf_z0);
  m_tree->Branch("fullmixture_gsf_omega",   &m_fullmixture_gsf_omega);
  m_tree->Branch("fullmixture_gsf_tanl",    &m_fullmixture_gsf_tanl);
  m_tree->Branch("fullmixture_gsf_chi2",    &m_fullmixture_gsf_chi2);
  m_tree->Branch("fullmixture_gsf_ndf",     &m_fullmixture_gsf_ndf);
  m_tree->Branch("fullmixture_gsf_nhits",   &m_fullmixture_gsf_nhits);
  m_tree->Branch("fullmixture_gsf_type",    &m_fullmixture_gsf_type);
  // Positive-weight components of the final smoother/reverse mixture at IP.
  // These vectors are automatic outputs, not a configurable diagnostic.
  m_tree->Branch("final_mixture_component_available",
                 &m_final_mixture_component_available);
  m_tree->Branch("final_mixture_component_n",
                 &m_final_mixture_component_n);
  m_tree->Branch("final_mixture_component_input_track_index",
                 &m_final_mixture_component_input_track_index);
  m_tree->Branch("final_mixture_component_output_track_index",
                 &m_final_mixture_component_output_track_index);
  m_tree->Branch("final_mixture_component_index",
                 &m_final_mixture_component_index);
  m_tree->Branch("final_mixture_component_id",
                 &m_final_mixture_component_id);
  m_tree->Branch("final_mixture_component_source",
                 &m_final_mixture_component_source);
  m_tree->Branch("final_mixture_component_valid",
                 &m_final_mixture_component_valid);
  m_tree->Branch("final_mixture_component_weight",
                 &m_final_mixture_component_weight);
  m_tree->Branch("final_mixture_component_kappa",
                 &m_final_mixture_component_kappa);
  m_tree->Branch("final_mixture_component_kappa_variance",
                 &m_final_mixture_component_kappa_variance);
  m_tree->Branch("final_mixture_component_pT",
                 &m_final_mixture_component_pT);
  // Complete component ancestry, including evaluated nodes later rejected,
  // cut, or consumed by a many-to-one KL merge.
  m_tree->Branch("lineage_graph_available", &m_lineage_graph_available);
  m_tree->Branch("lineage_node_n", &m_lineage_node_n);
  m_tree->Branch("lineage_node_input_track_index",
                 &m_lineage_node_input_track_index);
  m_tree->Branch("lineage_node_output_track_index",
                 &m_lineage_node_output_track_index);
  m_tree->Branch("lineage_node_id", &m_lineage_node_id);
  m_tree->Branch("lineage_node_source", &m_lineage_node_source);
  m_tree->Branch("lineage_node_operation", &m_lineage_node_operation);
  m_tree->Branch("lineage_node_hit_index", &m_lineage_node_hit_index);
  m_tree->Branch("lineage_node_surface_index",
                 &m_lineage_node_surface_index);
  m_tree->Branch("lineage_node_component_id",
                 &m_lineage_node_component_id);
  m_tree->Branch("lineage_node_generation", &m_lineage_node_generation);
  m_tree->Branch("lineage_node_bh_component_index",
                 &m_lineage_node_bh_component_index);
  m_tree->Branch("lineage_node_measurement_status",
                 &m_lineage_node_measurement_status);
  m_tree->Branch("lineage_node_fate", &m_lineage_node_fate);
  m_tree->Branch("lineage_node_no_radiation",
                 &m_lineage_node_no_radiation);
  m_tree->Branch("lineage_node_best_branch",
                 &m_lineage_node_best_branch);
  m_tree->Branch("lineage_node_final_mixture",
                 &m_lineage_node_final_mixture);
  m_tree->Branch("lineage_node_valid", &m_lineage_node_valid);
  m_tree->Branch("lineage_node_weight", &m_lineage_node_weight);
  m_tree->Branch("lineage_node_prior_weight",
                 &m_lineage_node_prior_weight);
  m_tree->Branch("lineage_node_bh_weight", &m_lineage_node_bh_weight);
  m_tree->Branch("lineage_node_bh_mean", &m_lineage_node_bh_mean);
  m_tree->Branch("lineage_node_bh_variance",
                 &m_lineage_node_bh_variance);
  m_tree->Branch("lineage_node_material_tx0",
                 &m_lineage_node_material_tx0);
  m_tree->Branch("lineage_node_dchi2", &m_lineage_node_dchi2);
  m_tree->Branch("lineage_node_logdet_innovation",
                 &m_lineage_node_logdet_innovation);
  m_tree->Branch("lineage_node_log_unnormalized_posterior",
                 &m_lineage_node_log_unnormalized_posterior);
  m_tree->Branch("lineage_node_normalized_posterior",
                 &m_lineage_node_normalized_posterior);
  m_tree->Branch("lineage_node_predicted_kappa",
                 &m_lineage_node_predicted_kappa);
  m_tree->Branch("lineage_node_predicted_kappa_variance",
                 &m_lineage_node_predicted_kappa_variance);
  m_tree->Branch("lineage_node_predicted_pT",
                 &m_lineage_node_predicted_pT);
  m_tree->Branch("lineage_node_filtered_kappa",
                 &m_lineage_node_filtered_kappa);
  m_tree->Branch("lineage_node_filtered_kappa_variance",
                 &m_lineage_node_filtered_kappa_variance);
  m_tree->Branch("lineage_node_filtered_pT",
                 &m_lineage_node_filtered_pT);
  m_tree->Branch("lineage_node_dominant_lineage_fraction",
                 &m_lineage_node_dominant_lineage_fraction);
  m_tree->Branch("lineage_node_merge_cost", &m_lineage_node_merge_cost);
  m_tree->Branch("lineage_node_cms_smooth_valid",
                 &m_lineage_node_cms_smooth_valid);
  m_tree->Branch("lineage_node_cms_smooth_forward_predicted_kappa",
                 &m_lineage_node_cms_smooth_forward_predicted_kappa);
  m_tree->Branch(
      "lineage_node_cms_smooth_forward_predicted_kappa_variance",
      &m_lineage_node_cms_smooth_forward_predicted_kappa_variance);
  m_tree->Branch("lineage_node_cms_smooth_forward_updated_kappa",
                 &m_lineage_node_cms_smooth_forward_updated_kappa);
  m_tree->Branch(
      "lineage_node_cms_smooth_forward_updated_kappa_variance",
      &m_lineage_node_cms_smooth_forward_updated_kappa_variance);
  m_tree->Branch("lineage_node_cms_smooth_all_other_kappa",
                 &m_lineage_node_cms_smooth_all_other_kappa);
  m_tree->Branch("lineage_node_cms_smooth_all_other_kappa_variance",
                 &m_lineage_node_cms_smooth_all_other_kappa_variance);
  m_tree->Branch("lineage_node_cms_smooth_all_other_pT",
                 &m_lineage_node_cms_smooth_all_other_pT);
  m_tree->Branch("lineage_node_cms_smooth_all_hit_kappa",
                 &m_lineage_node_cms_smooth_all_hit_kappa);
  m_tree->Branch("lineage_node_cms_smooth_all_hit_kappa_variance",
                 &m_lineage_node_cms_smooth_all_hit_kappa_variance);
  m_tree->Branch("lineage_node_cms_smooth_all_hit_pT",
                 &m_lineage_node_cms_smooth_all_hit_pT);
  m_tree->Branch("lineage_node_cms_smooth_all_other_log_overlap",
                 &m_lineage_node_cms_smooth_all_other_log_overlap);
  m_tree->Branch(
      "lineage_node_cms_smooth_all_hit_compatibility_dchi2",
      &m_lineage_node_cms_smooth_all_hit_compatibility_dchi2);
  m_tree->Branch(
      "lineage_node_cms_smooth_all_hit_compatibility_logdet",
      &m_lineage_node_cms_smooth_all_hit_compatibility_logdet);
  m_tree->Branch("lineage_node_cms_smooth_all_hit_log_overlap",
                 &m_lineage_node_cms_smooth_all_hit_log_overlap);
  m_tree->Branch("lineage_node_cms_smooth_local_dchi2",
                 &m_lineage_node_cms_smooth_local_dchi2);
  m_tree->Branch("lineage_node_cms_smooth_local_logdet_innovation",
                 &m_lineage_node_cms_smooth_local_logdet_innovation);
  m_tree->Branch("lineage_node_cms_smooth_local_log_likelihood",
                 &m_lineage_node_cms_smooth_local_log_likelihood);
  m_tree->Branch("lineage_node_cms_smooth_log_evidence",
                 &m_lineage_node_cms_smooth_log_evidence);
  m_tree->Branch("lineage_node_cms_smooth_log_weight_with_prior",
                 &m_lineage_node_cms_smooth_log_weight_with_prior);
  m_tree->Branch("lineage_node_cms_smooth_normalized_evidence",
                 &m_lineage_node_cms_smooth_normalized_evidence);
  m_tree->Branch("lineage_node_cms_smooth_normalized_weight_with_prior",
                 &m_lineage_node_cms_smooth_normalized_weight_with_prior);
  m_tree->Branch("lineage_edge_n", &m_lineage_edge_n);
  m_tree->Branch("lineage_edge_input_track_index",
                 &m_lineage_edge_input_track_index);
  m_tree->Branch("lineage_edge_output_track_index",
                 &m_lineage_edge_output_track_index);
  m_tree->Branch("lineage_edge_from_node_id",
                 &m_lineage_edge_from_node_id);
  m_tree->Branch("lineage_edge_to_node_id", &m_lineage_edge_to_node_id);
  m_tree->Branch("lineage_edge_operation", &m_lineage_edge_operation);
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
  m_tree->Branch("res_pT_bestbranch_gsf", &m_res_pT_bestbranch_gsf);
  m_tree->Branch("res_pT_weighted_gsf", &m_res_pT_weighted_gsf);
  m_tree->Branch("res_pT_fullmixture_gsf", &m_res_pT_fullmixture_gsf);
  m_tree->Branch("res_pT_ecal_gsf", &m_res_pT_ecal_gsf);
  m_tree->Branch("res_pT_lcio",     &m_res_pT_lcio);

  info() << "Output: " << m_outFileName
         << " genericTrackSource="
         << (m_useGlobalLossTracks.value() ? "GlobalLossTracks"
                                           : "GSFTracks")
         << " bestBranchSource=GSFTracksBestBranch"
         << " fullMixtureModeSource=GSFTracksFullMixtureMode"
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
  SmartDataPtr<DataWrapper<edm4hep::TrackCollection>> genericGsfWrapper(
      eventSvc(), "GSFTracks");
  const auto* gsfCol = m_useGlobalLossTracks.value()
                           ? m_inGlobalLossTracks.get()
                           : (genericGsfWrapper
                                  ? genericGsfWrapper->getData() : nullptr);
  SmartDataPtr<DataWrapper<edm4hep::TrackCollection>> bestBranchGsfWrapper(
      eventSvc(), "GSFTracksBestBranch");
  const auto* bestBranchGsfCol = bestBranchGsfWrapper
      ? bestBranchGsfWrapper->getData() : nullptr;
  m_final_mixture_component_available = 0;
  m_final_mixture_component_n = 0;
  m_final_mixture_component_input_track_index.clear();
  m_final_mixture_component_output_track_index.clear();
  m_final_mixture_component_index.clear();
  m_final_mixture_component_id.clear();
  m_final_mixture_component_source.clear();
  m_final_mixture_component_valid.clear();
  m_final_mixture_component_weight.clear();
  m_final_mixture_component_kappa.clear();
  m_final_mixture_component_kappa_variance.clear();
  m_final_mixture_component_pT.clear();
  try {
    const auto* inputTrackIndices =
        m_inFinalMixtureComponentInputTrackIndex.get();
    const auto* outputTrackIndices =
        m_inFinalMixtureComponentOutputTrackIndex.get();
    const auto* componentIndices = m_inFinalMixtureComponentIndex.get();
    const auto* componentIDs = m_inFinalMixtureComponentID.get();
    const auto* componentSources = m_inFinalMixtureComponentSource.get();
    const auto* componentValidity = m_inFinalMixtureComponentValid.get();
    const auto* componentWeights = m_inFinalMixtureComponentWeight.get();
    const auto* componentKappas = m_inFinalMixtureComponentKappa.get();
    const auto* componentKappaVariances =
        m_inFinalMixtureComponentKappaVariance.get();
    const std::size_t componentCount = componentWeights
        ? componentWeights->size() : 0;
    const bool consistent = inputTrackIndices && outputTrackIndices &&
        componentIndices && componentIDs && componentSources &&
        componentValidity && componentKappas && componentKappaVariances &&
        inputTrackIndices->size() == componentCount &&
        outputTrackIndices->size() == componentCount &&
        componentIndices->size() == componentCount &&
        componentIDs->size() == componentCount &&
        componentSources->size() == componentCount &&
        componentValidity->size() == componentCount &&
        componentKappas->size() == componentCount &&
        componentKappaVariances->size() == componentCount;
    if (!consistent) {
      warning() << "Event " << m_iev
                << ": inconsistent final-mixture component collections; "
                   "writing empty component vectors"
                << endmsg;
    } else {
      m_final_mixture_component_input_track_index.assign(
          inputTrackIndices->begin(), inputTrackIndices->end());
      m_final_mixture_component_output_track_index.assign(
          outputTrackIndices->begin(), outputTrackIndices->end());
      m_final_mixture_component_index.assign(
          componentIndices->begin(), componentIndices->end());
      m_final_mixture_component_id.assign(
          componentIDs->begin(), componentIDs->end());
      m_final_mixture_component_source.assign(
          componentSources->begin(), componentSources->end());
      m_final_mixture_component_valid.assign(
          componentValidity->begin(), componentValidity->end());
      m_final_mixture_component_weight.assign(
          componentWeights->begin(), componentWeights->end());
      m_final_mixture_component_kappa.assign(
          componentKappas->begin(), componentKappas->end());
      m_final_mixture_component_kappa_variance.assign(
          componentKappaVariances->begin(),
          componentKappaVariances->end());
      m_final_mixture_component_pT.reserve(componentCount);
      for (std::size_t componentIndex = 0;
           componentIndex < componentCount; ++componentIndex) {
        const double kappa = (*componentKappas)[componentIndex];
        m_final_mixture_component_pT.push_back(
            (*componentValidity)[componentIndex] == 1 &&
                    std::isfinite(kappa) && std::abs(kappa) > 1.0e-15
                ? 1.0 / std::abs(kappa)
                : std::numeric_limits<double>::quiet_NaN());
      }
      m_final_mixture_component_n =
          static_cast<int>(componentCount);
      m_final_mixture_component_available = componentCount > 0 ? 1 : 0;
    }
  } catch (...) {
    // Older producers and non-mixture methods do not provide these optional
    // automatic collections. Keep the vectors empty without changing flow.
  }
  m_lineage_graph_available = 0;
  m_lineage_node_n = 0;
  m_lineage_edge_n = 0;
  m_lineage_node_input_track_index.clear();
  m_lineage_node_output_track_index.clear();
  m_lineage_node_id.clear();
  m_lineage_node_source.clear();
  m_lineage_node_operation.clear();
  m_lineage_node_hit_index.clear();
  m_lineage_node_surface_index.clear();
  m_lineage_node_component_id.clear();
  m_lineage_node_generation.clear();
  m_lineage_node_bh_component_index.clear();
  m_lineage_node_measurement_status.clear();
  m_lineage_node_fate.clear();
  m_lineage_node_no_radiation.clear();
  m_lineage_node_best_branch.clear();
  m_lineage_node_final_mixture.clear();
  m_lineage_node_valid.clear();
  m_lineage_node_weight.clear();
  m_lineage_node_prior_weight.clear();
  m_lineage_node_bh_weight.clear();
  m_lineage_node_bh_mean.clear();
  m_lineage_node_bh_variance.clear();
  m_lineage_node_material_tx0.clear();
  m_lineage_node_dchi2.clear();
  m_lineage_node_logdet_innovation.clear();
  m_lineage_node_log_unnormalized_posterior.clear();
  m_lineage_node_normalized_posterior.clear();
  m_lineage_node_predicted_kappa.clear();
  m_lineage_node_predicted_kappa_variance.clear();
  m_lineage_node_predicted_pT.clear();
  m_lineage_node_filtered_kappa.clear();
  m_lineage_node_filtered_kappa_variance.clear();
  m_lineage_node_filtered_pT.clear();
  m_lineage_node_dominant_lineage_fraction.clear();
  m_lineage_node_merge_cost.clear();
  m_lineage_node_cms_smooth_valid.clear();
  m_lineage_node_cms_smooth_forward_predicted_kappa.clear();
  m_lineage_node_cms_smooth_forward_predicted_kappa_variance.clear();
  m_lineage_node_cms_smooth_forward_updated_kappa.clear();
  m_lineage_node_cms_smooth_forward_updated_kappa_variance.clear();
  m_lineage_node_cms_smooth_all_other_kappa.clear();
  m_lineage_node_cms_smooth_all_other_kappa_variance.clear();
  m_lineage_node_cms_smooth_all_other_pT.clear();
  m_lineage_node_cms_smooth_all_hit_kappa.clear();
  m_lineage_node_cms_smooth_all_hit_kappa_variance.clear();
  m_lineage_node_cms_smooth_all_hit_pT.clear();
  m_lineage_node_cms_smooth_all_other_log_overlap.clear();
  m_lineage_node_cms_smooth_all_hit_compatibility_dchi2.clear();
  m_lineage_node_cms_smooth_all_hit_compatibility_logdet.clear();
  m_lineage_node_cms_smooth_all_hit_log_overlap.clear();
  m_lineage_node_cms_smooth_local_dchi2.clear();
  m_lineage_node_cms_smooth_local_logdet_innovation.clear();
  m_lineage_node_cms_smooth_local_log_likelihood.clear();
  m_lineage_node_cms_smooth_log_evidence.clear();
  m_lineage_node_cms_smooth_log_weight_with_prior.clear();
  m_lineage_node_cms_smooth_normalized_evidence.clear();
  m_lineage_node_cms_smooth_normalized_weight_with_prior.clear();
  m_lineage_edge_input_track_index.clear();
  m_lineage_edge_output_track_index.clear();
  m_lineage_edge_from_node_id.clear();
  m_lineage_edge_to_node_id.clear();
  m_lineage_edge_operation.clear();
  try {
    const auto* nodeInputTrackIndices =
        m_inLineageNodeInputTrackIndex.get();
    const auto* nodeOutputTrackIndices =
        m_inLineageNodeOutputTrackIndex.get();
    const auto* nodeIds = m_inLineageNodeId.get();
    const auto* nodeSources = m_inLineageNodeSource.get();
    const auto* nodeOperations = m_inLineageNodeOperation.get();
    const auto* nodeHitIndices = m_inLineageNodeHitIndex.get();
    const auto* nodeSurfaceIndices = m_inLineageNodeSurfaceIndex.get();
    const auto* nodeComponentIds = m_inLineageNodeComponentId.get();
    const auto* nodeGenerations = m_inLineageNodeGeneration.get();
    const auto* nodeBhComponentIndices =
        m_inLineageNodeBhComponentIndex.get();
    const auto* nodeMeasurementStatuses =
        m_inLineageNodeMeasurementStatus.get();
    const auto* nodeFates = m_inLineageNodeFate.get();
    const auto* nodeNoRadiation = m_inLineageNodeNoRadiation.get();
    const auto* nodeBestBranch = m_inLineageNodeBestBranch.get();
    const auto* nodeFinalMixture = m_inLineageNodeFinalMixture.get();
    const auto* nodeValidity = m_inLineageNodeValid.get();
    const auto* nodeWeights = m_inLineageNodeWeight.get();
    const auto* nodePriorWeights = m_inLineageNodePriorWeight.get();
    const auto* nodeBhWeights = m_inLineageNodeBhWeight.get();
    const auto* nodeBhMeans = m_inLineageNodeBhMean.get();
    const auto* nodeBhVariances = m_inLineageNodeBhVariance.get();
    const auto* nodeMaterialTX0 = m_inLineageNodeMaterialTX0.get();
    const auto* nodeDChi2 = m_inLineageNodeDChi2.get();
    const auto* nodeLogDetInnovation =
        m_inLineageNodeLogDetInnovation.get();
    const auto* nodeLogUnnormalizedPosterior =
        m_inLineageNodeLogUnnormalizedPosterior.get();
    const auto* nodeNormalizedPosterior =
        m_inLineageNodeNormalizedPosterior.get();
    const auto* nodePredictedKappa = m_inLineageNodePredictedKappa.get();
    const auto* nodePredictedKappaVariance =
        m_inLineageNodePredictedKappaVariance.get();
    const auto* nodeFilteredKappa = m_inLineageNodeFilteredKappa.get();
    const auto* nodeFilteredKappaVariance =
        m_inLineageNodeFilteredKappaVariance.get();
    const auto* nodeDominantLineageFraction =
        m_inLineageNodeDominantLineageFraction.get();
    const auto* nodeMergeCost = m_inLineageNodeMergeCost.get();
    const auto* nodeCmsSmoothValid =
        m_inLineageNodeCmsSmoothValid.get();
    const auto* nodeCmsSmoothForwardPredictedKappa =
        m_inLineageNodeCmsSmoothForwardPredictedKappa.get();
    const auto* nodeCmsSmoothForwardPredictedKappaVariance =
        m_inLineageNodeCmsSmoothForwardPredictedKappaVariance.get();
    const auto* nodeCmsSmoothForwardUpdatedKappa =
        m_inLineageNodeCmsSmoothForwardUpdatedKappa.get();
    const auto* nodeCmsSmoothForwardUpdatedKappaVariance =
        m_inLineageNodeCmsSmoothForwardUpdatedKappaVariance.get();
    const auto* nodeCmsSmoothAllOtherKappa =
        m_inLineageNodeCmsSmoothAllOtherKappa.get();
    const auto* nodeCmsSmoothAllOtherKappaVariance =
        m_inLineageNodeCmsSmoothAllOtherKappaVariance.get();
    const auto* nodeCmsSmoothAllHitKappa =
        m_inLineageNodeCmsSmoothAllHitKappa.get();
    const auto* nodeCmsSmoothAllHitKappaVariance =
        m_inLineageNodeCmsSmoothAllHitKappaVariance.get();
    const auto* nodeCmsSmoothAllOtherLogOverlap =
        m_inLineageNodeCmsSmoothAllOtherLogOverlap.get();
    const auto* nodeCmsSmoothAllHitCompatibilityDChi2 =
        m_inLineageNodeCmsSmoothAllHitCompatibilityDChi2.get();
    const auto* nodeCmsSmoothAllHitCompatibilityLogDet =
        m_inLineageNodeCmsSmoothAllHitCompatibilityLogDet.get();
    const auto* nodeCmsSmoothAllHitLogOverlap =
        m_inLineageNodeCmsSmoothAllHitLogOverlap.get();
    const auto* nodeCmsSmoothLocalDChi2 =
        m_inLineageNodeCmsSmoothLocalDChi2.get();
    const auto* nodeCmsSmoothLocalLogDetInnovation =
        m_inLineageNodeCmsSmoothLocalLogDetInnovation.get();
    const auto* nodeCmsSmoothLocalLogLikelihood =
        m_inLineageNodeCmsSmoothLocalLogLikelihood.get();
    const auto* nodeCmsSmoothLogEvidence =
        m_inLineageNodeCmsSmoothLogEvidence.get();
    const auto* nodeCmsSmoothLogWeightWithPrior =
        m_inLineageNodeCmsSmoothLogWeightWithPrior.get();
    const auto* nodeCmsSmoothNormalizedEvidence =
        m_inLineageNodeCmsSmoothNormalizedEvidence.get();
    const auto* nodeCmsSmoothNormalizedWeightWithPrior =
        m_inLineageNodeCmsSmoothNormalizedWeightWithPrior.get();
    const std::size_t nodeCount = nodeIds ? nodeIds->size() : 0;
    auto nodeSizeIsConsistent = [nodeCount](const auto* collection) {
      return collection && collection->size() == nodeCount;
    };
    const bool nodesConsistent = nodeSizeIsConsistent(nodeInputTrackIndices) &&
        nodeSizeIsConsistent(nodeOutputTrackIndices) &&
        nodeSizeIsConsistent(nodeSources) &&
        nodeSizeIsConsistent(nodeOperations) &&
        nodeSizeIsConsistent(nodeHitIndices) &&
        nodeSizeIsConsistent(nodeSurfaceIndices) &&
        nodeSizeIsConsistent(nodeComponentIds) &&
        nodeSizeIsConsistent(nodeGenerations) &&
        nodeSizeIsConsistent(nodeBhComponentIndices) &&
        nodeSizeIsConsistent(nodeMeasurementStatuses) &&
        nodeSizeIsConsistent(nodeFates) &&
        nodeSizeIsConsistent(nodeNoRadiation) &&
        nodeSizeIsConsistent(nodeBestBranch) &&
        nodeSizeIsConsistent(nodeFinalMixture) &&
        nodeSizeIsConsistent(nodeValidity) &&
        nodeSizeIsConsistent(nodeWeights) &&
        nodeSizeIsConsistent(nodePriorWeights) &&
        nodeSizeIsConsistent(nodeBhWeights) &&
        nodeSizeIsConsistent(nodeBhMeans) &&
        nodeSizeIsConsistent(nodeBhVariances) &&
        nodeSizeIsConsistent(nodeMaterialTX0) &&
        nodeSizeIsConsistent(nodeDChi2) &&
        nodeSizeIsConsistent(nodeLogDetInnovation) &&
        nodeSizeIsConsistent(nodeLogUnnormalizedPosterior) &&
        nodeSizeIsConsistent(nodeNormalizedPosterior) &&
        nodeSizeIsConsistent(nodePredictedKappa) &&
        nodeSizeIsConsistent(nodePredictedKappaVariance) &&
        nodeSizeIsConsistent(nodeFilteredKappa) &&
        nodeSizeIsConsistent(nodeFilteredKappaVariance) &&
        nodeSizeIsConsistent(nodeDominantLineageFraction) &&
        nodeSizeIsConsistent(nodeMergeCost) &&
        nodeSizeIsConsistent(nodeCmsSmoothValid) &&
        nodeSizeIsConsistent(nodeCmsSmoothForwardPredictedKappa) &&
        nodeSizeIsConsistent(nodeCmsSmoothForwardPredictedKappaVariance) &&
        nodeSizeIsConsistent(nodeCmsSmoothForwardUpdatedKappa) &&
        nodeSizeIsConsistent(nodeCmsSmoothForwardUpdatedKappaVariance) &&
        nodeSizeIsConsistent(nodeCmsSmoothAllOtherKappa) &&
        nodeSizeIsConsistent(nodeCmsSmoothAllOtherKappaVariance) &&
        nodeSizeIsConsistent(nodeCmsSmoothAllHitKappa) &&
        nodeSizeIsConsistent(nodeCmsSmoothAllHitKappaVariance) &&
        nodeSizeIsConsistent(nodeCmsSmoothAllOtherLogOverlap) &&
        nodeSizeIsConsistent(nodeCmsSmoothAllHitCompatibilityDChi2) &&
        nodeSizeIsConsistent(nodeCmsSmoothAllHitCompatibilityLogDet) &&
        nodeSizeIsConsistent(nodeCmsSmoothAllHitLogOverlap) &&
        nodeSizeIsConsistent(nodeCmsSmoothLocalDChi2) &&
        nodeSizeIsConsistent(nodeCmsSmoothLocalLogDetInnovation) &&
        nodeSizeIsConsistent(nodeCmsSmoothLocalLogLikelihood) &&
        nodeSizeIsConsistent(nodeCmsSmoothLogEvidence) &&
        nodeSizeIsConsistent(nodeCmsSmoothLogWeightWithPrior) &&
        nodeSizeIsConsistent(nodeCmsSmoothNormalizedEvidence) &&
        nodeSizeIsConsistent(nodeCmsSmoothNormalizedWeightWithPrior);

    const auto* edgeInputTrackIndices =
        m_inLineageEdgeInputTrackIndex.get();
    const auto* edgeOutputTrackIndices =
        m_inLineageEdgeOutputTrackIndex.get();
    const auto* edgeFromNodeIds = m_inLineageEdgeFromNodeId.get();
    const auto* edgeToNodeIds = m_inLineageEdgeToNodeId.get();
    const auto* edgeOperations = m_inLineageEdgeOperation.get();
    const std::size_t edgeCount = edgeFromNodeIds
        ? edgeFromNodeIds->size() : 0;
    auto edgeSizeIsConsistent = [edgeCount](const auto* collection) {
      return collection && collection->size() == edgeCount;
    };
    const bool edgesConsistent = edgeSizeIsConsistent(edgeInputTrackIndices) &&
        edgeSizeIsConsistent(edgeOutputTrackIndices) &&
        edgeSizeIsConsistent(edgeToNodeIds) &&
        edgeSizeIsConsistent(edgeOperations);

    if (!nodesConsistent || !edgesConsistent) {
      warning() << "Event " << m_iev
                << ": inconsistent component-lineage graph collections; "
                   "writing an empty lineage graph"
                << endmsg;
    } else {
      m_lineage_node_input_track_index.assign(
          nodeInputTrackIndices->begin(), nodeInputTrackIndices->end());
      m_lineage_node_output_track_index.assign(
          nodeOutputTrackIndices->begin(), nodeOutputTrackIndices->end());
      m_lineage_node_id.assign(nodeIds->begin(), nodeIds->end());
      m_lineage_node_source.assign(nodeSources->begin(), nodeSources->end());
      m_lineage_node_operation.assign(
          nodeOperations->begin(), nodeOperations->end());
      m_lineage_node_hit_index.assign(
          nodeHitIndices->begin(), nodeHitIndices->end());
      m_lineage_node_surface_index.assign(
          nodeSurfaceIndices->begin(), nodeSurfaceIndices->end());
      m_lineage_node_component_id.assign(
          nodeComponentIds->begin(), nodeComponentIds->end());
      m_lineage_node_generation.assign(
          nodeGenerations->begin(), nodeGenerations->end());
      m_lineage_node_bh_component_index.assign(
          nodeBhComponentIndices->begin(), nodeBhComponentIndices->end());
      m_lineage_node_measurement_status.assign(
          nodeMeasurementStatuses->begin(), nodeMeasurementStatuses->end());
      m_lineage_node_fate.assign(nodeFates->begin(), nodeFates->end());
      m_lineage_node_no_radiation.assign(
          nodeNoRadiation->begin(), nodeNoRadiation->end());
      m_lineage_node_best_branch.assign(
          nodeBestBranch->begin(), nodeBestBranch->end());
      m_lineage_node_final_mixture.assign(
          nodeFinalMixture->begin(), nodeFinalMixture->end());
      m_lineage_node_valid.assign(nodeValidity->begin(), nodeValidity->end());
      m_lineage_node_weight.assign(nodeWeights->begin(), nodeWeights->end());
      m_lineage_node_prior_weight.assign(
          nodePriorWeights->begin(), nodePriorWeights->end());
      m_lineage_node_bh_weight.assign(
          nodeBhWeights->begin(), nodeBhWeights->end());
      m_lineage_node_bh_mean.assign(nodeBhMeans->begin(), nodeBhMeans->end());
      m_lineage_node_bh_variance.assign(
          nodeBhVariances->begin(), nodeBhVariances->end());
      m_lineage_node_material_tx0.assign(
          nodeMaterialTX0->begin(), nodeMaterialTX0->end());
      m_lineage_node_dchi2.assign(nodeDChi2->begin(), nodeDChi2->end());
      m_lineage_node_logdet_innovation.assign(
          nodeLogDetInnovation->begin(), nodeLogDetInnovation->end());
      m_lineage_node_log_unnormalized_posterior.assign(
          nodeLogUnnormalizedPosterior->begin(),
          nodeLogUnnormalizedPosterior->end());
      m_lineage_node_normalized_posterior.assign(
          nodeNormalizedPosterior->begin(), nodeNormalizedPosterior->end());
      m_lineage_node_predicted_kappa.assign(
          nodePredictedKappa->begin(), nodePredictedKappa->end());
      m_lineage_node_predicted_kappa_variance.assign(
          nodePredictedKappaVariance->begin(),
          nodePredictedKappaVariance->end());
      m_lineage_node_filtered_kappa.assign(
          nodeFilteredKappa->begin(), nodeFilteredKappa->end());
      m_lineage_node_filtered_kappa_variance.assign(
          nodeFilteredKappaVariance->begin(),
          nodeFilteredKappaVariance->end());
      m_lineage_node_dominant_lineage_fraction.assign(
          nodeDominantLineageFraction->begin(),
          nodeDominantLineageFraction->end());
      m_lineage_node_merge_cost.assign(
          nodeMergeCost->begin(), nodeMergeCost->end());
      m_lineage_node_cms_smooth_valid.assign(
          nodeCmsSmoothValid->begin(), nodeCmsSmoothValid->end());
      m_lineage_node_cms_smooth_forward_predicted_kappa.assign(
          nodeCmsSmoothForwardPredictedKappa->begin(),
          nodeCmsSmoothForwardPredictedKappa->end());
      m_lineage_node_cms_smooth_forward_predicted_kappa_variance.assign(
          nodeCmsSmoothForwardPredictedKappaVariance->begin(),
          nodeCmsSmoothForwardPredictedKappaVariance->end());
      m_lineage_node_cms_smooth_forward_updated_kappa.assign(
          nodeCmsSmoothForwardUpdatedKappa->begin(),
          nodeCmsSmoothForwardUpdatedKappa->end());
      m_lineage_node_cms_smooth_forward_updated_kappa_variance.assign(
          nodeCmsSmoothForwardUpdatedKappaVariance->begin(),
          nodeCmsSmoothForwardUpdatedKappaVariance->end());
      m_lineage_node_cms_smooth_all_other_kappa.assign(
          nodeCmsSmoothAllOtherKappa->begin(),
          nodeCmsSmoothAllOtherKappa->end());
      m_lineage_node_cms_smooth_all_other_kappa_variance.assign(
          nodeCmsSmoothAllOtherKappaVariance->begin(),
          nodeCmsSmoothAllOtherKappaVariance->end());
      m_lineage_node_cms_smooth_all_hit_kappa.assign(
          nodeCmsSmoothAllHitKappa->begin(),
          nodeCmsSmoothAllHitKappa->end());
      m_lineage_node_cms_smooth_all_hit_kappa_variance.assign(
          nodeCmsSmoothAllHitKappaVariance->begin(),
          nodeCmsSmoothAllHitKappaVariance->end());
      m_lineage_node_cms_smooth_all_other_log_overlap.assign(
          nodeCmsSmoothAllOtherLogOverlap->begin(),
          nodeCmsSmoothAllOtherLogOverlap->end());
      m_lineage_node_cms_smooth_all_hit_compatibility_dchi2.assign(
          nodeCmsSmoothAllHitCompatibilityDChi2->begin(),
          nodeCmsSmoothAllHitCompatibilityDChi2->end());
      m_lineage_node_cms_smooth_all_hit_compatibility_logdet.assign(
          nodeCmsSmoothAllHitCompatibilityLogDet->begin(),
          nodeCmsSmoothAllHitCompatibilityLogDet->end());
      m_lineage_node_cms_smooth_all_hit_log_overlap.assign(
          nodeCmsSmoothAllHitLogOverlap->begin(),
          nodeCmsSmoothAllHitLogOverlap->end());
      m_lineage_node_cms_smooth_local_dchi2.assign(
          nodeCmsSmoothLocalDChi2->begin(),
          nodeCmsSmoothLocalDChi2->end());
      m_lineage_node_cms_smooth_local_logdet_innovation.assign(
          nodeCmsSmoothLocalLogDetInnovation->begin(),
          nodeCmsSmoothLocalLogDetInnovation->end());
      m_lineage_node_cms_smooth_local_log_likelihood.assign(
          nodeCmsSmoothLocalLogLikelihood->begin(),
          nodeCmsSmoothLocalLogLikelihood->end());
      m_lineage_node_cms_smooth_log_evidence.assign(
          nodeCmsSmoothLogEvidence->begin(),
          nodeCmsSmoothLogEvidence->end());
      m_lineage_node_cms_smooth_log_weight_with_prior.assign(
          nodeCmsSmoothLogWeightWithPrior->begin(),
          nodeCmsSmoothLogWeightWithPrior->end());
      m_lineage_node_cms_smooth_normalized_evidence.assign(
          nodeCmsSmoothNormalizedEvidence->begin(),
          nodeCmsSmoothNormalizedEvidence->end());
      m_lineage_node_cms_smooth_normalized_weight_with_prior.assign(
          nodeCmsSmoothNormalizedWeightWithPrior->begin(),
          nodeCmsSmoothNormalizedWeightWithPrior->end());
      m_lineage_node_predicted_pT.reserve(nodeCount);
      m_lineage_node_filtered_pT.reserve(nodeCount);
      m_lineage_node_cms_smooth_all_other_pT.reserve(nodeCount);
      m_lineage_node_cms_smooth_all_hit_pT.reserve(nodeCount);
      for (std::size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex) {
        const double predictedKappa = (*nodePredictedKappa)[nodeIndex];
        const double filteredKappa = (*nodeFilteredKappa)[nodeIndex];
        const double allOtherKappa = (*nodeCmsSmoothAllOtherKappa)[nodeIndex];
        const double allHitKappa = (*nodeCmsSmoothAllHitKappa)[nodeIndex];
        m_lineage_node_predicted_pT.push_back(
            std::isfinite(predictedKappa) &&
                    std::abs(predictedKappa) > 1.0e-15
                ? 1.0 / std::abs(predictedKappa)
                : std::numeric_limits<double>::quiet_NaN());
        m_lineage_node_filtered_pT.push_back(
            std::isfinite(filteredKappa) &&
                    std::abs(filteredKappa) > 1.0e-15
                ? 1.0 / std::abs(filteredKappa)
                : std::numeric_limits<double>::quiet_NaN());
        m_lineage_node_cms_smooth_all_other_pT.push_back(
            std::isfinite(allOtherKappa) &&
                    std::abs(allOtherKappa) > 1.0e-15
                ? 1.0 / std::abs(allOtherKappa)
                : std::numeric_limits<double>::quiet_NaN());
        m_lineage_node_cms_smooth_all_hit_pT.push_back(
            std::isfinite(allHitKappa) &&
                    std::abs(allHitKappa) > 1.0e-15
                ? 1.0 / std::abs(allHitKappa)
                : std::numeric_limits<double>::quiet_NaN());
      }
      m_lineage_edge_input_track_index.assign(
          edgeInputTrackIndices->begin(), edgeInputTrackIndices->end());
      m_lineage_edge_output_track_index.assign(
          edgeOutputTrackIndices->begin(), edgeOutputTrackIndices->end());
      m_lineage_edge_from_node_id.assign(
          edgeFromNodeIds->begin(), edgeFromNodeIds->end());
      m_lineage_edge_to_node_id.assign(
          edgeToNodeIds->begin(), edgeToNodeIds->end());
      m_lineage_edge_operation.assign(
          edgeOperations->begin(), edgeOperations->end());
      m_lineage_node_n = static_cast<int>(nodeCount);
      m_lineage_edge_n = static_cast<int>(edgeCount);
      m_lineage_graph_available = nodeCount > 0 ? 1 : 0;
    }
  } catch (...) {
    // Older producers and methods without a final mixture do not provide a
    // lineage graph. Keep the default-on tuple branches present and empty.
  }
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
  const auto* weightedGsfCol = weightedGsfWrapper
      ? weightedGsfWrapper->getData() : nullptr;
  SmartDataPtr<DataWrapper<edm4hep::TrackCollection>> fullMixtureGsfWrapper(
      eventSvc(), "GSFTracksFullMixtureMode");
  const auto* fullMixtureGsfCol = fullMixtureGsfWrapper
      ? fullMixtureGsfWrapper->getData() : nullptr;
  SmartDataPtr<DataWrapper<podio::UserDataCollection<std::int32_t>>>
      fullMixtureStatusWrapper(eventSvc(), "GSFFullMixtureModeStatus");
  const auto* fullMixtureStatus = fullMixtureStatusWrapper
      ? fullMixtureStatusWrapper->getData() : nullptr;
  m_fullmixture_gsf_status =
      fullMixtureModeStatusValue(FullMixtureModeStatus::NotApplicable);
  if (fullMixtureStatus && !fullMixtureStatus->empty())
    m_fullmixture_gsf_status = (*fullMixtureStatus)[0];

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
  m_bestbranch_gsf_hit_x.clear(); m_bestbranch_gsf_hit_y.clear();
  m_bestbranch_gsf_hit_z.clear(); m_bestbranch_gsf_hit_r.clear();
  m_bestbranch_gsf_hit_edep.clear();
  m_bestbranch_gsf_hit_cellid.clear();
  m_bestbranch_gsf_hit_n = 0;

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
    } catch (const std::exception& e) {
      warning() << "Event " << m_iev << ": generic GSF track access failed — "
                << e.what() << " — writing empty generic fields" << endmsg;
      fillTrack(nullptr,
                m_gsf_pT, m_gsf_p, m_gsf_eta, m_gsf_theta,
                m_gsf_phi, m_gsf_d0, m_gsf_z0,
                m_gsf_omega, m_gsf_tanl,
                m_gsf_chi2, m_gsf_ndf, m_gsf_nhits, m_gsf_type);
    } catch (...) {
      warning() << "Event " << m_iev
                << ": generic GSF track access failed (unknown exception) — "
                   "writing empty generic fields" << endmsg;
      fillTrack(nullptr,
                m_gsf_pT, m_gsf_p, m_gsf_eta, m_gsf_theta,
                m_gsf_phi, m_gsf_d0, m_gsf_z0,
                m_gsf_omega, m_gsf_tanl,
                m_gsf_chi2, m_gsf_ndf, m_gsf_nhits, m_gsf_type);
    }
  }

  m_bestbranch_gsf_available =
      bestBranchGsfCol && bestBranchGsfCol->size() > 0 ? 1 : 0;
  try {
    fillTrack(bestBranchGsfCol,
              m_bestbranch_gsf_pT, m_bestbranch_gsf_p,
              m_bestbranch_gsf_eta, m_bestbranch_gsf_theta,
              m_bestbranch_gsf_phi, m_bestbranch_gsf_d0,
              m_bestbranch_gsf_z0, m_bestbranch_gsf_omega,
              m_bestbranch_gsf_tanl, m_bestbranch_gsf_chi2,
              m_bestbranch_gsf_ndf, m_bestbranch_gsf_nhits,
              m_bestbranch_gsf_type);
  } catch (const std::exception& e) {
    warning() << "Event " << m_iev << ": BestBranch GSF track access failed — "
              << e.what() << " — writing unavailable BestBranch fields"
              << endmsg;
    m_bestbranch_gsf_available = 0;
    fillTrack(nullptr,
              m_bestbranch_gsf_pT, m_bestbranch_gsf_p,
              m_bestbranch_gsf_eta, m_bestbranch_gsf_theta,
              m_bestbranch_gsf_phi, m_bestbranch_gsf_d0,
              m_bestbranch_gsf_z0, m_bestbranch_gsf_omega,
              m_bestbranch_gsf_tanl, m_bestbranch_gsf_chi2,
              m_bestbranch_gsf_ndf, m_bestbranch_gsf_nhits,
              m_bestbranch_gsf_type);
  } catch (...) {
    warning() << "Event " << m_iev
              << ": BestBranch GSF track access failed (unknown exception) — "
                 "writing unavailable BestBranch fields" << endmsg;
    m_bestbranch_gsf_available = 0;
    fillTrack(nullptr,
              m_bestbranch_gsf_pT, m_bestbranch_gsf_p,
              m_bestbranch_gsf_eta, m_bestbranch_gsf_theta,
              m_bestbranch_gsf_phi, m_bestbranch_gsf_d0,
              m_bestbranch_gsf_z0, m_bestbranch_gsf_omega,
              m_bestbranch_gsf_tanl, m_bestbranch_gsf_chi2,
              m_bestbranch_gsf_ndf, m_bestbranch_gsf_nhits,
              m_bestbranch_gsf_type);
  }

  auto fillHitBranches = [&](
      const edm4hep::TrackCollection* collection, const char* label,
      int& count, std::vector<float>& xs, std::vector<float>& ys,
      std::vector<float>& zs, std::vector<float>& rs,
      std::vector<float>& edeps,
      std::vector<unsigned long long>& cellids) {
    if (!collection || collection->size() == 0) return;
    try {
      const auto& trk = (*collection)[0];
      for (const auto& th : trk.getTrackerHits()) {
        if (!th.isAvailable()) continue;
        auto& pos = th.getPosition();
        xs.push_back((float)pos.x);
        ys.push_back((float)pos.y);
        zs.push_back((float)pos.z);
        rs.push_back((float)std::hypot(pos.x, pos.y));
        edeps.push_back(th.getEDep());
        cellids.push_back(th.getCellID());
      }
    } catch (const std::exception& e) {
      warning() << "Event " << m_iev << ": " << label
                << " hit access failed — "
                << e.what() << " — skipping GSF per-hit data" << endmsg;
    } catch (...) {
      warning() << "Event " << m_iev << ": " << label
                << " hit access failed (unknown exception) — "
                   "skipping GSF per-hit data" << endmsg;
    }
    count = (int)xs.size();
  };
  fillHitBranches(gsfCol, "generic GSF", m_gsf_hit_n,
                  m_gsf_hit_x, m_gsf_hit_y, m_gsf_hit_z, m_gsf_hit_r,
                  m_gsf_hit_edep, m_gsf_hit_cellid);
  fillHitBranches(bestBranchGsfCol, "BestBranch GSF",
                  m_bestbranch_gsf_hit_n,
                  m_bestbranch_gsf_hit_x, m_bestbranch_gsf_hit_y,
                  m_bestbranch_gsf_hit_z, m_bestbranch_gsf_hit_r,
                  m_bestbranch_gsf_hit_edep,
                  m_bestbranch_gsf_hit_cellid);

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
      (m_weighted_gsf_available && m_bestbranch_gsf_available &&
       (m_weighted_gsf_omega != m_bestbranch_gsf_omega ||
        m_weighted_gsf_d0 != m_bestbranch_gsf_d0 ||
        m_weighted_gsf_z0 != m_bestbranch_gsf_z0 ||
        m_weighted_gsf_phi != m_bestbranch_gsf_phi ||
        m_weighted_gsf_tanl != m_bestbranch_gsf_tanl ||
        m_weighted_gsf_chi2 != m_bestbranch_gsf_chi2 ||
        m_weighted_gsf_ndf != m_bestbranch_gsf_ndf)) ? 1 : 0;

  m_fullmixture_gsf_available =
      fullMixtureGsfCol && fullMixtureGsfCol->size() > 0 ? 1 : 0;
  try {
    fillTrack(fullMixtureGsfCol,
              m_fullmixture_gsf_pT, m_fullmixture_gsf_p,
              m_fullmixture_gsf_eta, m_fullmixture_gsf_theta,
              m_fullmixture_gsf_phi, m_fullmixture_gsf_d0,
              m_fullmixture_gsf_z0, m_fullmixture_gsf_omega,
              m_fullmixture_gsf_tanl, m_fullmixture_gsf_chi2,
              m_fullmixture_gsf_ndf, m_fullmixture_gsf_nhits,
              m_fullmixture_gsf_type);
  } catch (const std::exception& e) {
    warning() << "Event " << m_iev
              << ": FullMixtureMode GSF track access failed — " << e.what()
              << " — writing unavailable full-mixture fields" << endmsg;
    m_fullmixture_gsf_available = 0;
    fillTrack(nullptr,
              m_fullmixture_gsf_pT, m_fullmixture_gsf_p,
              m_fullmixture_gsf_eta, m_fullmixture_gsf_theta,
              m_fullmixture_gsf_phi, m_fullmixture_gsf_d0,
              m_fullmixture_gsf_z0, m_fullmixture_gsf_omega,
              m_fullmixture_gsf_tanl, m_fullmixture_gsf_chi2,
              m_fullmixture_gsf_ndf, m_fullmixture_gsf_nhits,
              m_fullmixture_gsf_type);
  } catch (...) {
    warning() << "Event " << m_iev
              << ": FullMixtureMode GSF track access failed (unknown "
                 "exception) — writing unavailable full-mixture fields"
              << endmsg;
    m_fullmixture_gsf_available = 0;
    fillTrack(nullptr,
              m_fullmixture_gsf_pT, m_fullmixture_gsf_p,
              m_fullmixture_gsf_eta, m_fullmixture_gsf_theta,
              m_fullmixture_gsf_phi, m_fullmixture_gsf_d0,
              m_fullmixture_gsf_z0, m_fullmixture_gsf_omega,
              m_fullmixture_gsf_tanl, m_fullmixture_gsf_chi2,
              m_fullmixture_gsf_ndf, m_fullmixture_gsf_nhits,
              m_fullmixture_gsf_type);
  }
  m_fullmixture_gsf_changed =
      (m_fullmixture_gsf_available && m_bestbranch_gsf_available &&
       (m_fullmixture_gsf_omega != m_bestbranch_gsf_omega ||
        m_fullmixture_gsf_d0 != m_bestbranch_gsf_d0 ||
        m_fullmixture_gsf_z0 != m_bestbranch_gsf_z0 ||
        m_fullmixture_gsf_phi != m_bestbranch_gsf_phi ||
        m_fullmixture_gsf_tanl != m_bestbranch_gsf_tanl ||
        m_fullmixture_gsf_chi2 != m_bestbranch_gsf_chi2 ||
        m_fullmixture_gsf_ndf != m_bestbranch_gsf_ndf)) ? 1 : 0;

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
  m_ecal_gsf_changed =
      (m_ecal_gsf_available && m_bestbranch_gsf_available &&
       (m_ecal_gsf_omega != m_bestbranch_gsf_omega ||
        m_ecal_gsf_d0 != m_bestbranch_gsf_d0 ||
        m_ecal_gsf_z0 != m_bestbranch_gsf_z0 ||
        m_ecal_gsf_phi != m_bestbranch_gsf_phi ||
        m_ecal_gsf_tanl != m_bestbranch_gsf_tanl ||
        m_ecal_gsf_chi2 != m_bestbranch_gsf_chi2 ||
        m_ecal_gsf_ndf != m_bestbranch_gsf_ndf)) ? 1 : 0;

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
      (m_mc_pT > 0 && gsfCol && gsfCol->size() > 0)
          ? (m_gsf_pT - m_mc_pT) / m_mc_pT : 0;
  m_res_pT_bestbranch_gsf =
      (m_mc_pT > 0 && m_bestbranch_gsf_available)
          ? (m_bestbranch_gsf_pT - m_mc_pT) / m_mc_pT : 0;
  m_res_pT_weighted_gsf = (m_mc_pT > 0 && m_weighted_gsf_available)
      ? (m_weighted_gsf_pT - m_mc_pT) / m_mc_pT : 0;
  m_res_pT_fullmixture_gsf =
      (m_mc_pT > 0 && m_fullmixture_gsf_available)
          ? (m_fullmixture_gsf_pT - m_mc_pT) / m_mc_pT : 0;
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
