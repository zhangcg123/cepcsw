#ifndef TrackParticleRelationAlg_h
#define TrackParticleRelationAlg_h 1

#include "k4FWCore/DataHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#include "edm4hep/MCRecoTrackParticleAssociationCollection.h"

class TrackParticleRelationAlg : public GaudiAlgorithm {
 public:
  // Constructor of this form must be provided
  TrackParticleRelationAlg( const std::string& name, ISvcLocator* pSvcLocator );

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
        "SITTrackerHitAssociation", "SETTrackerHitAssociation", "FTDTrackerHitAssociation"}};

  // output TrackParticleAssociation
  std::vector<DataHandle<edm4hep::MCRecoTrackParticleAssociationCollection>* > m_outColHdls;

  int m_nEvt;
};
#endif
