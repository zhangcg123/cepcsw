#ifndef MuonDigiAlg_H
#define MuonDigiAlg_H

#include "k4FWCore/DataHandle.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"

#include <DDRec/DetectorData.h>
#include "DetInterface/IGeomSvc.h"

#include "k4FWCore/DataHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "edm4hep/MutableCaloHitContribution.h"
#include "edm4hep/MutableSimCalorimeterHit.h"
#include "edm4hep/SimCalorimeterHit.h"
#include "edm4hep/CalorimeterHit.h"
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/MCRecoCaloAssociationCollection.h"
#include "edm4hep/MCRecoCaloParticleAssociationCollection.h"

#include <DDRec/DetectorData.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DD4hep/Segmentations.h> 
#include "DetInterface/IGeomSvc.h"
#include "TVector3.h"
#include "TRandom3.h"
#include "TFile.h"
#include "TString.h"
#include "TH3.h"
#include "TH1.h"

#include <cstdlib>
#include "time.h"
#include <TTimeStamp.h> 
#include <ctime>
#include <iostream>
#include <TMath.h>
#define PI 3.141592653
class MuonDigiAlg : public GaudiAlgorithm
{
 public:
  
  MuonDigiAlg(const std::string& name, ISvcLocator* svcLoc);
 
  virtual StatusCode initialize() ;
  virtual StatusCode execute() ; 
  virtual StatusCode finalize() ;
  void Clear();


 protected:


  TRandom3 rand_muon;

  TTree* m_tree;
  TFile* m_wfile;
    
  SmartIF<IGeomSvc> m_geosvc;
  dd4hep::DDSegmentation::BitFieldCoder* m_decoder;
  dd4hep::rec::CellIDPositionConverter* m_cellIDConverter;
  dd4hep::Detector* m_dd4hep;

  std::map<unsigned long long, double> map_cell_edep;
  std::map<unsigned long long, double> map_cell_adc;
  std::map<unsigned long long, int> map_cell_layer;
  std::map<unsigned long long, int> map_cell_slayer;
  std::map<unsigned long long, int> map_cell_strip;
  std::map<unsigned long long, int> map_cell_fe;
  std::map<unsigned long long, int> map_cell_env;


 
#define MAX_SIZE 10000 

  int n_hit, n_cell;

  unsigned long long hit_cellid[MAX_SIZE];
  double hit_posx[MAX_SIZE];
  double hit_posy[MAX_SIZE];
  double hit_posz[MAX_SIZE];
  double hit_edep[MAX_SIZE];
  int hit_layer[MAX_SIZE];
  int hit_slayer[MAX_SIZE];
  int hit_strip[MAX_SIZE];
  int hit_fe[MAX_SIZE];
  int hit_env[MAX_SIZE];

  unsigned long long cell_cellid[MAX_SIZE];
  double cell_edep[MAX_SIZE];
  double cell_adc[MAX_SIZE];
  double cell_posx[MAX_SIZE];
  double cell_posy[MAX_SIZE];
  double cell_posz[MAX_SIZE];
  int cell_layer[MAX_SIZE];
  int cell_slayer[MAX_SIZE];
  int cell_strip[MAX_SIZE];
  int cell_fe[MAX_SIZE];
  int cell_env[MAX_SIZE];


  Gaudi::Property<int>   _writeNtuple{this,  "WriteNtuple", 1, "Write ntuple"};
  Gaudi::Property<std::string> _filename{this, "OutFileName", "testout.root", "Output file name"};

  Gaudi::Property<double>  m_hitEff{ this, "hitEff", 0.9, "Efficiency of a single hit on a Strip" };


  // Input collections
  DataHandle<edm4hep::SimTrackerHitCollection>            m_inputMuonBarrel{"MuonBarrelHitsCollection", Gaudi::DataHandle::Reader, this};
  // Output collections
  DataHandle<edm4hep::TrackerHitCollection>               m_outputMuonBarrel{"MuonBarrelTrackerHits", Gaudi::DataHandle::Writer, this};
  //DataHandle<edm4hep::MCRecoTrackerAssociationCollection> m_assMuonBarrel{"MuonBarrelTrackerHitAssociationCollection", Gaudi::DataHandle::Writer, this};

  edm4hep::TrackerHitCollection* trkhitVec;

  int m_nEvt=0;
};
#endif



