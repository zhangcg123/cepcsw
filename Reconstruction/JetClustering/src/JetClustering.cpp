#include "JetClustering.h"

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

DECLARE_COMPONENT( JetClustering )

//------------------------------------------------------------------------------
JetClustering::JetClustering( const string& name, ISvcLocator* pSvcLocator )
    : Algorithm( name, pSvcLocator ) {

  declareProperty("InputPFOs", m_PFOColHdl, "Handle of the Input PFO collection");
  declareProperty("Algorithm", m_algo, "Jet clustering algorithm");
  declareProperty("nJets", m_nJets, "Number of jets to be clustered");
  declareProperty("R", m_R, "Jet clustering radius");
  declareProperty("OutputFile", m_outputFile, "Output file name");

}

//------------------------------------------------------------------------------
StatusCode JetClustering::initialize(){

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
  _tree->Branch("mass_allpfo", &mass_allpfo, "mass_allpfo/D");
  _tree->Branch("ymerge", ymerge, "ymerge[6]/D");
  
  return StatusCode::SUCCESS;

}

//------------------------------------------------------------------------------
StatusCode JetClustering::execute(){
  
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
  //initial indces
  CleanVars();

  //resize particles. there are many particles are empty, maybe upstream bugs
  TLorentzVector p4_allpfo; 
  vector<PseudoJet> input_particles;
  for(const auto& pfo : *PFO){
    if ( isnan(pfo.getMomentum()[0]) || isnan(pfo.getMomentum()[1]) || isnan(pfo.getMomentum()[2]) || pfo.getEnergy() == 0) {
      info() << "Particle " << _particle_index << " px: " << pfo.getMomentum()[0] << " py: " << pfo.getMomentum()[1] << " pz: " << pfo.getMomentum()[2] << " E: " << pfo.getEnergy()  << endmsg;
      _particle_index++;
      continue;
    }
    else{
      input_particles.push_back(PseudoJet(pfo.getMomentum()[0], pfo.getMomentum()[1], pfo.getMomentum()[2], pfo.getEnergy()));
      TLorentzVector p4_pfo; 
      p4_pfo.SetPxPyPzE( pfo.getMomentum()[0], pfo.getMomentum()[1], pfo.getMomentum()[2], pfo.getEnergy() );
      p4_allpfo += p4_pfo;
      _particle_index++;
      debug() << "Particle " << _particle_index << " px: "<<pfo.getMomentum()[0]<<" py: "<<pfo.getMomentum()[1]<<" pz: "<<pfo.getMomentum()[2]<<" E: "<<pfo.getEnergy()<<endmsg;
    }
  }
  mass_allpfo = p4_allpfo.M();

  // create a jet definition
  int nJets = m_nJets;
  double R = m_R;

  JetDefinition jet_def(ee_kt_algorithm);
  //JetDefinition jet_def(antikt_algorithm, R);

  // run the clustering, extract the jets
  //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

  ClusterSequence clust_seq(input_particles, jet_def);
  
  vector<PseudoJet> jets = sorted_by_pt(clust_seq.exclusive_jets(nJets));
  
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  
  //_time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
  
  //info() << "FastJet clustering done in seconds: " << _time << endmsg;
  
  //string jet_def_str = jet_def.description();
  //debug() << "Clustering with " << jet_def_str << " for " << nJets << " jets with R = " << R << endmsg;
  
  // print the jets
  info() << "                     px           py           pz           E           costheta  phi " << endmsg;
  for (unsigned i = 0; i < jets.size(); i++) {
    info() << "   jet " << i << "             " << jets[i].px() << " " << jets[i].py() << " " << jets[i].pz() << " " << jets[i].E() << " " << jets[i].cos_theta() << " " << jets[i].phi() << endmsg;
    vector<PseudoJet> constituents = jets[i].constituents();
    for (unsigned j = 0; j < constituents.size(); j++) {
      if ( i == 1 )constituents_E1tot += constituents[j].E();
      if ( i == 2 )constituents_E2tot += constituents[j].E();
      info() << "    constituent  " << j << " " << constituents[j].px() << " " << constituents[j].py() << " " << constituents[j].pz() << " " << constituents[j].E() << " " << constituents[j].cos_theta() << " " << constituents[j].phi() << endmsg;
    }
  }
  info() << "Total energy of constituents: " << constituents_E1tot << " " << constituents_E2tot << endmsg;


  double _ymin[20];
  for(int i=1; i<6;i++){
    _ymin[i-1] = clust_seq.exclusive_ymerge (i);
    info() << " -log10(y" << i << i+1 << ") = " << -log10(_ymin[i-1]) << endmsg;
  }
  

  mass = (jets[0] + jets[1]).m();

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
  _tree->Fill();

  _nEvt++;
  return StatusCode::SUCCESS;

}

//------------------------------------------------------------------------------
StatusCode JetClustering::finalize(){
  debug() << "Finalizing..." << endmsg;
  _file->Write();
  return StatusCode::SUCCESS;
}
