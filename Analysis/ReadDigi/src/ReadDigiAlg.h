#ifndef READ_DIGI_H
#define READ_DIGI_H

#include "k4FWCore/DataHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/TrackerHit.h"
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/MCRecoTrackParticleAssociationCollection.h"
#include "HelixClassD.hh"

#include "TFile.h"
#include "TTree.h"
#include "TVector3.h"
using namespace std;

class ReadDigiAlg : public GaudiAlgorithm
{

public :

  ReadDigiAlg(const std::string& name, ISvcLocator* svcLoc);

  virtual StatusCode initialize();
  virtual StatusCode execute();
  virtual StatusCode finalize();

  StatusCode Clear(); 

private :
  DataHandle<edm4hep::MCParticleCollection> m_MCParticleCol{"MCParticle", Gaudi::DataHandle::Reader, this};

  //DataHandle<edm4hep::TrackerHitCollection> m_SITRawColHdl{"SITTrackerHits", Gaudi::DataHandle::Reader, this};
  //DataHandle<edm4hep::TrackerHitCollection> m_SETRawColHdl{"SETTrackerHits", Gaudi::DataHandle::Reader, this};
  //DataHandle<edm4hep::TrackerHitCollection> m_FTDRawColHdl{"FTDStripTrackerHits", Gaudi::DataHandle::Reader, this};  

  //DataHandle<edm4hep::TrackerHitCollection> m_VTXTrackerHitColHdl{"VTXTrackerHits", Gaudi::DataHandle::Reader, this};
  //DataHandle<edm4hep::TrackerHitCollection> m_SITTrackerHitColHdl{"SITSpacePoints", Gaudi::DataHandle::Reader, this};
  //DataHandle<edm4hep::TrackerHitCollection> m_TPCTrackerHitColHdl{"TPCTrackerHits", Gaudi::DataHandle::Reader, this};
  //DataHandle<edm4hep::TrackerHitCollection> m_SETTrackerHitColHdl{"SETSpacePoints", Gaudi::DataHandle::Reader, this};
  //DataHandle<edm4hep::TrackerHitCollection> m_FTDSpacePointColHdl{"FTDSpacePoints", Gaudi::DataHandle::Reader, this};
  //DataHandle<edm4hep::TrackerHitCollection> m_FTDPixelTrackerHitColHdl{"FTDPixelTrackerHits", Gaudi::DataHandle::Reader, this};

  DataHandle<edm4hep::TrackCollection>      m_SiTrk{"SiTracks", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::TrackCollection>      m_TPCTrk{"ClupatraTracks", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::TrackCollection>      m_fullTrk{"CompleteTracks", Gaudi::DataHandle::Reader, this};

  DataHandle<edm4hep::MCRecoTrackParticleAssociationCollection> m_TPCTrkAssoHdl{"ClupatraTracksParticleAssociation",  Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::MCRecoTrackParticleAssociationCollection> m_fullTrkAssoHdl{"CompleteTracksParticleAssociation",  Gaudi::DataHandle::Reader, this};

  //DataHandle<edm4hep::SimCalorimeterHitCollection> m_ECalBarrelHitCol{"EcalBarrelCollection", Gaudi::DataHandle::Reader, this};
  //DataHandle<edm4hep::SimCalorimeterHitCollection> m_ECalEndcapHitCol{"EcalEndcapCollection", Gaudi::DataHandle::Reader, this};
  //DataHandle<edm4hep::SimCalorimeterHitCollection> m_HCalBarrelHitCol{"HcalBarrelCollection", Gaudi::DataHandle::Reader, this};
  //DataHandle<edm4hep::SimCalorimeterHitCollection> m_HCalEndcapHitCol{"HcalEndcapsCollection", Gaudi::DataHandle::Reader, this};

  mutable Gaudi::Property<std::string> _filename{this, "OutFileName", "testout.root", "Output file name"};
  mutable Gaudi::Property<double> _Bfield{this, "BField", 3., "Magnet field"};


  typedef std::vector<float> FloatVec;
  typedef std::vector<int>   IntVec;


  int _nEvt; 
  TFile *m_wfile;
  TTree *m_mctree, *m_sitrktree, *m_tpctrktree, *m_fulltrktree;

  //MCParticle
  int N_MCP;
  FloatVec MCP_px, MCP_py, MCP_pz, MCP_E, MCP_endPoint_x, MCP_endPoint_y, MCP_endPoint_z;
  IntVec MCP_pdgid, MCP_gStatus; 

  //Tracker 
  int N_SiTrk, N_TPCTrk, N_fullTrk;
  //int N_VTXhit, N_SIThit, N_TPChit, N_SEThit, N_FTDhit, N_SITrawhit, N_SETrawhit;
  FloatVec m_trk_px, m_trk_py, m_trk_pz, m_trk_p;
  FloatVec m_trkstate_d0, m_trkstate_z0, m_trkstate_phi, m_trkstate_tanL, m_trkstate_omega, m_trkstate_kappa;
  FloatVec m_trkstate_refx, m_trkstate_refy, m_trkstate_refz;
  IntVec m_trkstate_tag, m_trkstate_location, m_trk_Nhit, m_trk_type;
  FloatVec m_truthMC_tag, m_truthMC_pid, m_truthMC_px, m_truthMC_py, m_truthMC_pz, m_truthMC_E;
  FloatVec m_truthMC_EPx, m_truthMC_EPy, m_truthMC_EPz, m_truthMC_weight;


  //ECal
  //int Nhit_EcalB;
  //int Nhit_EcalE;
  //int Nhit_HcalB; 
  //int Nhit_HcalE;
  //IntVec CaloHit_type; //0: EM hit.   1: Had hit.
  //FloatVec CaloHit_x, CaloHit_y, CaloHit_z, CaloHit_E, CaloHit_Eem, CaloHit_Ehad, CaloHit_aveT, CaloHit_Tmin, CaloHit_TmaxE;


};

#endif
