#include "GenMatch.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartDataPtr.h"

#include "DetInterface/IGeomSvc.h"
#include "DD4hep/Detector.h"
#include "DD4hep/DD4hepUnits.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include <math.h>
#include "fastjet/ClusterSequence.hh"
#include "fastjet/PseudoJet.hh"
//time measurement
#include <chrono>

#include "TLorentzVector.h"

using namespace fastjet;
using namespace std;

DECLARE_COMPONENT( GenMatch )

//------------------------------------------------------------------------------
GenMatch::GenMatch( const string& name, ISvcLocator* pSvcLocator )
    : Algorithm( name, pSvcLocator ) {

  declareProperty("InputPFOs", m_PFOColHdl, "Handle of the Input PFO collection");
  declareProperty("InputGEN", m_MCParticleGenHdl, "Handle of the Input GEN collection");
  //declareProperty("Algorithm", m_algo, "Jet clustering algorithm");
  //declareProperty("nJets", m_nJets, "Number of jets to be clustered");
  //declareProperty("R", m_R, "Jet clustering radius");
  //declareProperty("OutputFile", m_outputFile, "Output file name");

}

//------------------------------------------------------------------------------
StatusCode GenMatch::initialize(){

  _nEvt = 0;

  // Create a new TTree
  _file = TFile::Open(m_outputFile.value().c_str(), "RECREATE");
  _tree = new TTree("jets", "jets");

  _tree->Branch("jet1_px", &jet1_px, "jet1_px/D");
  _tree->Branch("jet1_py", &jet1_py, "jet1_py/D");
  _tree->Branch("jet1_pz", &jet1_pz, "jet1_pz/D");
  _tree->Branch("jet1_E", &jet1_E, "jet1_E/D");
  _tree->Branch("jet1_costheta", &jet1_costheta, "jet1_costheta/D");
  _tree->Branch("jet1_phi", &jet1_phi, "jet1_phi/D");
  _tree->Branch("jet1_pt", &jet1_pt, "jet1_pt/D");
  _tree->Branch("jet1_nconstituents", &jet1_nconstituents, "jet1_nconstituents/I");
  _tree->Branch("jet2_px", &jet2_px, "jet2_px/D");
  _tree->Branch("jet2_py", &jet2_py, "jet2_py/D");
  _tree->Branch("jet2_pz", &jet2_pz, "jet2_pz/D");
  _tree->Branch("jet2_E", &jet2_E, "jet2_E/D");
  _tree->Branch("jet2_costheta", &jet2_costheta, "jet2_costheta/D");
  _tree->Branch("jet2_phi", &jet2_phi, "jet2_phi/D");
  _tree->Branch("jet2_pt", &jet2_pt, "jet2_pt/D");
  _tree->Branch("jet2_nconstituents", &jet2_nconstituents, "jet2_nconstituents/I");
  _tree->Branch("constituents_E1tot", &constituents_E1tot, "constituents_E1tot/D");
  _tree->Branch("constituents_E2tot", &constituents_E2tot, "constituents_E2tot/D");
  _tree->Branch("mass", &mass, "mass/D");
  _tree->Branch("ymerge", ymerge, "ymerge[6]/D");
  _tree->Branch("nparticles", &nparticles, "nparticles/I");
  _tree->Branch("jet1_GENMatch_id", &jet1_GENMatch_id, "jet1_GENMatch_id/I");
  _tree->Branch("jet2_GENMatch_id", &jet2_GENMatch_id, "jet2_GENMatch_id/I");
  _tree->Branch("jet1_GENMatch_mindR", &jet1_GENMatch_mindR, "jet1_GENMatch_mindR/D");
  _tree->Branch("jet2_GENMatch_mindR", &jet2_GENMatch_mindR, "jet2_GENMatch_mindR/D");

  _tree->Branch("PFO_Energy_muon", &PFO_Energy_muon);
  _tree->Branch("PFO_Energy_muon_GENMatch_dR", &PFO_Energy_muon_GENMatch_dR);
  _tree->Branch("PFO_Energy_muon_GENMatch_ID", &PFO_Energy_muon_GENMatch_ID);
  _tree->Branch("PFO_Energy_muon_GENMatch_E", &PFO_Energy_muon_GENMatch_E);
  _tree->Branch("PFO_Energy_Charge", &PFO_Energy_Charge);
  _tree->Branch("PFO_Energy_Charge_Ecal", &PFO_Energy_Charge_Ecal);
  _tree->Branch("PFO_Energy_Charge_Hcal", &PFO_Energy_Charge_Hcal);
  _tree->Branch("PFO_Energy_Charge_GENMatch_dR", &PFO_Energy_Charge_GENMatch_dR);
  _tree->Branch("PFO_Energy_Charge_GENMatch_ID", &PFO_Energy_Charge_GENMatch_ID);
  _tree->Branch("PFO_Energy_Charge_GENMatch_E", &PFO_Energy_Charge_GENMatch_E);
  _tree->Branch("PFO_Hits_Charge_E", &PFO_Hits_Charge_E);
  _tree->Branch("PFO_Hits_Charge_R", &PFO_Hits_Charge_R);
  _tree->Branch("PFO_Hits_Charge_theta", &PFO_Hits_Charge_theta);
  _tree->Branch("PFO_Hits_Charge_phi", &PFO_Hits_Charge_phi);

  _tree->Branch("PFO_Energy_Neutral", &PFO_Energy_Neutral);
  _tree->Branch("PFO_Energy_Neutral_singleCluster", &PFO_Energy_Neutral_singleCluster);
  _tree->Branch("PFO_Energy_Neutral_singleCluster_R", &PFO_Energy_Neutral_singleCluster_R);
  _tree->Branch("PFO_Hits_Neutral_E", &PFO_Hits_Neutral_E);
  _tree->Branch("PFO_Hits_Neutral_R", &PFO_Hits_Neutral_R);
  _tree->Branch("PFO_Hits_Neutral_theta", &PFO_Hits_Neutral_theta);
  _tree->Branch("PFO_Hits_Neutral_phi", &PFO_Hits_Neutral_phi);

  _tree->Branch("GEN_PFO_Energy_muon", &GEN_PFO_Energy_muon);
  _tree->Branch("GEN_PFO_Energy_pipm", &GEN_PFO_Energy_pipm);
  _tree->Branch("GEN_PFO_Energy_pi0", &GEN_PFO_Energy_pi0);
  _tree->Branch("GEN_PFO_Energy_ppm", &GEN_PFO_Energy_ppm);
  _tree->Branch("GEN_PFO_Energy_kpm", &GEN_PFO_Energy_kpm);
  _tree->Branch("GEN_PFO_Energy_kL", &GEN_PFO_Energy_kL);
  _tree->Branch("GEN_PFO_Energy_gamma", &GEN_PFO_Energy_gamma);
  _tree->Branch("GEN_PFO_Energy_n", &GEN_PFO_Energy_n);
  _tree->Branch("GEN_PFO_Energy_e", &GEN_PFO_Energy_e);


  _tree->Branch("particle_index_muon", &particle_index_muon, "particle_index_muon/I");
  _tree->Branch("particle_index_Charge", &particle_index_Charge, "particle_index_Charge/I");
  _tree->Branch("particle_index_Neutral", &particle_index_Neutral, "particle_index_Neutral/I");
  _tree->Branch("particle_index_Neutral_singleCluster", &particle_index_Neutral_singleCluster, "particle_index_Neutral_singleCluster/I");

  _tree->Branch("GEN_particle_gamma_index", &GEN_particle_gamma_index, "GEN_particle_gamma_index/I");
  _tree->Branch("GEN_particle_pipm_index", &GEN_particle_pipm_index, "GEN_particle_pipm_index/I");
  _tree->Branch("GEN_particle_pi0_index", &GEN_particle_pi0_index, "GEN_particle_pi0_index/I");
  _tree->Branch("GEN_particle_ppm_index", &GEN_particle_ppm_index, "GEN_particle_ppm_index/I");
  _tree->Branch("GEN_particle_kpm_index", &GEN_particle_kpm_index, "GEN_particle_kpm_index/I");
  _tree->Branch("GEN_particle_kL_index", &GEN_particle_kL_index, "GEN_particle_kL_index/I");
  _tree->Branch("GEN_particle_n_index", &GEN_particle_n_index, "GEN_particle_n_index/I");
  _tree->Branch("GEN_particle_e_index", &GEN_particle_e_index, "GEN_particle_e_index/I");

  _tree->Branch("ymerge_gen", ymerge_gen, "ymerge_gen[6]/D");

  _tree->Branch("mass_gen_match", &mass_gen_match, "mass_gen_match/D");
  _tree->Branch("GEN_jet1_px", &GEN_jet1_px, "GEN_jet1_px/D");
  _tree->Branch("GEN_jet1_py", &GEN_jet1_py, "GEN_jet1_py/D");
  _tree->Branch("GEN_jet1_pz", &GEN_jet1_pz, "GEN_jet1_pz/D");
  _tree->Branch("GEN_jet1_E", &GEN_jet1_E, "GEN_jet1_E/D");
  _tree->Branch("GEN_jet1_costheta", &GEN_jet1_costheta, "GEN_jet1_costheta/D");
  _tree->Branch("GEN_jet1_phi", &GEN_jet1_phi, "GEN_jet1_phi/D");
  _tree->Branch("GEN_jet1_pt", &GEN_jet1_pt, "GEN_jet1_pt/D");
  _tree->Branch("GEN_jet1_nconstituents", &GEN_jet1_nconstituents, "GEN_jet1_nconstituents/I");
  _tree->Branch("GEN_jet2_px", &GEN_jet2_px, "GEN_jet2_px/D");
  _tree->Branch("GEN_jet2_py", &GEN_jet2_py, "GEN_jet2_py/D");
  _tree->Branch("GEN_jet2_pz", &GEN_jet2_pz, "GEN_jet2_pz/D");
  _tree->Branch("GEN_jet2_E", &GEN_jet2_E, "GEN_jet2_E/D");
  _tree->Branch("GEN_jet2_costheta", &GEN_jet2_costheta, "GEN_jet2_costheta/D");
  _tree->Branch("GEN_jet2_phi", &GEN_jet2_phi, "GEN_jet2_phi/D");
  _tree->Branch("GEN_jet2_pt", &GEN_jet2_pt, "GEN_jet2_pt/D");
  _tree->Branch("GEN_jet2_nconstituents", &GEN_jet2_nconstituents, "GEN_jet2_nconstituents/I");
  _tree->Branch("GEN_ISR_E", &GEN_ISR_E, "GEN_ISR_E/D");
  _tree->Branch("GEN_ISR_pt", &GEN_ISR_pt, "GEN_ISR_pt/D");
  _tree->Branch("GEN_inv_E", &GEN_inv_E, "GEN_inv_E/D");
  _tree->Branch("GEN_inv_pt", &GEN_inv_pt, "GEN_inv_pt/D");
  _tree->Branch("GEN_constituents_E1tot", &GEN_constituents_E1tot, "GEN_constituents_E1tot/D");
  _tree->Branch("GEN_constituents_E2tot", &GEN_constituents_E2tot, "GEN_constituents_E2tot/D");
  _tree->Branch("GEN_mass", &GEN_mass, "GEN_mass/D");
  _tree->Branch("barrelRatio", &barrelRatio, "barrelRatio/D");
  _tree->Branch("mass_allParticles", &mass_allParticles, "mass_allParticles/D");
  _tree->Branch("GEN_nparticles", &GEN_nparticles, "GEN_nparticles/I");
  
  return StatusCode::SUCCESS;

}

//------------------------------------------------------------------------------
StatusCode GenMatch::execute(){
  
  const edm4hep::ReconstructedParticleCollection* PFO = nullptr;
  try {
    PFO = m_PFOColHdl.get();
  }
  catch ( GaudiException &e ){
    info() << "Collection " << m_PFOColHdl.fullKey() << " is unavailable in event " << _nEvt << endmsg;
    return StatusCode::SUCCESS;
  }
  // Check if the collection is empty
  if ( PFO->size() == 0 ) {
    info() << "Collection " << m_PFOColHdl.fullKey() << " is empty in event " << _nEvt << endmsg;
    return StatusCode::SUCCESS;
  }

  const edm4hep::MCParticleCollection* MCParticlesGen = nullptr;
  try {
    MCParticlesGen = m_MCParticleGenHdl.get();
  }
  catch ( GaudiException &e ){
    info() << "Collection " << m_MCParticleGenHdl.fullKey() << " is unavailable in event " << _nEvt << endmsg;
    return StatusCode::SUCCESS;
  }
  // Check if the collection is empty
  if ( MCParticlesGen->size() == 0 ) {
    info() << "Collection " << m_MCParticleGenHdl.fullKey() << " is empty in event " << _nEvt << endmsg;
    return StatusCode::SUCCESS;
  }
  //initial indces
  CleanVars();
  
  //resize particles. there are many particles are empty, maybe upstream bugs
  vector<PseudoJet> input_particles;
  TLorentzVector input_particles_sum;
  for(const auto& pfo : *PFO){
    if ( isnan(pfo.getMomentum()[0]) || isnan(pfo.getMomentum()[1]) || isnan(pfo.getMomentum()[2]) || pfo.getEnergy() == 0) {
      info() << "Particle " << _particle_index << " px: " << pfo.getMomentum()[0] << " py: " << pfo.getMomentum()[1] << " pz: " << pfo.getMomentum()[2] << " E: " << pfo.getEnergy()  << endmsg;
      _particle_index++;
      continue;
    }
    else{
      input_particles.push_back(PseudoJet(pfo.getMomentum()[0], pfo.getMomentum()[1], pfo.getMomentum()[2], pfo.getEnergy()));
      _particle_index++;
      _nparticles++;
      TLorentzVector p4_pfo;
      p4_pfo.SetPxPyPzE(pfo.getMomentum()[0], pfo.getMomentum()[1], pfo.getMomentum()[2], pfo.getEnergy());
      input_particles_sum += p4_pfo;
      //std::cout << "Particle " << _particle_index << " px: "<<pfo.getMomentum()[0]<<" py: "<<pfo.getMomentum()[1]<<" pz: "<<pfo.getMomentum()[2]<<" E: "<<pfo.getEnergy() << ", mass "<< pfo.getMass() << std::endl;

      if (abs(pfo.getCharge()) > 0 && pfo.clusters_size() == 0){
        //PFO_Energy_muon[_particle_index_muon] = pfo.getEnergy();
        //_particle_index_muon++;
        PFO_Energy_muon.push_back(pfo.getEnergy());

        float mini_dR = 99.0;
        float dR_temp = 99.0; 
        float deta = 99.0; 
        float dphi = 99.0;
        int PFO_Energy_muon_GENMatch_ID_temp = -999;
        float PFO_Energy_muon_GENMatch_E_temp = -999;
        for(const auto& Gen : *MCParticlesGen){
          if (Gen.getGeneratorStatus() != 1) continue;
          if ( abs(Gen.getPDG())==12 || abs(Gen.getPDG())==14 || abs(Gen.getPDG())==16 ) continue;
          TLorentzVector p4_gen;
          p4_gen.SetPxPyPzE(Gen.getMomentum()[0], Gen.getMomentum()[1], Gen.getMomentum()[2], Gen.getEnergy());
          deta = p4_pfo.Eta() - p4_gen.Eta();
          dphi = TVector2::Phi_mpi_pi(p4_pfo.Phi() - p4_gen.Phi());
          dR_temp = TMath::Sqrt(deta*deta + dphi*dphi);
          if (dR_temp < mini_dR){
            mini_dR = dR_temp;
            PFO_Energy_muon_GENMatch_ID_temp = Gen.getPDG();
            PFO_Energy_muon_GENMatch_E_temp = Gen.getEnergy();
          }
        }
        PFO_Energy_muon_GENMatch_dR.push_back(mini_dR);
        PFO_Energy_muon_GENMatch_ID.push_back(PFO_Energy_muon_GENMatch_ID_temp);
        PFO_Energy_muon_GENMatch_E.push_back(PFO_Energy_muon_GENMatch_E_temp);

      } 
      else if (abs(pfo.getCharge()) > 0 && pfo.clusters_size() > 0){
        //PFO_Energy_Charge[_particle_index_Charge] = pfo.getEnergy();
        //_particle_index_Charge++;
        PFO_Energy_Charge.push_back(pfo.getEnergy());

        float PFO_Energy_Charge_Ecal_temp = 0;
        float PFO_Energy_Charge_Hcal_temp = 0;
        int PFO_Energy_Charge_GENMatch_ID_temp = -999;
        float PFO_Energy_Charge_GENMatch_E_temp = -999;
        for (unsigned i = 0; i < pfo.clusters_size(); i++){
          if (TMath::Sqrt(pfo.getClusters(i).getPosition().x * pfo.getClusters(i).getPosition().x + pfo.getClusters(i).getPosition().y * pfo.getClusters(i).getPosition().y) < 2130){
            PFO_Energy_Charge_Ecal_temp += pfo.getClusters(i).getEnergy();
          }else{
            PFO_Energy_Charge_Hcal_temp += pfo.getClusters(i).getEnergy();
          }
        }

        PFO_Energy_Charge_Ecal.push_back(PFO_Energy_Charge_Ecal_temp);
        PFO_Energy_Charge_Hcal.push_back(PFO_Energy_Charge_Hcal_temp);

        float mini_dR = 99.0;
        float dR_temp = 99.0; 
        float deta = 99.0; 
        float dphi = 99.0;
        for(const auto& Gen : *MCParticlesGen){
          if (Gen.getGeneratorStatus() != 1) continue;
          if ( abs(Gen.getPDG())==12 || abs(Gen.getPDG())==14 || abs(Gen.getPDG())==16 ) continue;
          TLorentzVector p4_gen;
          p4_gen.SetPxPyPzE(Gen.getMomentum()[0], Gen.getMomentum()[1], Gen.getMomentum()[2], Gen.getEnergy());
          deta = p4_pfo.Eta() - p4_gen.Eta();
          dphi = TVector2::Phi_mpi_pi(p4_pfo.Phi() - p4_gen.Phi());
          dR_temp = TMath::Sqrt(deta*deta + dphi*dphi);
          if (dR_temp < mini_dR){
            mini_dR = dR_temp;
            PFO_Energy_Charge_GENMatch_ID_temp = Gen.getPDG();
            PFO_Energy_Charge_GENMatch_E_temp = Gen.getEnergy();
          }
        }
        PFO_Energy_Charge_GENMatch_dR.push_back(mini_dR);
        PFO_Energy_Charge_GENMatch_ID.push_back(PFO_Energy_Charge_GENMatch_ID_temp);
        PFO_Energy_Charge_GENMatch_E.push_back(PFO_Energy_Charge_GENMatch_E_temp);

        
        for (unsigned i = 0; i < pfo.clusters_size(); i++){
          std::vector<double> PFO_Hits_Charge_perCluster_E;
          std::vector<double> PFO_Hits_Charge_perCluster_R;
          std::vector<double> PFO_Hits_Charge_perCluster_theta;
          std::vector<double> PFO_Hits_Charge_perCluster_phi;
          for (unsigned j = 0; j < pfo.getClusters(i).hits_size(); j++){
            PFO_Hits_Charge_perCluster_E.push_back(pfo.getClusters(i).getHits(j).getEnergy());
            PFO_Hits_Charge_perCluster_R.push_back(TMath::Sqrt(pfo.getClusters(i).getHits(j).getPosition().x*pfo.getClusters(i).getHits(j).getPosition().x + pfo.getClusters(i).getHits(j).getPosition().y*pfo.getClusters(i).getHits(j).getPosition().y));
            PFO_Hits_Charge_perCluster_theta.push_back(pfo.getClusters(i).getHits(j).getPosition().z/std::asin(TMath::Sqrt(pfo.getClusters(i).getHits(j).getPosition().x*pfo.getClusters(i).getHits(j).getPosition().x + pfo.getClusters(i).getHits(j).getPosition().y*pfo.getClusters(i).getHits(j).getPosition().y + pfo.getClusters(i).getHits(j).getPosition().z*pfo.getClusters(i).getHits(j).getPosition().z)));
            PFO_Hits_Charge_perCluster_phi.push_back(std::atan2(pfo.getClusters(i).getHits(j).getPosition().y, pfo.getClusters(i).getHits(j).getPosition().x));
          }
          //info() << "[Clusters Hits] " << PFO_Hits_Charge_perCluster_E << endmsg;
          //std::copy(PFO_Hits_Charge_perCluster_E.begin(), PFO_Hits_Charge_perCluster_E.end(), std::ostream_iterator<double>(std::cout, " "));
          //std::cout << "[Clusters Hits] " << std::endl;
          PFO_Hits_Charge_E.push_back(PFO_Hits_Charge_perCluster_E);
          PFO_Hits_Charge_R.push_back(PFO_Hits_Charge_perCluster_R);
          PFO_Hits_Charge_theta.push_back(PFO_Hits_Charge_perCluster_theta);
          PFO_Hits_Charge_phi.push_back(PFO_Hits_Charge_perCluster_phi);
        }


      }
      else if (abs(pfo.getCharge()) == 0 && pfo.clusters_size() > 1){
        //PFO_Energy_Neutral[_particle_index_Neutral] += pfo.getEnergy();
        //_particle_index_Neutral++;
        PFO_Energy_Neutral.push_back(pfo.getEnergy());
      }
      else if (abs(pfo.getCharge()) == 0 && pfo.clusters_size() == 1){
        //PFO_Energy_Neutral_singleCluster[_particle_index_Neutral_singleCluster] = pfo.getEnergy();
        //PFO_Energy_Neutral_singleCluster_R[_particle_index_Neutral_singleCluster] = TMath::Sqrt(pfo.getClusters(0).getPosition().x * pfo.getClusters(0).getPosition().x + pfo.getClusters(0).getPosition().y * pfo.getClusters(0).getPosition().y + pfo.getClusters(0).getPosition().z * pfo.getClusters(0).getPosition().z);
        //_particle_index_Neutral_singleCluster++;
        PFO_Energy_Neutral_singleCluster.push_back(pfo.getEnergy());
        PFO_Energy_Neutral_singleCluster_R.push_back(TMath::Sqrt(pfo.getClusters(0).getPosition().x * pfo.getClusters(0).getPosition().x + pfo.getClusters(0).getPosition().y * pfo.getClusters(0).getPosition().y));

        for (unsigned i = 0; i < pfo.clusters_size(); i++){
          std::vector<double> PFO_Hits_Neutral_perCluster_E;
          std::vector<double> PFO_Hits_Neutral_perCluster_R;
          std::vector<double> PFO_Hits_Neutral_perCluster_theta;
          std::vector<double> PFO_Hits_Neutral_perCluster_phi;
          for (unsigned j = 0; j < pfo.getClusters(i).hits_size(); j++){
            PFO_Hits_Neutral_perCluster_E.push_back(pfo.getClusters(i).getHits(j).getEnergy());
            PFO_Hits_Neutral_perCluster_R.push_back(TMath::Sqrt(pfo.getClusters(i).getHits(j).getPosition().x*pfo.getClusters(i).getHits(j).getPosition().x + pfo.getClusters(i).getHits(j).getPosition().y*pfo.getClusters(i).getHits(j).getPosition().y));
            PFO_Hits_Neutral_perCluster_theta.push_back(pfo.getClusters(i).getHits(j).getPosition().z/std::asin(TMath::Sqrt(pfo.getClusters(i).getHits(j).getPosition().x*pfo.getClusters(i).getHits(j).getPosition().x + pfo.getClusters(i).getHits(j).getPosition().y*pfo.getClusters(i).getHits(j).getPosition().y + pfo.getClusters(i).getHits(j).getPosition().z*pfo.getClusters(i).getHits(j).getPosition().z)));
            PFO_Hits_Neutral_perCluster_phi.push_back(std::atan2(pfo.getClusters(i).getHits(j).getPosition().y, pfo.getClusters(i).getHits(j).getPosition().x));
          }
          PFO_Hits_Neutral_E.push_back(PFO_Hits_Neutral_perCluster_E);
          PFO_Hits_Neutral_R.push_back(PFO_Hits_Neutral_perCluster_R);
          PFO_Hits_Neutral_theta.push_back(PFO_Hits_Neutral_perCluster_theta);
          PFO_Hits_Neutral_phi.push_back(PFO_Hits_Neutral_perCluster_phi);
        }
      }
    }
  }

  particle_index_muon = _particle_index_muon;
  particle_index_Charge = _particle_index_Charge;
  particle_index_Neutral = _particle_index_Neutral;
  particle_index_Neutral_singleCluster = _particle_index_Neutral_singleCluster;

  // create a jet definition
  int nJets = m_nJets;
  double R = m_R;

  JetDefinition jet_def(ee_kt_algorithm);
  //JetDefinition jet_def(antikt_algorithm, R);

  // run the clustering, extract the jets
  //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

  ClusterSequence clust_seq(input_particles, jet_def);
  
  vector<PseudoJet> jets = sorted_by_pt(clust_seq.exclusive_jets(nJets));
  
  //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  
  //_time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
  
  //info() << "FastJet clustering done in seconds: " << _time << endmsg;
  
  //string jet_def_str = jet_def.description();
  //debug() << "Clustering with " << jet_def_str << " for " << nJets << " jets with R = " << R << endmsg;
  
  // print the jets
  TLorentzVector Particles_Gen_match_sum;
  //info() << "                     px           py           pz           E           costheta  phi " << endmsg;
  for (unsigned i = 0; i < jets.size(); i++) {
    //info() << "   jet " << i << "             " << jets[i].px() << " " << jets[i].py() << " " << jets[i].pz() << " " << jets[i].E() << " " << jets[i].cos_theta() << " " << jets[i].phi() << endmsg;
    vector<PseudoJet> constituents = jets[i].constituents();

    for (unsigned j = 0; j < constituents.size(); j++) {
      if ( i == 1 ){
        constituents_E1tot += constituents[j].E();
        jet1_nparticles++;
        }
      if ( i == 2 ) {
        constituents_E2tot += constituents[j].E();
        jet2_nparticles++;
        }
      //info() << "    constituent  " << j << " " << constituents[j].px() << " " << constituents[j].py() << " " << constituents[j].pz() << " " << constituents[j].E() << " " << constituents[j].cos_theta() << " " << constituents[j].phi() << endmsg;

      TLorentzVector p4_pfo;
      p4_pfo.SetPxPyPzE(constituents[j].px(), constituents[j].py(), constituents[j].pz(), constituents[j].E());

      float mini_dR = 0.3;
      float dR_temp = 0.0; 
      float deta = 0.0; 
      float dphi =0.0;
      TLorentzVector p4_gen_match;
      for(const auto& Gen : *MCParticlesGen){
        if (Gen.getGeneratorStatus() != 1) continue;
        if ( abs(Gen.getPDG())==12 || abs(Gen.getPDG())==14 || abs(Gen.getPDG())==16 ) continue;
        TLorentzVector p4_gen;
        p4_gen.SetPxPyPzE(Gen.getMomentum()[0], Gen.getMomentum()[1], Gen.getMomentum()[2], Gen.getEnergy());
        deta = p4_pfo.Eta() - p4_gen.Eta();
        dphi = TVector2::Phi_mpi_pi(p4_pfo.Phi() - p4_gen.Phi());
        dR_temp = TMath::Sqrt(deta*deta + dphi*dphi);
        if (dR_temp < mini_dR){
          mini_dR = dR_temp;
          p4_gen_match = p4_gen;
        }
      }
      Particles_Gen_match_sum += p4_gen_match;
    }
  }
  //info() << "Total energy of constituents: " << constituents_E1tot << " " << constituents_E2tot << endmsg;


  double _ymin[20];
  for(int i=1; i<6;i++){
    _ymin[i-1] = clust_seq.exclusive_ymerge (i);
   // info() << " -log10(y" << i << i+1 << ") = " << -log10(_ymin[i-1]) << endmsg;
  }
  

  mass = (jets[0] + jets[1]).m();


  vector<PseudoJet> input_particles_gen;
  int NNeutrinoCount = 0;
  for(const auto& Gen : *MCParticlesGen){
    //info() << "[[DEBUG]] Gen Particle stable 1 " << _gen_particle_index << " px: " << Gen.getMomentum()[0] << " py: " << Gen.getMomentum()[1] << " pz: " << Gen.getMomentum()[2] << " E: " << Gen.getEnergy() << " PDG ID: " << Gen.getPDG() << " GeneratorStatus: " << Gen.getGeneratorStatus() << endmsg;
    if ( (abs(Gen.getPDG())==12 || abs(Gen.getPDG())==14 || abs(Gen.getPDG())==16) && Gen.getGeneratorStatus()!=2 ){ 
      NNeutrinoCount++;
//cout<<"  Found Neutrino #"<<NNeutrinoCount<<endl;
      if(NNeutrinoCount > 2) {
        auto tmpP0 = Gen.getMomentum();
        TVector3 tmpP = TVector3(tmpP0.x, tmpP0.y, tmpP0.z);
        GEN_inv_E += tmpP.Mag();
        GEN_inv_pt += tmpP.Perp();
//cout<<"    Inv energy "<<GEN_inv_E<<", pt "<<GEN_inv_pt<<endl; 
      }      
      _gen_particle_index++;
      continue;
    }
    if (Gen.getGeneratorStatus() != 1) continue;
    //info() << "[[DEBUG]] Gen Particle stable 1 " << _gen_particle_index << " px: " << Gen.getMomentum()[0] << " py: " << Gen.getMomentum()[1] << " pz: " << Gen.getMomentum()[2] << " E: " << Gen.getEnergy() << " PDG ID: " << Gen.getPDG() << " GeneratorStatus: " << Gen.getGeneratorStatus() << endmsg;
    input_particles_gen.push_back(PseudoJet(Gen.getMomentum()[0], Gen.getMomentum()[1], Gen.getMomentum()[2], Gen.getEnergy()));
    if (Gen.getPDG()==22){
      //GEN_PFO_Energy_gamma[_gen_particle_gamma_index] = Gen.getEnergy();
      //_gen_particle_gamma_index++;
      GEN_PFO_Energy_gamma.push_back(Gen.getEnergy());
      int NParent = Gen.parents_size();
//cout<<"  Found photon. index "<<_gen_particle_index<<", Nparent = "<<NParent<<endl;
      if(_gen_particle_index<4 && NParent==0){
        auto tmpP0 = Gen.getMomentum();
        TVector3 tmpP = TVector3(tmpP0.x, tmpP0.y, tmpP0.z);        
        GEN_ISR_E += tmpP.Mag();
        GEN_ISR_pt += tmpP.Perp();
//cout<<"  Found ISR photon. E = "<<GEN_ISR_E<<", pt = "<<GEN_ISR_pt<<endl;
      }
    }
    else if (Gen.getPDG()==211 || Gen.getPDG()==-211){
      //GEN_PFO_Energy_pipm[_gen_particle_pipm_index] = Gen.getEnergy();
      //_gen_particle_pipm_index++;
      GEN_PFO_Energy_pipm.push_back(Gen.getEnergy());
    }
    else if (Gen.getPDG()==111){
      //GEN_PFO_Energy_pi0[_gen_particle_pi0_index] = Gen.getEnergy();
      //_gen_particle_pi0_index++;
      GEN_PFO_Energy_pi0.push_back(Gen.getEnergy());
    }
    else if (Gen.getPDG()==2212 || Gen.getPDG()==-2212){
      //GEN_PFO_Energy_ppm[_gen_particle_ppm_index] = Gen.getEnergy();
      //_gen_particle_ppm_index++;
      GEN_PFO_Energy_ppm.push_back(Gen.getEnergy());
    }
    else if (Gen.getPDG()==321 || Gen.getPDG()==-321){
      //GEN_PFO_Energy_kpm[_gen_particle_kpm_index] = Gen.getEnergy();
      //_gen_particle_kpm_index++;
      GEN_PFO_Energy_kpm.push_back(Gen.getEnergy());
    }
    else if (Gen.getPDG()==130){
      //GEN_PFO_Energy_kL[_gen_particle_kL_index] = Gen.getEnergy();
      //_gen_particle_kL_index++;
      GEN_PFO_Energy_kL.push_back(Gen.getEnergy());
    }
    
    else if (abs(Gen.getPDG())==2112){
      //GEN_PFO_Energy_n[_gen_particle_n_index] = Gen.getEnergy();
      //_gen_particle_n_index++;
      GEN_PFO_Energy_n.push_back(Gen.getEnergy());
    }
    
    else if (abs(Gen.getPDG())==11){
      //GEN_PFO_Energy_e[_gen_particle_e_index] = Gen.getEnergy();
      //_gen_particle_e_index++;
      GEN_PFO_Energy_e.push_back(Gen.getEnergy());
    }
    else if (abs(Gen.getPDG())==13){
      //GEN_PFO_Energy_e[_gen_particle_e_index] = Gen.getEnergy();
      //_gen_particle_e_index++;
      GEN_PFO_Energy_muon.push_back(Gen.getEnergy());
    }
    
    _gen_particle_index++;
  }

  int Nmc = 0;
  int Nmc_barrel = 0;
  int n_status1 = 0;
  for(const auto& Gen : *MCParticlesGen){
    if (Gen.getGeneratorStatus() != 1) continue;
    n_status1++;

    TVector3 part(Gen.getMomentum().x, Gen.getMomentum().y, Gen.getMomentum().z);

    if(n_status1<=4)
    {
      // ISR photon should not hit ECAL barrel
      if(Gen.getPDG()==22 && Gen.getEnergy()>0 && fabs(part.CosTheta())<0.85) Nmc_barrel = 0;
      continue;
    }
    Nmc++;
    if(fabs(part.CosTheta())<0.85) Nmc_barrel++;
  }

  barrelRatio = (double)Nmc_barrel/(double)Nmc;

  GEN_particle_gamma_index = _gen_particle_gamma_index;
  GEN_particle_pipm_index = _gen_particle_pipm_index;
  GEN_particle_pi0_index = _gen_particle_pi0_index;
  GEN_particle_ppm_index = _gen_particle_ppm_index;
  GEN_particle_kpm_index = _gen_particle_kpm_index;
  GEN_particle_kL_index = _gen_particle_kL_index;
  GEN_particle_n_index = _gen_particle_n_index;
  GEN_particle_e_index = _gen_particle_e_index;

  //info() << "[[DEBUG]] Gen Particle stable 1 " << _gen_particle_index << " seperate: " << GEN_particle_gamma_index+GEN_particle_pipm_index+GEN_particle_pi0_index+GEN_particle_ppm_index+GEN_particle_kpm_index+GEN_particle_kL_index+GEN_particle_n_index << endmsg;

  // create a jet definition

  JetDefinition Genjet_def(ee_kt_algorithm);
  //JetDefinition jet_def(antikt_algorithm, R);

  // run the clustering, extract the jets
  //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

  ClusterSequence Genclust_seq(input_particles_gen, Genjet_def);
  
  vector<PseudoJet> Genjets = sorted_by_pt(Genclust_seq.exclusive_jets(nJets));
  
  //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  double _ymin_gen[20];
  for(int i=1; i<6;i++){
    _ymin_gen[i-1] = Genclust_seq.exclusive_ymerge (i);
    //info() << " -log10(y" << i << i+1 << ") = " << -log10(_ymin_gen[i-1]) << endmsg;
  }

  // ############## Jet Gen Match
  for (unsigned i = 0; i < jets.size(); i++) {
    float GEN_mini_dR = 999.;
    int GEN_mini_ID_temp = -99.0;
    float GEN_dR_temp = -99.0; 
    float GEN_deta = 0.0; 
    float GEN_dphi =0.0;

    for (unsigned j = 0; j < Genjets.size(); j++) {
      GEN_deta = jets[i].eta() - Genjets[j].eta();
      GEN_dphi = TVector2::Phi_mpi_pi(jets[i].phi() - Genjets[j].phi());
      GEN_dR_temp = TMath::Sqrt(GEN_deta*GEN_deta + GEN_dphi*GEN_dphi);
      
      if (GEN_dR_temp < GEN_mini_dR){
        GEN_mini_dR = GEN_dR_temp;
        GEN_mini_ID_temp = j;
      }
    }
    if (i==0) {
      jet1_GENMatch_id = GEN_mini_ID_temp;
      jet1_GENMatch_mindR = GEN_mini_dR;
      }
    if (i==1) {
      jet2_GENMatch_id = GEN_mini_ID_temp;
      jet2_GENMatch_mindR = GEN_mini_dR;
      }
  }


  
  //for (unsigned i = 0; i < Genjets.size(); i++) {
  //  vector<PseudoJet> constituents = Genjets[i].constituents();
  //  for (unsigned j = 0; j < constituents.size(); j++) {

  //    for(const auto& Gen : *MCParticlesGen){
  //      if (abs(constituents[j].px()-Gen.getMomentum()[0])<0.00001 && abs(constituents[j].py()-Gen.getMomentum()[1])<0.00001 && abs(constituents[j].pz()-Gen.getMomentum()[2])<0.00001 && abs(constituents[j].E()-Gen.getEnergy()[0])<0.00001 )
  //       if (i==0) {
  //        jet1_GENMatch_id = GEN_mini_ID_temp;
  //        jet1_GENMatch_mindR = GEN_mini_dR;
  //        }
  //       if (i==1) {
  //        jet2_GENMatch_id = GEN_mini_ID_temp;
  //        jet2_GENMatch_mindR = GEN_mini_dR;
  //        }
  //    }
  //    
  //  }
  //}
    
  



  // fill the tree
  jet1_px = jets[0].px();
  jet1_py = jets[0].py();
  jet1_pz = jets[0].pz();
  jet1_E = jets[0].E();
  jet1_costheta = jets[0].cos_theta();
  jet1_phi = jets[0].phi();
  jet1_pt = jets[0].pt();
  jet1_nconstituents = jets[0].constituents().size();
  jet2_px = jets[1].px();
  jet2_py = jets[1].py();
  jet2_pz = jets[1].pz();
  jet2_E = jets[1].E();
  jet2_costheta = jets[1].cos_theta();
  jet2_phi = jets[1].phi();
  jet2_pt = jets[1].pt();
  jet2_nconstituents = jets[1].constituents().size();
  for(int i=0; i<6; i++){
    ymerge[i] = _ymin[i];
  }
  for(int i=0; i<6; i++){
    ymerge_gen[i] = _ymin_gen[i];
  }

  mass_gen_match = Particles_Gen_match_sum.M();
  mass_allParticles = input_particles_sum.M();
  nparticles = _nparticles;
  GEN_nparticles = _gen_particle_index;

  GEN_mass = (Genjets[0] + Genjets[1]).m();
  GEN_jet1_px = Genjets[0].px();
  GEN_jet1_py = Genjets[0].py();
  GEN_jet1_pz = Genjets[0].pz();
  GEN_jet1_E = Genjets[0].E();
  GEN_jet1_costheta = Genjets[0].cos_theta();
  GEN_jet1_phi = Genjets[0].phi();
  GEN_jet1_pt = Genjets[0].pt();
  GEN_jet1_nconstituents = Genjets[0].constituents().size();
  GEN_jet2_px = Genjets[1].px();
  GEN_jet2_py = Genjets[1].py();
  GEN_jet2_pz = Genjets[1].pz();
  GEN_jet2_E = Genjets[1].E();
  GEN_jet2_costheta = Genjets[1].cos_theta();
  GEN_jet2_phi = Genjets[1].phi();
  GEN_jet2_pt = Genjets[1].pt();
  GEN_jet2_nconstituents = Genjets[1].constituents().size();

  _tree->Fill();

  _nEvt++;
  return StatusCode::SUCCESS;

}

//------------------------------------------------------------------------------
StatusCode GenMatch::finalize(){
  debug() << "Finalizing..." << endmsg;
  _file->Write();

  delete _tree;
  return StatusCode::SUCCESS;
}


void GenMatch::CleanVars(){

  _particle_index = 0;
  _gen_particle_index = _particle_index_muon = _particle_index_Charge = _particle_index_Neutral = _particle_index_Neutral_singleCluster = 0;
  particle_index_muon = particle_index_Charge = particle_index_Neutral = particle_index_Neutral_singleCluster = 0;
  _jet_index = 0;
  _nparticles = 0;
  _time = 0;

  jet1_px = jet1_py = jet1_pz = jet1_E = 0;
  jet2_px = jet2_py = jet2_pz = jet2_E = 0;
  jet1_costheta = jet1_phi = 0;
  jet2_costheta = jet2_phi = 0;
  jet1_pt = jet2_pt = jet1_GENMatch_mindR = jet2_GENMatch_mindR = 0;
  jet1_nconstituents = jet2_nconstituents = nparticles = jet1_GENMatch_id = jet2_GENMatch_id = 0;
  constituents_E1tot = constituents_E2tot = 0;
  _gen_particle_gamma_index = _gen_particle_pipm_index = _gen_particle_pi0_index = _gen_particle_ppm_index = _gen_particle_kpm_index = _gen_particle_kL_index = _gen_particle_n_index = _gen_particle_e_index = 0;
  GEN_particle_gamma_index = GEN_particle_pipm_index = GEN_particle_pi0_index = GEN_particle_ppm_index = GEN_particle_kpm_index = GEN_particle_kL_index = GEN_particle_n_index = GEN_particle_e_index = 0;
  for(int i=0; i<6; i++){
    ymerge[i] = 0;
  }
  for(int i=0; i<6; i++){
    ymerge_gen[i] = 0;
  }

  PFO_Energy_muon.clear(); PFO_Energy_muon_GENMatch_dR.clear(); PFO_Energy_muon_GENMatch_ID.clear(); PFO_Energy_muon_GENMatch_E.clear(); PFO_Energy_Charge.clear(); PFO_Energy_Charge_Ecal.clear(); PFO_Energy_Charge_Hcal.clear(); PFO_Energy_Charge_GENMatch_dR.clear(); PFO_Energy_Charge_GENMatch_ID.clear(); PFO_Energy_Charge_GENMatch_E.clear(); PFO_Energy_Neutral.clear(); PFO_Energy_Neutral_singleCluster.clear(); PFO_Energy_Neutral_singleCluster_R.clear();
  GEN_PFO_Energy_muon.clear(); GEN_PFO_Energy_pipm.clear(); GEN_PFO_Energy_pi0.clear(); GEN_PFO_Energy_ppm.clear(); GEN_PFO_Energy_kpm.clear(); GEN_PFO_Energy_kL.clear(); GEN_PFO_Energy_n.clear(); GEN_PFO_Energy_e.clear(); GEN_PFO_Energy_gamma.clear();
  PFO_Hits_Charge_E.clear(); PFO_Hits_Charge_R.clear(); PFO_Hits_Charge_theta.clear(); PFO_Hits_Charge_phi.clear();
  PFO_Hits_Neutral_E.clear(); PFO_Hits_Neutral_R.clear(); PFO_Hits_Neutral_theta.clear(); PFO_Hits_Neutral_phi.clear();

  mass = mass_gen_match = GEN_mass = mass_allParticles = 0;
  barrelRatio = 0;
  GEN_inv_E = GEN_inv_pt = 0;
  GEN_ISR_E = GEN_ISR_pt = 0;
  GEN_jet1_px = GEN_jet1_py = GEN_jet1_pz = GEN_jet1_E = 0;
  GEN_jet2_px = GEN_jet2_py = GEN_jet2_pz = GEN_jet2_E = 0;
  GEN_jet1_costheta = GEN_jet1_phi = 0;
  GEN_jet2_costheta = GEN_jet2_phi = 0;
  GEN_jet1_pt = GEN_jet2_pt = 0;
  GEN_jet1_nconstituents = GEN_jet2_nconstituents = GEN_nparticles = 0;
  GEN_constituents_E1tot = GEN_constituents_E2tot = 0;

}

