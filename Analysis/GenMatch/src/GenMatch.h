#ifndef GenMatch_h
#define GenMatch_h 1

#include "UTIL/ILDConf.h"
#include "k4FWCore/DataHandle.h"
#include "GaudiKernel/Algorithm.h"
#include <random>
#include "GaudiKernel/NTuple.h"
#include "TFile.h"
#include "TTree.h"

#include "edm4hep/ReconstructedParticleData.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/ReconstructedParticle.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/MCParticleCollectionData.h"

class GenMatch : public Algorithm {
 public:
  // Constructor of this form must be provided
  GenMatch( const std::string& name, ISvcLocator* pSvcLocator );

  // Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

 private:

  DataHandle<edm4hep::ReconstructedParticleCollection> m_PFOColHdl{"CyberPFO", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::MCParticleCollection> m_MCParticleGenHdl{"MCParticle", Gaudi::DataHandle::Reader, this};
  Gaudi::Property<std::string> m_algo{this, "Algorithm", "ee_kt_algorithm"};
  Gaudi::Property<int> m_nJets{this, "nJets", 2};
  Gaudi::Property<double> m_R{this, "R", 0.6};
  Gaudi::Property<std::string> m_outputFile{this, "OutputFile", "GenMatch.root"};

  int _nEvt;
  int _particle_index;
  int _jet_index;
  int _nparticles;
  int jet1_nparticles;
  int jet2_nparticles;
  double _time;

  TFile* _file;
  TTree* _tree;
  double jet1_px, jet1_py, jet1_pz, jet1_E;
  double jet2_px, jet2_py, jet2_pz, jet2_E;
  double jet1_costheta, jet1_phi;
  double jet2_costheta, jet2_phi;
  double jet1_pt, jet2_pt;
  int jet1_nconstituents, jet2_nconstituents, nparticles, jet1_GENMatch_id, jet2_GENMatch_id;
  double constituents_E1tot;
  double constituents_E2tot;
  double ymerge[6];
  double mass;
  double mass_gen_match, jet1_GENMatch_mindR, jet2_GENMatch_mindR;
  double mass_allParticles;
  //double PFO_Energy_muon[50];
  //double PFO_Energy_Charge[50];
  //double PFO_Energy_Neutral[50];
  //double PFO_Energy_Neutral_singleCluster[600];
  //double PFO_Energy_Neutral_singleCluster_R[600];
  int _particle_index_muon, _particle_index_Charge, _particle_index_Neutral, _particle_index_Neutral_singleCluster;
  int particle_index_muon, particle_index_Charge, particle_index_Neutral, particle_index_Neutral_singleCluster;
  //double GEN_PFO_Energy_muon[50];
  //double GEN_PFO_Energy_pipm[120];
  //double GEN_PFO_Energy_pi0[120];
  //double GEN_PFO_Energy_ppm[120];
  //double GEN_PFO_Energy_kpm[120];
  //double GEN_PFO_Energy_kL[120];
  //double GEN_PFO_Energy_n[50];
  //double GEN_PFO_Energy_e[50];
  //double GEN_PFO_Energy_gamma[120];
  std::vector<double> PFO_Energy_muon, PFO_Energy_muon_GENMatch_dR, PFO_Energy_muon_GENMatch_E, PFO_Energy_Charge, PFO_Energy_Charge_Ecal, PFO_Energy_Charge_Hcal, PFO_Energy_Charge_GENMatch_dR, PFO_Energy_Charge_GENMatch_E, PFO_Energy_Neutral, PFO_Energy_Neutral_singleCluster, PFO_Energy_Neutral_singleCluster_R;
  std::vector<int> PFO_Energy_Charge_GENMatch_ID, PFO_Energy_muon_GENMatch_ID;
  std::vector<double> GEN_PFO_Energy_muon, GEN_PFO_Energy_pipm, GEN_PFO_Energy_pi0, GEN_PFO_Energy_ppm, GEN_PFO_Energy_kpm, GEN_PFO_Energy_kL, GEN_PFO_Energy_n, GEN_PFO_Energy_e, GEN_PFO_Energy_gamma;
  std::vector<std::vector<double>> PFO_Hits_Charge_E, PFO_Hits_Charge_R, PFO_Hits_Charge_theta, PFO_Hits_Charge_phi;
  std::vector<std::vector<double>> PFO_Hits_Neutral_E, PFO_Hits_Neutral_R, PFO_Hits_Neutral_theta, PFO_Hits_Neutral_phi;
  //double PFO_Energy_muon[500];

  int _gen_particle_index;
  double barrelRatio;
  double ymerge_gen[6];
  double GEN_mass;
  double GEN_ISR_E, GEN_ISR_pt;
  double GEN_inv_E, GEN_inv_pt;
  double GEN_jet1_px, GEN_jet1_py, GEN_jet1_pz, GEN_jet1_E;
  double GEN_jet2_px, GEN_jet2_py, GEN_jet2_pz, GEN_jet2_E;
  double GEN_jet1_costheta, GEN_jet1_phi;
  double GEN_jet2_costheta, GEN_jet2_phi;
  double GEN_jet1_pt, GEN_jet2_pt;
  int GEN_jet1_nconstituents, GEN_jet2_nconstituents, GEN_nparticles;
  double GEN_constituents_E1tot;
  double GEN_constituents_E2tot;
  int _gen_particle_gamma_index, _gen_particle_pipm_index, _gen_particle_pi0_index, _gen_particle_ppm_index, _gen_particle_kpm_index, _gen_particle_kL_index, _gen_particle_n_index, _gen_particle_e_index;
  int GEN_particle_gamma_index, GEN_particle_pipm_index, GEN_particle_pi0_index, GEN_particle_ppm_index, GEN_particle_kpm_index, GEN_particle_kL_index, GEN_particle_n_index, GEN_particle_e_index;

  void CleanVars();

};
#endif

