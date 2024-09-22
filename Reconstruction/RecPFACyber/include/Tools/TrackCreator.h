#ifndef TRACK_CREATOR_H
#define TRACK_CREATOR_H

#include "CyberDataCol.h"
#include "Tools/Algorithm.h"
#include "Algorithm/TrackExtrapolatingAlg.h"
#include "TVector3.h"

namespace Cyber{
  class TrackCreator{

  public: 

    //initialize a CaloHitCreator
    TrackCreator( const Settings& m_settings );
    ~TrackCreator() { delete m_TrkExtraAlg; };
   
    StatusCode CreateTracks( CyberDataCol& m_DataCol, 
                             std::vector<DataHandle<edm4hep::TrackCollection>*>& r_TrackCols, 
                             DataHandle<edm4hep::MCRecoTrackParticleAssociationCollection>* r_MCParticleTrkCol ); 
   
    StatusCode CreateTracksFromMCParticle(CyberDataCol& m_DataCol, 
                                          DataHandle<edm4hep::MCParticleCollection>& r_MCParticleCol);


    StatusCode Reset(){};

  private: 
    const Cyber::Settings  settings; 
    Cyber::Algorithm*      m_TrkExtraAlg; 
    Cyber::Settings        m_TrkExtraSettings;  


  };
};
#endif
