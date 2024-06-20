#ifndef TrueMuonTagAlg_h
#define TrueMuonTagAlg_h 1

#include "k4FWCore/DataHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#include "edm4hep/MCRecoTrackParticleAssociationCollection.h"

#include <UTIL/BitField64.h>
#include <UTIL/ILDConf.h>

#include "TRandom3.h"

class TrueMuonTagAlg : public GaudiAlgorithm {
 public:
  // Constructor of this form must be provided
  TrueMuonTagAlg( const std::string& name, ISvcLocator* pSvcLocator );

  // Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

 private:
  // input MCParticle
  DataHandle<edm4hep::MCParticleCollection>                              m_inMCParticleColHdl{"MCParticle", Gaudi::DataHandle::Reader, this};
  // input Tracks to make relation
  std::vector<DataHandle<edm4hep::TrackCollection>* >                    m_inTrackColHdls;
  Gaudi::Property<std::vector<std::string> >                             m_inTrackCollectionNames{this, "TrackList", {"SiTracks"}};
  // input TrackerAssociation to link TrackerHit and SimTrackerHit
  std::vector<DataHandle<edm4hep::MCRecoTrackerAssociationCollection>* > m_inAssociationColHdls;
  Gaudi::Property<std::vector<std::string> >                             m_inAssociationCollectionNames{this, "TrackerAssociationList", {"VXDTrackerHitAssociation",
        "SITTrackerHitAssociation", "SETTrackerHitAssociation", "FTDTrackerHitAssociation", "TPCTrackerHitAss"}};

  // output TrackParticleAssociation
  //std::vector<DataHandle<edm4hep::MCRecoTrackParticleAssociationCollection>* > m_outColHdls;
  std::vector<DataHandle<edm4hep::TrackCollection>* > m_outTrackColHdls;

  // muon tag efficiency 
  Gaudi::Property<double> _m_muonTagEff{this, "MuonTagEfficiency", 1.0};
  // Muon barrel/endcap separate angle
  Gaudi::Property<double> _m_muonDetTanTheta{this,"MuonDetTanTheta", 1.2};

  int m_nEvt;

  TRandom3 fRandom{1234};

};
#endif
