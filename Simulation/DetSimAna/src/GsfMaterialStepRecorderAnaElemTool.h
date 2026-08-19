#ifndef GsfMaterialStepRecorderAnaElemTool_h
#define GsfMaterialStepRecorderAnaElemTool_h

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/SmartIF.h"
#include "DetInterface/IGeomSvc.h"
#include "DetSimInterface/IAnaElemTool.h"

#include <string>
#include <vector>

class TFile;
class TTree;
namespace dd4hep { namespace rec { class MaterialManager; } }

class GsfMaterialStepRecorderAnaElemTool : public extends<AlgTool, IAnaElemTool> {
public:
  using extends::extends;

  StatusCode initialize() override;
  StatusCode finalize() override;

  void BeginOfRunAction(const G4Run*) override;
  void BeginOfEventAction(const G4Event*) override;
  void EndOfEventAction(const G4Event*) override;
  void UserSteppingAction(const G4Step*) override;

private:
  Gaudi::Property<std::string> m_outputFile{this, "OutputFile",
      "gsf_material_steps.root", "Output ROOT file for Geant4 material steps"};
  Gaudi::Property<std::vector<int>> m_pdgs{this, "PDGs", {11, -11},
      "PDG codes to record"};
  Gaudi::Property<bool> m_primaryOnly{this, "PrimaryOnly", false,
      "Record only primary Geant4 tracks (parent ID == 0)"};
  Gaudi::Property<bool> m_trackerOnly{this, "TrackerOnly", true,
      "Record only steps whose pre or post point is inside tracker_region_rmax/zmax when available"};
  Gaudi::Property<double> m_minStepLengthMm{this, "MinStepLengthMm", 0.0,
      "Minimum Geant4 step length to record [mm]"};
  Gaudi::Property<double> m_minAbsLossGeV{this, "MinAbsLossGeV", 0.0,
      "Minimum absolute momentum loss to record [GeV]"};
  Gaudi::Property<bool> m_recordZeroLoss{this, "RecordZeroLoss", true,
      "Record steps even when post momentum is not smaller than pre momentum"};
  Gaudi::Property<bool> m_recordDD4hepSurfaceIntervals{
      this, "RecordDD4hepSurfaceIntervals", false,
      "Write midpoint-to-midpoint DD4hep material intervals reconstructed "
      "from consecutive sensitive-volume traversals"};

  TFile* m_file = nullptr;
  TTree* m_tree = nullptr;
  TTree* m_dd4hepTree = nullptr;

  SmartIF<IGeomSvc> m_geosvc;
  dd4hep::rec::MaterialManager* m_materialManager = nullptr;
  double m_trackerR = 0.0;
  double m_trackerZ = 0.0;

  // ── event-level branches ──
  int m_run_id = -1;
  int m_event_id = -1;
  int m_step_count = 0;

  // ── step-level vectors ──
  std::vector<int>   m_track_id;
  std::vector<int>   m_parent_id;
  std::vector<int>   m_is_primary;
  std::vector<int>   m_track_step_number;
  std::vector<int>   m_recorded_step_index;
  std::vector<int>   m_pdg;
  std::vector<int>   m_charge;
  std::vector<int>   m_step_status_pre;
  std::vector<int>   m_step_status_post;
  std::vector<int>   m_process_subtype;
  std::vector<int>   m_pre_volume_copy_no;
  std::vector<int>   m_post_volume_copy_no;
  std::vector<int>   m_pre_sensitive;
  std::vector<int>   m_post_sensitive;

  std::vector<float>  m_pre_x, m_pre_y, m_pre_z, m_pre_r;
  std::vector<float>  m_post_x, m_post_y, m_post_z, m_post_r;
  std::vector<float>  m_mid_x, m_mid_y, m_mid_z, m_mid_r;
  std::vector<float>  m_pre_px, m_pre_py, m_pre_pz, m_pre_p, m_pre_pT;
  std::vector<float>  m_post_px, m_post_py, m_post_pz, m_post_p, m_post_pT;
  std::vector<float>  m_pre_dir_x, m_pre_dir_y, m_pre_dir_z;
  std::vector<float>  m_post_dir_x, m_post_dir_y, m_post_dir_z;
  std::vector<float>  m_dp, m_loss, m_retained;
  std::vector<float>  m_pre_ekin, m_post_ekin, m_dekin;
  std::vector<float>  m_edep, m_nonion_edep;
  std::vector<float>  m_step_length, m_material_radlen, m_step_tX0;
  std::vector<float>  m_track_length_pre, m_track_length_post;
  std::vector<float>  m_global_time_pre, m_global_time_post;

  std::vector<std::string> m_pre_volume;
  std::vector<std::string> m_post_volume;
  std::vector<std::string> m_pre_touchable_path;
  std::vector<std::string> m_post_touchable_path;
  std::vector<std::string> m_material;
  std::vector<std::string> m_process;

  // ── optional DD4hep midpoint-to-midpoint interval tree ──
  int m_dd4hep_interval_count = 0;
  std::vector<int> m_dd4hep_track_id;
  std::vector<int> m_dd4hep_parent_id;
  std::vector<int> m_dd4hep_is_primary;
  std::vector<int> m_dd4hep_pdg;
  std::vector<int> m_dd4hep_interval_index;
  std::vector<int> m_dd4hep_from_track_step;
  std::vector<int> m_dd4hep_to_track_step;
  std::vector<int> m_dd4hep_segment_count;
  std::vector<int> m_dd4hep_reverse_segment_count;
  std::vector<int> m_dd4hep_valid;
  std::vector<int> m_dd4hep_reverse_valid;
  std::vector<int> m_dd4hep_coverage_repaired;
  std::vector<int> m_dd4hep_reverse_coverage_repaired;
  std::vector<int> m_dd4hep_g4_step_count;
  std::vector<int> m_dd4hep_ebrem_step_count;
  std::vector<float> m_dd4hep_from_x, m_dd4hep_from_y;
  std::vector<float> m_dd4hep_from_z, m_dd4hep_from_r;
  std::vector<float> m_dd4hep_to_x, m_dd4hep_to_y;
  std::vector<float> m_dd4hep_to_z, m_dd4hep_to_r;
  std::vector<float> m_dd4hep_from_track_length_mm;
  std::vector<float> m_dd4hep_to_track_length_mm;
  std::vector<float> m_dd4hep_path_length_mm;
  std::vector<float> m_dd4hep_initial_covered_length_mm;
  std::vector<float> m_dd4hep_reverse_initial_covered_length_mm;
  std::vector<float> m_dd4hep_covered_length_mm;
  std::vector<float> m_dd4hep_reverse_covered_length_mm;
  std::vector<float> m_dd4hep_path_tX0;
  std::vector<float> m_dd4hep_reverse_path_tX0;
  std::vector<float> m_dd4hep_g4_tX0;
  std::vector<float> m_dd4hep_p_before_GeV;
  std::vector<float> m_dd4hep_p_after_GeV;
  std::vector<float> m_dd4hep_ebrem_loss_GeV;
  std::vector<std::string> m_dd4hep_surface_from;
  std::vector<std::string> m_dd4hep_surface_to;
  std::vector<std::string> m_dd4hep_materials;
  std::vector<std::string> m_dd4hep_reverse_materials;

  bool acceptPdg(int pdg) const;
  bool insideTracker(double r, double z) const;
  void bookTree();
  void buildDD4hepSurfaceIntervals();
  void clearVectors();
};

#endif
