#ifndef RecGsfTracking_GsfFlatTuple_h
#define RecGsfTracking_GsfFlatTuple_h

#include "GaudiKernel/Algorithm.h"
#include "k4FWCore/DataHandle.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include "GsfTruthEventData/MaterialIntervalCollection.h"
#include "podio/UserDataCollection.h"
#include "TruthBHLossScopeStatus.h"

#include <cstdint>
#include <vector>

class TFile;
class TTree;

/// Post-processor: reads CompleteTracks, GSFTracks, the optional paired
/// GSFTracksEcalConstrained collection, and MCParticle from the event store and
/// writes a flat ROOT TTree with relevant tracking parameters and per-hit data
/// for downstream analysis.
class RecGsfFlatTuple : public Algorithm {
public:
  RecGsfFlatTuple(const std::string& name, ISvcLocator* svc);
  ~RecGsfFlatTuple() override = default;
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  DataHandle<edm4hep::TrackCollection>      m_inCompleteTracks{
      "CompleteTracks", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::TrackCollection>      m_inGsfTracks{
      "GSFTracks", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::MCParticleCollection> m_inMCParticles{
      "MCParticle", Gaudi::DataHandle::Reader, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_inTruthBHLossStatus{
      "GSFTruthBHLossStatus", Gaudi::DataHandle::Reader, this};
  DataHandle<gsftruth::MaterialIntervalCollection> m_inTruthMaterialIntervals{
      "GSFTruthMaterialIntervals", Gaudi::DataHandle::Reader, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_inTruthMaterialStatus{
      "GSFTruthMaterialRecordStatus", Gaudi::DataHandle::Reader, this};

  Gaudi::Property<std::string> m_outFileName{this, "OutputFile",
      "gsf_tuple.root", "Output ROOT file name"};
  Gaudi::Property<double> m_bField{this, "BField", 3.0, "Magnetic field [T]"};
  Gaudi::Property<std::vector<std::string>> m_hitCollectionNames{
      this, "HitCollectionNames", {}, "Tracker hit collections to dump (all hits)"};

  // ── output file & tree ──
  TFile* m_file = nullptr;
  TTree* m_tree = nullptr;

  // ── branch variables ──
  int    m_iev = 0;
  // truth
  double m_mc_px = 0, m_mc_py = 0, m_mc_pz = 0;
  double m_mc_pT = 0, m_mc_p = 0, m_mc_eta = 0, m_mc_theta = 0, m_mc_phi = 0;
  int    m_mc_pdg = 0;
  double m_mc_vx = 0, m_mc_vy = 0, m_mc_vz = 0;
  // LCIO (CompleteTracks) AtIP
  double m_lcio_omega = 0, m_lcio_d0 = 0, m_lcio_z0 = 0;
  double m_lcio_phi = 0, m_lcio_tanl = 0;
  double m_lcio_pT = 0, m_lcio_p = 0, m_lcio_eta = 0, m_lcio_theta = 0;
  double m_lcio_chi2 = 0;
  int    m_lcio_ndf = 0;
  int    m_lcio_nhits = 0;
  int    m_lcio_type = 0;
  // GSF AtIP
  double m_gsf_omega = 0, m_gsf_d0 = 0, m_gsf_z0 = 0;
  double m_gsf_phi = 0, m_gsf_tanl = 0;
  double m_gsf_pT = 0, m_gsf_p = 0, m_gsf_eta = 0, m_gsf_theta = 0;
  double m_gsf_chi2 = 0;
  int    m_gsf_ndf = 0;
  int    m_gsf_nhits = 0;
  int    m_gsf_type = 0;
  // truth BH-loss oracle scope for CompleteTracks index 0
  int    m_truth_bh_scope_status =
      truthBHLossStatusValue(TruthBHLossScopeStatus::Disabled);
  int    m_truth_bh_scope_valid = 0;
  // passive truth/DD4hep/runtime material record for its configured track
  int    m_truth_material_scope_status =
      truthBHLossStatusValue(TruthBHLossScopeStatus::Disabled);
  int    m_truth_material_scope_valid = 0;
  int    m_truth_material_interval_n = 0;
  std::vector<int> m_truth_material_input_track_index;
  std::vector<int> m_truth_material_output_track_index;
  std::vector<int> m_truth_material_hit_from_index;
  std::vector<int> m_truth_material_hit_to_index;
  std::vector<int> m_truth_material_surface_from_index;
  std::vector<int> m_truth_material_surface_to_index;
  std::vector<unsigned long long> m_truth_material_cell_from;
  std::vector<unsigned long long> m_truth_material_cell_to;
  std::vector<int> m_truth_material_track_id;
  std::vector<int> m_truth_material_first_step;
  std::vector<int> m_truth_material_last_step;
  std::vector<double> m_truth_material_start_hook_fraction;
  std::vector<double> m_truth_material_end_hook_fraction;
  std::vector<double> m_truth_material_start_x;
  std::vector<double> m_truth_material_start_y;
  std::vector<double> m_truth_material_start_z;
  std::vector<double> m_truth_material_end_x;
  std::vector<double> m_truth_material_end_y;
  std::vector<double> m_truth_material_end_z;
  std::vector<int> m_truth_material_step_count;
  std::vector<double> m_truth_material_g4_tx0;
  std::vector<double> m_truth_material_p_before;
  std::vector<double> m_truth_material_ebrem_loss;
  std::vector<double> m_truth_material_retained_fraction;
  std::vector<int> m_truth_material_dd4hep_hook_valid;
  std::vector<int> m_truth_material_dd4hep_hook_layer_count;
  std::vector<double> m_truth_material_dd4hep_hook_tx0;
  std::vector<int> m_truth_material_runtime_mode;
  std::vector<double> m_truth_material_split_threshold;
  std::vector<int> m_truth_material_forward_candidate_count;
  std::vector<int> m_truth_material_forward_valid_count;
  std::vector<int> m_truth_material_forward_above_threshold_count;
  std::vector<double> m_truth_material_forward_weighted_tx0;
  std::vector<double> m_truth_material_forward_min_tx0;
  std::vector<double> m_truth_material_forward_max_tx0;
  std::vector<int> m_truth_material_forward_leading_component_id;
  std::vector<double> m_truth_material_forward_leading_component_weight;
  std::vector<double> m_truth_material_forward_leading_tx0;
  std::vector<int> m_truth_material_reverse_candidate_count;
  std::vector<int> m_truth_material_reverse_valid_count;
  std::vector<int> m_truth_material_reverse_above_threshold_count;
  std::vector<double> m_truth_material_reverse_weighted_tx0;
  std::vector<double> m_truth_material_reverse_min_tx0;
  std::vector<double> m_truth_material_reverse_max_tx0;
  std::vector<int> m_truth_material_reverse_leading_component_id;
  std::vector<double> m_truth_material_reverse_leading_component_weight;
  std::vector<double> m_truth_material_reverse_leading_tx0;
  // paired ECAL-constrained GSF AtIP
  double m_ecal_gsf_omega = 0, m_ecal_gsf_d0 = 0, m_ecal_gsf_z0 = 0;
  double m_ecal_gsf_phi = 0, m_ecal_gsf_tanl = 0;
  double m_ecal_gsf_pT = 0, m_ecal_gsf_p = 0;
  double m_ecal_gsf_eta = 0, m_ecal_gsf_theta = 0;
  double m_ecal_gsf_chi2 = 0;
  int    m_ecal_gsf_ndf = 0;
  int    m_ecal_gsf_nhits = 0;
  int    m_ecal_gsf_type = 0;
  int    m_ecal_gsf_available = 0;
  int    m_ecal_gsf_changed = 0;
  // resolution
  double m_res_pT_gsf = 0;     // (gsf_pT - mc_pT) / mc_pT
  double m_res_pT_ecal_gsf = 0; // (ecal_gsf_pT - mc_pT) / mc_pT
  double m_res_pT_lcio = 0;    // (lcio_pT - mc_pT) / mc_pT

  // ── per-hit data for the LCIO (CompleteTracks) track ──
  int                   m_lcio_hit_n = 0;
  std::vector<float>    m_lcio_hit_x;
  std::vector<float>    m_lcio_hit_y;
  std::vector<float>    m_lcio_hit_z;
  std::vector<float>    m_lcio_hit_r;
  std::vector<float>    m_lcio_hit_edep;
  std::vector<unsigned long long> m_lcio_hit_cellid;

  // ── per-hit data for the GSF track ──
  int                   m_gsf_hit_n = 0;
  std::vector<float>    m_gsf_hit_x;
  std::vector<float>    m_gsf_hit_y;
  std::vector<float>    m_gsf_hit_z;
  std::vector<float>    m_gsf_hit_r;
  std::vector<float>    m_gsf_hit_edep;
  std::vector<unsigned long long> m_gsf_hit_cellid;

  // ── all tracker hits from original collections ──
  int                   m_all_hit_n = 0;
  std::vector<float>    m_all_hit_x;
  std::vector<float>    m_all_hit_y;
  std::vector<float>    m_all_hit_z;
  std::vector<float>    m_all_hit_r;
  std::vector<float>    m_all_hit_edep;
  std::vector<unsigned long long> m_all_hit_cellid;
  std::vector<int>      m_all_hit_det;  // detector tag: 0=first collection, 1=second, ...
};

#endif
