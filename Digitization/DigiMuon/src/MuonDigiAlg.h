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
  void MuonHandle(int _i); // 2->Barrel  0->Endcap
  void Clear();
  void GetSimHit(edm4hep::SimTrackerHit _simhit, dd4hep::DDSegmentation::BitFieldCoder* _m_decoder, int _i, int _message[6], double & _Edep, unsigned long long & _cellid, double & _xydist, edm4hep::Vector3d & _pos, dd4hep::Position & _ddpos, std::array<unsigned long long, 2> & _key, double & _mcpz);
  double Gethit_sipm_length_Barrel(dd4hep::Position _ddpos, edm4hep::Vector3d _pos, int _message[6]);
  double Gethit_sipm_length_Endcap(dd4hep::Position _ddpos, edm4hep::Vector3d _pos, int _message[6]);
  double EdeptoADC(double _Edep, double _hit_sipm_length);
  void SaveData_mapcell(std::array<unsigned long long, 2> _key, double _Edep, int _message[6], edm4hep::Vector3d _pos);
  void Cut3();
  void Find_anotherlayer(int _i, std::array<unsigned long long, 2> _key1, std::array<unsigned long long, 2> _key2, double _ddposi, int & _anotherlayer_cell_num);
  void Save_trkhit(edm4hep::TrackerHitCollection* _trkhitVec, std::array<unsigned long long, 2> _key, int _pdgid, edm4hep::Vector3d _pos);
  void Save_onelayer_signal(edm4hep::TrackerHitCollection* _trkhitVec);



 protected:

  TTree* m_tree;
  TFile* m_wfile;
    
  SmartIF<IGeomSvc> m_geosvc;
  dd4hep::DDSegmentation::BitFieldCoder* m_decoder_barrel;
  dd4hep::DDSegmentation::BitFieldCoder* m_decoder_endcap;
  dd4hep::rec::CellIDPositionConverter* m_cellIDConverter;
  dd4hep::Detector* m_dd4hep;

  std::map<std::array<unsigned long long, 2>, double> map_cell_edep;
  std::map<std::array<unsigned long long, 2>, double> map_cell_adc;
  std::map<std::array<unsigned long long, 2>, int> map_cell_layer;
  std::map<std::array<unsigned long long, 2>, int> map_cell_slayer;
  std::map<std::array<unsigned long long, 2>, int> map_cell_strip;
  std::map<std::array<unsigned long long, 2>, int> map_cell_fe;
  std::map<std::array<unsigned long long, 2>, int> map_cell_env;
  std::map<std::array<unsigned long long, 2>, int> map_cell_pdgid;
  std::map<std::array<unsigned long long, 2>, edm4hep::Vector3d> map_cell_pos;
  std::map<int, double> map_muonhit;
  std::map<int, int> map_pdgid;

 
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
  int cell_superlayernumber;
  double cell_pt;


  Gaudi::Property<int>   _writeNtuple{this,  "WriteNtuple", 1, "Write ntuple"};
  Gaudi::Property<std::string> _filename{this, "OutFileName", "testout.root", "Output file name"};


  Gaudi::Property<double>  m_hitEff{ this, "SiPMEff", 1, "Efficiency of a single hit on a Strip" };
  Gaudi::Property<double>  m_EdepMin{ this, "EdepMin", 0.0001, "Minimum Edep of a mip" };
  Gaudi::Property<double>  m_hit_Edep_min{ this, "HitEdepMin", 0.000001, "Minimum Edep of a mip"};  
  Gaudi::Property<double>  m_hit_Edep_max{ this, "HitEdepMax", 0.1, "Maximum Edep of a mip"};  

  // number of strips parallel to beam direction 
  // in each slayer, each strip width=4cm, so 
  int strip_length[6] = {26, 38, 50, 62, 74, 86}; //for barrel
  double endcap_strip_length[193] = {2.12, 2.12, 2.12, 2.13, 2.14, 2.15, 2.16, 2.17, 2.19, 2.22,
                                     2.24, 2.28, 2.32, 2.37, 2.45, 2.65, 2.64, 2.63, 2.62, 2.61,
                                     2.60, 2.59, 2.57, 2.56, 2.54, 2.53, 2.51, 2.50, 2.48, 2.46, 
                                     2.44, 2.42, 2.40, 2.38, 2.36, 2.33, 2.31, 2.28, 2.26, 2.23,
                                     2.20, 2.17, 2.14, 2.11, 2.07, 2.04, 2.00, 1.97, 1.93, 1.89, 
                                     1.84, 1.80, 1.75, 1.70, 1.65, 1.60, 1.54, 1.48, 1.42, 1.35, 
                                     1.28, 1.20, 1.12, 1.02, 0.92, 0.80, 0.65, 0.46, 2.20, 2.20,
                                     2.20, 2.20, 2.20, 2.20, 2.20, 2.21, 2.21, 2.21, 2.21, 2.22,
                                     2.22, 2.22, 2.23, 2.23, 2.23, 2.24, 2.24, 2.25, 2.25, 2.26,
                                     2.26, 2.27, 2.28, 2.28, 2.29, 2.30, 2.31, 2.32, 2.32, 2.33,
                                     2.34, 2.35, 2.36, 2.38, 2.39, 2.40, 2.41, 2.43, 2.44, 2.45,
                                     2.47, 2.49, 2.50, 2.52, 2.54, 2.56, 2.58, 2.60, 2.62, 2.65, 
                                     2.67, 2.70, 2.73, 2.76, 2.79, 2.82, 2.86, 2.90, 2.94, 2.99,
                                     3.04, 3.10, 3.16, 3.23, 3.31, 3.41, 3.53, 3.70, 4.14, 4.12, 
                                     4.09, 4.06, 4.03, 4.00, 3.97, 3.94, 3.91, 3.87, 3.84, 3.81, 
                                     3.77, 3.74, 3.70, 3.67, 3.63, 3.59, 3.55, 3.51, 3.47, 3.43, 
                                     3.38, 3.34, 3.30, 3.25, 3.20, 3.15, 3.10, 3.05, 3.00, 2.95, 
                                     2.89, 2.83, 2.77, 2.71, 2.65, 2.58, 2.52, 2.45, 2.37, 2.30, 
                                     2.22, 2.14, 2.05, 1.96, 1.86, 1.76, 1.65, 1.53, 1.40, 1.25, 
                                     1.09, 0.89, 0.63};  


  // Input collections
  DataHandle<edm4hep::SimTrackerHitCollection>            m_inputMuonBarrel{"MuonBarrelHitsCollection", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::SimTrackerHitCollection>            m_inputMuonEndcap{"MuonEndcapHitsCollection", Gaudi::DataHandle::Reader, this};
  // Output collections
  DataHandle<edm4hep::TrackerHitCollection>               m_outputMuonBarrel{"MuonBarrelTrackerHits", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::TrackerHitCollection>               m_outputMuonEndcap{"MuonEndcapTrackerHits", Gaudi::DataHandle::Writer, this};

  //DataHandle<edm4hep::MCRecoTrackerAssociationCollection> m_assMuonBarrel{"MuonBarrelTrackerHitAssociationCollection", Gaudi::DataHandle::Writer, this};
  SmartIF<IRndmGenSvc> m_randSvc;
  edm4hep::TrackerHitCollection* trkhitVec;
  int m_nEvt=0;
};
#endif



