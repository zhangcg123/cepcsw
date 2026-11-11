#ifndef OUTPUT_CREATOR_H
#define OUTPUT_CREATOR_H

#include "k4FWCore/DataHandle.h"
#include "edm4hep/MutableCalorimeterHit.h"
#include "edm4hep/Vector3f.h"
#include "CyberDataCol.h"
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
#include "edm4hep/ClusterMCParticleLinkCollection.h"
#else
#include "edm4hep/MCRecoClusterParticleAssociationCollection.h"
#endif
#include "Tools/Algorithm.h"
#include <string>
//#include "Tools/HelixClassD.h"

namespace Cyber{

#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
  using ClusterTruthCollection = edm4hep::ClusterMCParticleLinkCollection;
#else
  using ClusterTruthCollection = edm4hep::MCRecoClusterParticleAssociationCollection;
#endif

  class OutputCreator{
  public: 

    OutputCreator( const Settings& m_settings);
    ~OutputCreator() {};

    StatusCode CreateClusterSnapshot(
        const CyberDataCol& data,
        const std::string& sourceName,
        DataHandle<edm4hep::ClusterCollection>& clusters,
        DataHandle<edm4hep::CalorimeterHitCollection>& hits);

    StatusCode CreatePFOSnapshot(
        const CyberDataCol& data,
        edm4hep::ReconstructedParticleCollection& pfos,
        edm4hep::ClusterCollection& ecal, edm4hep::CalorimeterHitCollection& ecalHits,
        edm4hep::ClusterCollection& hcal, edm4hep::CalorimeterHitCollection& hcalHits,
        edm4hep::TrackCollection& truthTracks,
        ClusterTruthCollection& ecalTruth);

    StatusCode CreateOutputCollections( CyberDataCol& m_DataCol, 
                                        DataHandle<edm4hep::CalorimeterHitCollection>& m_outRecHitsHandler, 
                                        DataHandle<edm4hep::CalorimeterHitCollection>& m_outRecCoreHandler, 
                                        DataHandle<edm4hep::CalorimeterHitCollection>& m_outRecHcalHitsHandler, 
                                        DataHandle<edm4hep::TrackCollection>& m_outTrkHandler, 
                                        std::map<std::string, DataHandle<edm4hep::ClusterCollection>*>& m_outClusterColHandler, 
                                        DataHandle<edm4hep::ReconstructedParticleCollection>& m_recPFOHandler );


    StatusCode Reset() { return StatusCode::SUCCESS; }

    edm4hep::Track TruthTrack(edm4hep::MCParticle _mcp, edm4hep::TrackCollection* _trkCol );

  private: 
    const Cyber::Settings   settings;
  
  };
};
#endif
