#ifndef RecGsfTracking_GsfSimHitTuple_h
#define RecGsfTracking_GsfSimHitTuple_h

#include "GaudiKernel/Algorithm.h"
#include "k4FWCore/DataHandle.h"
#include "edm4hep/MCParticleCollection.h"

#include <string>
#include <vector>

class TFile;
class TTree;

/// Diagnostic tuple for CEPC electron energy-loss studies.
///
/// This reads EDM4hep SimTrackerHit collections from the event store and
/// writes the hit-level truth momentum, position, path length, EDep, and
/// MCParticle links. SimTrackerHit momentum is the particle momentum at the
/// hit position, not a full Geant4 pre/post-step pair, but it is the truth
/// information available in standard CEPCSW ROOT outputs.
class RecGsfSimHitTuple : public Algorithm {
public:
  RecGsfSimHitTuple(const std::string& name, ISvcLocator* svc);
  ~RecGsfSimHitTuple() override = default;

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  DataHandle<edm4hep::MCParticleCollection> m_inMCParticles{
      "MCParticle", Gaudi::DataHandle::Reader, this};

  Gaudi::Property<std::string> m_outFileName{this, "OutputFile",
      "gsf_simhit_tuple.root", "Output ROOT file name"};
  Gaudi::Property<std::vector<std::string>> m_simHitCollectionNames{
      this, "SimHitCollectionNames", {}, "SimTrackerHit collections to dump"};
  Gaudi::Property<bool> m_primaryOnly{this, "PrimaryOnly", true,
      "Keep only hits linked to MCParticle[0]"};
  Gaudi::Property<bool> m_electronOnly{this, "ElectronOnly", true,
      "Keep only hits linked to electrons/positrons"};

  TFile* m_file = nullptr;
  TTree* m_tree = nullptr;

  int m_iev = 0;

  // Primary MCParticle summary.
  int m_mc_pdg = 0;
  int m_mc_gen_status = 0;
  int m_mc_sim_status = 0;
  double m_mc_vx = 0, m_mc_vy = 0, m_mc_vz = 0;
  double m_mc_ex = 0, m_mc_ey = 0, m_mc_ez = 0;
  double m_mc_px = 0, m_mc_py = 0, m_mc_pz = 0, m_mc_p = 0, m_mc_pT = 0;
  double m_mc_end_px = 0, m_mc_end_py = 0, m_mc_end_pz = 0;
  double m_mc_end_p = 0, m_mc_end_pT = 0;
  double m_mc_retained_p = 0;

  // SimTrackerHit dump. One tree entry per event, vector branches per hit.
  int m_hit_n = 0;
  std::vector<int> m_hit_det;
  std::vector<int> m_hit_col_index;
  std::vector<int> m_hit_mc_index;
  std::vector<int> m_hit_mc_col;
  std::vector<int> m_hit_pdg;
  std::vector<int> m_hit_quality;
  std::vector<unsigned long long> m_hit_cellid;
  std::vector<float> m_hit_x, m_hit_y, m_hit_z, m_hit_r;
  std::vector<float> m_hit_px, m_hit_py, m_hit_pz, m_hit_p, m_hit_pT;
  std::vector<float> m_hit_time;
  std::vector<float> m_hit_edep;
  std::vector<float> m_hit_path_length;
  std::vector<float> m_hit_retained_vs_primary;

  void clearVectors();
};

#endif
