#ifndef TRACK_CREATOR_C
#define TRACK_CREATOR_C

#include "Tools/TrackCreator.h"

namespace Cyber{

  TrackCreator::TrackCreator(const Settings& m_settings) : settings( m_settings ){

  };


  StatusCode TrackCreator::CreateTracks( CyberDataCol& m_DataCol, 
                                         std::vector<DataHandle<edm4hep::TrackCollection>*>& r_TrackCols, 
                                         DataHandle<edm4hep::MCRecoTrackParticleAssociationCollection>* r_MCParticleTrkCol ){

    if(r_TrackCols.size()==0 || settings.map_stringVecPars.at("trackCollections").size()==0) StatusCode::SUCCESS;

    //Save readin collections
    m_DataCol.collectionMap_Track.clear(); 
    for(int icol=0; icol<r_TrackCols.size(); icol++){
      const edm4hep::TrackCollection* const_TrkCol = r_TrackCols[icol]->get(); 

      std::vector<edm4hep::Track> m_TrkCol; m_TrkCol.clear();
      for(unsigned int icol=0; icol<const_TrkCol->size(); icol++){
        edm4hep::Track m_trk = const_TrkCol->at(icol);
        if(!m_trk.isAvailable() || m_trk.getType()==0) continue;
        m_TrkCol.push_back(m_trk);
      }

      m_DataCol.collectionMap_Track[ settings.map_stringVecPars.at("trackCollections")[icol] ] = m_TrkCol; 
    }


    //Convert to local objects
    std::vector<std::shared_ptr<Cyber::Track>> m_trkCol; m_trkCol.clear();
    const edm4hep::MCRecoTrackParticleAssociationCollection* const_MCPTrkAssoCol = r_MCParticleTrkCol->get();

    for(auto iter : m_DataCol.collectionMap_Track){
      auto const_TrkCol = iter.second; 
      for(int itrk=0; itrk<const_TrkCol.size(); itrk++){

        //Cyber::Track* m_trk = new Cyber::Track();
        std::shared_ptr<Cyber::Track> m_trk = std::make_shared<Cyber::Track>();
        std::vector<Cyber::TrackState> m_trkstates;

        for(int its=0; its<const_TrkCol[itrk].trackStates_size(); its++){
          Cyber::TrackState m_trkst;
          m_trkst.D0 = const_TrkCol[itrk].getTrackStates(its).D0;
          m_trkst.Z0 = const_TrkCol[itrk].getTrackStates(its).Z0;
          m_trkst.phi0 = const_TrkCol[itrk].getTrackStates(its).phi;
          m_trkst.tanLambda = const_TrkCol[itrk].getTrackStates(its).tanLambda;
          m_trkst.Omega = const_TrkCol[itrk].getTrackStates(its).omega;
          m_trkst.Kappa = m_trkst.Omega*1000./(0.299792458*settings.map_floatPars.at("BField"));   
          m_trkst.location = const_TrkCol[itrk].getTrackStates(its).location;
          m_trkst.referencePoint.SetXYZ( const_TrkCol[itrk].getTrackStates(its).referencePoint[0],
                                         const_TrkCol[itrk].getTrackStates(its).referencePoint[1],
                                         const_TrkCol[itrk].getTrackStates(its).referencePoint[2] );

          m_trkstates.push_back(m_trkst);        
        }
        m_trk->setTrackStates("Input", m_trkstates);
        m_trk->setType(const_TrkCol[itrk].getType());
        m_trk->setOriginTrack( const_TrkCol[itrk] );

        for(int ilink=0; ilink<const_MCPTrkAssoCol->size(); ilink++){
          if( const_TrkCol[itrk] == const_MCPTrkAssoCol->at(ilink).getRec() ) {
            m_trk->addLinkedMCP( std::make_pair(const_MCPTrkAssoCol->at(ilink).getSim(), const_MCPTrkAssoCol->at(ilink).getWeight()) );
            break;
          }
        }


        m_trkCol.push_back(m_trk);
      }
    }

    //SelectGoodTrack(m_trkCol);
    m_DataCol.TrackCol = m_trkCol;


    //Track extrapolation
    //  Write settings: geometry description
    //  m_TrkExtraSettings.map_floatPar["Nlayers"] = 28;

    m_TrkExtraAlg = new TrackExtrapolatingAlg();
    m_TrkExtraAlg->ReadSettings(m_TrkExtraSettings);
    m_TrkExtraAlg->Initialize( m_DataCol );
    m_TrkExtraAlg->RunAlgorithm( m_DataCol );
    m_TrkExtraAlg->ClearAlgorithm();
    delete m_TrkExtraAlg;
    m_TrkExtraAlg = nullptr;

    return StatusCode::SUCCESS;
  }


  StatusCode TrackCreator::CreateTracksFromMCParticle(CyberDataCol& m_DataCol, 
                                          DataHandle<edm4hep::MCParticleCollection>& r_MCParticleCol){


    // Convert MC charged particles to local Track objects.
    // Assuming the tracks are ideal helixes.

    // Get charged MC particles with generatorStatus==1
    const edm4hep::MCParticleCollection* const_MCPCol = r_MCParticleCol.get();
    std::vector<edm4hep::MCParticle> m_MCPvec; m_MCPvec.clear(); 
    for(int i=0; i<const_MCPCol->size(); i++){
      edm4hep::MCParticle m_MCp = const_MCPCol->at(i);
      if(m_MCp.getGeneratorStatus()==1 && m_MCp.getCharge()!=0)
        m_MCPvec.push_back(m_MCp);
    }

    // Convert to local Track objects
    std::vector<std::shared_ptr<Cyber::Track>> m_trkCol; m_trkCol.clear();
    for(auto mcp : m_MCPvec){
      std::shared_ptr<Cyber::Track> m_trk = std::make_shared<Cyber::Track>();
      std::vector<Cyber::TrackState> m_trkstates;

      TVector3 mcp_vertex(mcp.getVertex().x, mcp.getVertex().y, mcp.getVertex().z);
      TVector3 mcp_p(mcp.getMomentum().x, mcp.getMomentum().y, mcp.getMomentum().z);
      double mcp_pT = TMath::Sqrt(mcp_p.X()*mcp_p.X() + mcp_p.Y()*mcp_p.Y());
      double charge = mcp.getCharge();

      // Evaluate track state at vertex
      Cyber::TrackState m_trkst;  
      m_trkst.location = 1;  // At IP
      m_trkst.D0 = 0;
      m_trkst.Z0 = 0;
      m_trkst.phi0 = TMath::ATan2(mcp_p.Y(), mcp_p.X());
      m_trkst.Kappa = 1 / mcp_pT;
      if(charge<0) m_trkst.Kappa = -m_trkst.Kappa;
      m_trkst.tanLambda = mcp_p.Z() / mcp_pT;
      m_trkst.Omega = 0.3 * settings.map_floatPars.at("BField") / 1000. / mcp_pT;
      m_trkst.referencePoint = mcp_vertex;
      m_trkstates.push_back(m_trkst);

      m_trk->setTrackStates("Input", m_trkstates);
      m_trk->setType(0);  // It is a "MC track" and not detected by any tracker system
      m_trk->addLinkedMCP( std::make_pair(mcp, 1.) );
      m_trkCol.push_back(m_trk);
    }
    m_DataCol.TrackCol = m_trkCol;

    m_TrkExtraSettings.map_intPars["Input_track"] = 1;

    m_TrkExtraAlg = new TrackExtrapolatingAlg();
    m_TrkExtraAlg->ReadSettings(m_TrkExtraSettings);
    m_TrkExtraAlg->Initialize( m_DataCol );
    m_TrkExtraAlg->RunAlgorithm( m_DataCol );
    m_TrkExtraAlg->ClearAlgorithm();
    delete m_TrkExtraAlg;
    m_TrkExtraAlg = nullptr;

    return StatusCode::SUCCESS;
  }


  StatusCode TrackCreator::SelectGoodTrack(std::vector<std::shared_ptr<Cyber::Track>>& trkCol){
    if(trkCol.size()<2) return StatusCode::SUCCESS;

    for(int itrk=0; itrk<trkCol.size(); itrk++){
      //Endpoint cut
      if( fabs( trkCol[itrk]->getEndPoint().z() )<settings.map_floatPars.at("TrkEndZCut") && 
          trkCol[itrk]->getEndPoint().Perp()>settings.map_floatPars.at("TrkEndRCutMin") &&  
          trkCol[itrk]->getEndPoint().Perp()<settings.map_floatPars.at("TrkEndRCutMax") ){  

        trkCol.erase(trkCol.begin() + itrk);
        itrk--;
        continue;
      }
      //Start point cut
      if( trkCol[itrk]->getStartPoint().Perp()>settings.map_floatPars.at("TrkStartRCutMin") &&
          trkCol[itrk]->getStartPoint().Perp()<settings.map_floatPars.at("TrkStartRCutMax") ){ 

        trkCol.erase(trkCol.begin() + itrk);
        itrk--;
        continue;
      }
      //Track length cut
      if( (trkCol[itrk]->getEndPoint().Perp()-trkCol[itrk]->getStartPoint().Perp() )<settings.map_floatPars.at("TrkLengthCut") ){

        trkCol.erase(trkCol.begin() + itrk);
        itrk--;
        continue;
      }
    }

    if(trkCol.size()<2) return StatusCode::SUCCESS;

    //Remove the broken tracks
    std::sort(trkCol.begin(), trkCol.end(),  compTrkIP);
    for(int itrk=0; itrk<trkCol.size()-1; itrk++){
      for(int jtrk=itrk+1; jtrk<trkCol.size(); jtrk++){
        double deltaP = (trkCol[itrk]->getP3() - trkCol[jtrk]->getP3()).Mag();
        if( trkCol[jtrk]->getP3().Perp() < settings.map_floatPars.at("BrokenTrkMinP") &&
            ( deltaP/max(trkCol[itrk]->getMomentum(), trkCol[jtrk]->getMomentum()) < settings.map_floatPars.at("BrokenTrkDeltaPCut") ||
            (trkCol[itrk]->getEndPoint()-trkCol[jtrk]->getStartPoint()).Mag() < settings.map_floatPars.at("BrokenTrkDistance") ) ){
          trkCol.erase(trkCol.begin() + jtrk);
          jtrk--;
          if(itrk>jtrk+1) itrk--;
        }
      }
    }

    std::sort(trkCol.begin(), trkCol.end(),  compTrkP);

    return StatusCode::SUCCESS;
  }

};
#endif
