#ifndef JetClustering_h
#define JetClustering_h 1

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

class JetClustering : public Algorithm {
 public:
  // Constructor of this form must be provided
  JetClustering( const std::string& name, ISvcLocator* pSvcLocator );

  // Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

 private:
  DataHandle<edm4hep::ReconstructedParticleCollection> m_PFOColHdl{"PandoraPFOs", Gaudi::DataHandle::Reader, this};
  Gaudi::Property<std::string> m_algo{this, "Algorithm", "ee_kt_algorithm"};
  Gaudi::Property<int> m_nJets{this, "nJets", 2};
  Gaudi::Property<double> m_R{this, "R", 0.6};
  Gaudi::Property<std::string> m_outputFile{this, "OutputFile", "JetClustering.root"};

  int _nEvt;
  int _particle_index;
  int _jet_index;
  int _nparticles;
  double _time;

  TFile* _file;
  TTree* _tree;
  double jet1_px, jet1_py, jet1_pz, jet1_E;
  double jet2_px, jet2_py, jet2_pz, jet2_E;
  double jet1_costheta, jet1_phi;
  double jet2_costheta, jet2_phi;
  double jet1_pt, jet2_pt;
  int jet1_nconstituents, jet2_nconstituents;
  double constituents_E1tot;
  double constituents_E2tot;
  double ymerge[6];
  double mass;
  double mass_allpfo;

  void CleanVars(){

    _particle_index = 0;
    _jet_index = 0;
    _nparticles = 0;
    _time = 0;
    
    jet1_px = jet1_py = jet1_pz = jet1_E = 0;
    jet2_px = jet2_py = jet2_pz = jet2_E = 0;
    jet1_costheta = jet1_phi = 0;
    jet2_costheta = jet2_phi = 0;
    jet1_pt = jet2_pt = 0;
    jet1_nconstituents = jet2_nconstituents = 0;
    constituents_E1tot = constituents_E2tot = 0;
    for(int i=0; i<6; i++){
      ymerge[i] = 0;
    }
    mass = 0;
    mass_allpfo = 0;
  }

};
#endif

