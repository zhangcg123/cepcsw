#ifndef TrackParticleRelationAlg_h
#define TrackParticleRelationAlg_h 1

#include "k4FWCore/DataHandle.h"
#include "GaudiKernel/Algorithm.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/EDM4hepVersion.h"
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
#include "edm4hep/TrackerHitSimTrackerHitLinkCollection.h"
#include "edm4hep/TrackMCParticleLinkCollection.h"
using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::TrackerHitSimTrackerHitLinkCollection;
using CEPCSWTrackMCParticleLinkCollection = edm4hep::TrackMCParticleLinkCollection;
#else
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#include "edm4hep/MCRecoTrackParticleAssociationCollection.h"
using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::MCRecoTrackerAssociationCollection;
using CEPCSWTrackMCParticleLinkCollection = edm4hep::MCRecoTrackParticleAssociationCollection;
#endif

class TrackParticleRelationAlg : public Algorithm {
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
  std::vector<DataHandle<CEPCSWTrackerHitSimTrackerHitLinkCollection>* > m_inAssociationColHdls;
  Gaudi::Property<std::vector<std::string> >                             m_inAssociationCollectionNames{this, "TrackerAssociationList", {"VXDTrackerHitAssociation",
        "SITTrackerHitAssociation", "SETTrackerHitAssociation", "FTDTrackerHitAssociation"}};

  // output TrackParticleAssociation
  std::vector<DataHandle<CEPCSWTrackMCParticleLinkCollection>* > m_outColHdls;

  int m_nEvt;
};
#endif
