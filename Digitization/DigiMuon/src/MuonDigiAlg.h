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
//#include <TRandom.h>
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


  // double EDepConvertADC(double edep){
  //   rand.SetSeed(0);
  //   randGen.SetSeed(0);
  //   double adc = 0;
  //   Double_t random_num = rand.Uniform(0, 1);
  //   if(random_num>0.1){
  //     double LandauRandom = randGen.Landau(47.09,7.922);
  //     adc = LandauRandom * edep / 0.00141;
  //   }
  //   return random_num;
  // }



 protected:


  TRandom3 rand_muon;

  TTree* hitloop;
  TTree* eventloop;
  TFile* m_wfile;
    
  SmartIF<IGeomSvc> m_geosvc;
  dd4hep::DDSegmentation::BitFieldCoder* m_decoder;
  dd4hep::rec::CellIDPositionConverter* m_cellIDConverter;
  dd4hep::Detector* m_dd4hep;

  std::vector<unsigned long long> MuonBarrel_cellid;
  std::vector<double> MuonBarrel_posx;
  std::vector<double> MuonBarrel_posy;
  std::vector<double> MuonBarrel_posz;
  std::vector<unsigned long long> cellidtemp;
  std::vector<double> edeptemp;
  std::vector<double> ADCtemp;
  std::vector<unsigned long long> MuonBarrel_stripcellid;
  std::vector<double> MuonBarrel_stripedep;





  mutable Gaudi::Property<int>   _writeNtuple{this,  "WriteNtuple", 1, "Write ntuple"};
  mutable Gaudi::Property<std::string> _filename{this, "OutFileName", "testout.root", "Output file name"};

  // Input collections
  DataHandle<edm4hep::SimTrackerHitCollection>            m_inputMuonBarrel{"MuonBarrelHitsCollection", Gaudi::DataHandle::Reader, this};
  // Output collections
  DataHandle<edm4hep::TrackerHitCollection>               m_outputMuonBarrel{"MuonBarrelTrackerHits", Gaudi::DataHandle::Writer, this};
  //DataHandle<edm4hep::MCRecoTrackerAssociationCollection> m_assMuonBarrel{"MuonBarrelTrackerHitAssociationCollection", Gaudi::DataHandle::Writer, this};

  edm4hep::TrackerHitCollection* trkhitVec;

  int m_nEvt=0;
};
#endif



