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

  TFile* m_file = nullptr;
  TTree* m_tree = nullptr;

  SmartIF<IGeomSvc> m_geosvc;
  double m_trackerR = 0.0;
  double m_trackerZ = 0.0;

  // ── event-level branches ──
  int m_event_id = -1;
  int m_step_count = 0;

  // ── step-level vectors ──
  std::vector<int>   m_track_id;
  std::vector<int>   m_parent_id;
  std::vector<int>   m_pdg;
  std::vector<int>   m_charge;
  std::vector<int>   m_step_status_pre;
  std::vector<int>   m_step_status_post;
  std::vector<int>   m_process_subtype;
  std::vector<int>   m_pre_volume_copy_no;
  std::vector<int>   m_post_volume_copy_no;

  std::vector<float>  m_pre_x, m_pre_y, m_pre_z, m_pre_r;
  std::vector<float>  m_post_x, m_post_y, m_post_z, m_post_r;
  std::vector<float>  m_mid_x, m_mid_y, m_mid_z, m_mid_r;
  std::vector<float>  m_pre_px, m_pre_py, m_pre_pz, m_pre_p, m_pre_pT;
  std::vector<float>  m_post_px, m_post_py, m_post_pz, m_post_p, m_post_pT;
  std::vector<float>  m_dp, m_loss, m_retained;
  std::vector<float>  m_pre_ekin, m_post_ekin, m_dekin;
  std::vector<float>  m_edep, m_nonion_edep;
  std::vector<float>  m_step_length, m_material_radlen, m_step_tX0;
  std::vector<float>  m_global_time_pre, m_global_time_post;

  std::vector<std::string> m_pre_volume;
  std::vector<std::string> m_post_volume;
  std::vector<std::string> m_material;
  std::vector<std::string> m_process;

  bool acceptPdg(int pdg) const;
  bool insideTracker(double r, double z) const;
  void bookTree();
  void clearVectors();
};

#endif
