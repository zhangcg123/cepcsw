#ifndef OUTPUT_CREATOR_C
#define OUTPUT_CREATOR_C

#include "Tools/OutputCreator.h"

namespace Cyber{

  OutputCreator::OutputCreator( const Settings& m_settings ): settings(m_settings){

  }


  StatusCode OutputCreator::CreateOutputCollections( CyberDataCol& m_DataCol,
                                                     DataHandle<edm4hep::CalorimeterHitCollection>& m_outRecHitsHandler,
                                                     DataHandle<edm4hep::CalorimeterHitCollection>& m_outRecCoreHandler,
                                                     DataHandle<edm4hep::CalorimeterHitCollection>& m_outRecHcalHitsHandler,
                                                     DataHandle<edm4hep::TrackCollection>& m_outTrkHandler,
                                                     std::map<std::string, DataHandle<edm4hep::ClusterCollection>*>& m_outClusterColHandler,
                                                     DataHandle<edm4hep::ReconstructedParticleCollection>& m_recPFOHandler ){

    //Register the collections
    edm4hep::CalorimeterHitCollection* m_calohitCol = m_outRecHitsHandler.createAndPut();
    edm4hep::CalorimeterHitCollection* m_corehitCol = m_outRecCoreHandler.createAndPut();
    edm4hep::CalorimeterHitCollection* m_hcalhitCol = m_outRecHcalHitsHandler.createAndPut();
    edm4hep::ReconstructedParticleCollection* m_pfocol = m_recPFOHandler.createAndPut();
    edm4hep::ClusterCollection* m_Ecalcluster = m_outClusterColHandler["EcalCluster"]->createAndPut();
    edm4hep::ClusterCollection* m_Ecalcore    = m_outClusterColHandler["EcalCore"]->createAndPut();
    edm4hep::ClusterCollection* m_Hcalcluster = m_outClusterColHandler["HcalCluster"]->createAndPut();

    edm4hep::TrackCollection* m_trkCol = nullptr;
    if(settings.map_boolPars.at("UseTruthTrk")) m_trkCol = m_outTrkHandler.createAndPut();


    //PFO
    std::vector<std::shared_ptr<Cyber::PFObject>> p_pfos = m_DataCol.map_PFObjects[settings.map_stringPars.at("OutputPFO")];
/*
std::cout<<"  Input PFO size: "<<p_pfos.size()<<std::endl;    
 double totE_Ecal = 0;
 double totE_Hcal = 0;
 for(int i=0; i<p_pfos.size(); i++){
   cout<<"    PFO #"<<i<<": track size "<<p_pfos[i]->getTracks().size()<<", leading P "<<p_pfos[i]->getTrackMomentum();
   cout<<", ECAL cluster size "<<p_pfos[i]->getECALClusters().size()<<", totE "<<p_pfos[i]->getECALClusterEnergy();
   cout<<", HCAL cluster size "<<p_pfos[i]->getHCALClusters().size()<<", totE "<<p_pfos[i]->getHCALClusterEnergy()<<endl;
   totE_Ecal += p_pfos[i]->getECALClusterEnergy();
   totE_Hcal += p_pfos[i]->getHCALClusterEnergy();
 }
 cout<<"-----Neutral cluster Ecal total energy: "<<totE_Ecal<<", Hcal total energy: "<<totE_Hcal<<endl;
*/

    for(int ip=0; ip<p_pfos.size(); ip++){
      auto m_pfo = m_pfocol->create();
      std::vector<const Track*> vec_trks = p_pfos[ip]->getTracks();
      std::vector<const Calo3DCluster*> vec_Ecalclus = p_pfos[ip]->getECALClusters();
      std::vector<const Calo3DCluster*> vec_Hcalclus = p_pfos[ip]->getHCALClusters();

      TVector3 vec_Pos(0.,0.,0.);

      //Write ECAL cluster
      double EcalClusE = 0;
      for(int ic=0; ic<vec_Ecalclus.size(); ic++){
        auto p_clus = vec_Ecalclus[ic];
        auto m_clus = m_Ecalcluster->create();

        //create high granularity ECAL hits
        for(int ih=0; ih<p_clus->getCaloHits().size(); ih++){
          const Cyber::CaloHit* p_hit = p_clus->getCaloHits()[ih];
          edm4hep::Vector3f pos(p_hit->getPosition().x(), p_hit->getPosition().y(), p_hit->getPosition().z());
          int _module = p_hit->getModule();
          int _layer = p_hit->getLayer();
          int _cellID = (_module<<5) + _layer;  //A self-defined cell-ID. 
   
          auto _hit = m_calohitCol->create();
          _hit.setCellID(_cellID);
          _hit.setEnergy( p_hit->getEnergy()*settings.map_floatPars.at("ECALCalib") );
          _hit.setPosition( pos );
          _hit.setType(1); //Ecal barrel
          m_clus.addToHits(_hit);
        }
   
        //create cluster core from 2D cluster
        auto m_core = m_Ecalcore->create();
        double totE = 0.;
        for(int ih=0; ih<p_clus->getCluster().size(); ih++){
          const Cyber::Calo2DCluster* p_core = p_clus->getCluster()[ih];
          edm4hep::Vector3f pos(p_core->getPos().x(), p_core->getPos().y(), p_core->getPos().z());
          int _module = p_core->getTowerID()[0][0];
          int _layer = p_core->getDlayer();
          int _cellID = (_module<<5) + _layer;
   
          auto _hit = m_corehitCol->create();
          _hit.setCellID(_cellID);
          _hit.setEnergy( p_core->getEnergy()*settings.map_floatPars.at("ECALCalib") );
          _hit.setPosition( pos );
          _hit.setType(1); //Ecal barrel
          m_core.addToHits(_hit);
          totE += p_core->getEnergy()*settings.map_floatPars.at("ECALCalib");
        }
   
        double tmp_clusE = p_clus->getEnergy()*settings.map_floatPars.at("ECALCalib");
        TVector3 clus_pos = p_clus->getShowerCenter();
        edm4hep::Vector3f pos( clus_pos.x(), clus_pos.y(), clus_pos.z() );
        double tmp_phi = std::atan2(clus_pos.y(), clus_pos.x())* 180.0 / M_PI; //TODO: use TVector3 to calculate
        if (tmp_phi < 0) tmp_phi += 360.0;
        double tmp_theta = std::atan2(clus_pos.z(), clus_pos.Perp())* 180.0 / M_PI + 90;
        tmp_clusE = m_DataCol.EnergyCorrSvc->energyCorrection(tmp_clusE, tmp_phi, tmp_theta);
        totE = m_DataCol.EnergyCorrSvc->energyCorrection(totE, tmp_phi, tmp_theta);

        m_core.setEnergy(totE);
        m_core.setPosition( pos );
        m_clus.setEnergy( tmp_clusE );
        m_clus.setPosition( pos );
        m_clus.addToClusters(m_core);
        m_pfo.addToClusters( m_clus );

        EcalClusE += tmp_clusE;
        vec_Pos += tmp_clusE*p_clus->getShowerCenter();
      }


      //Write HCAL cluster
      double HcalClusE = 0;
      for(int ic=0; ic<vec_Hcalclus.size(); ic++){
        auto p_clus = vec_Hcalclus[ic];
        auto m_clus = m_Hcalcluster->create();
   
        for(int ih=0; ih<p_clus->getCaloHits().size(); ih++){
          const Cyber::CaloHit* p_hit = p_clus->getCaloHits()[ih];
          //auto _hit = m_hcalhitCol->create();
          auto _hit = p_hit->getOriginHit().clone();
          m_hcalhitCol->push_back(_hit);
          m_clus.addToHits(_hit);
        }
   
        double tmp_clusE = p_clus->getHitsE()*settings.map_floatPars.at("HCALCalib");
        m_clus.setEnergy( tmp_clusE );
        edm4hep::Vector3f pos( p_clus->getHitCenter().x(), p_clus->getHitCenter().y(), p_clus->getHitCenter().z() );
        m_clus.setPosition( pos );
        m_pfo.addToClusters( m_clus );

        HcalClusE += tmp_clusE;
        vec_Pos += tmp_clusE*p_clus->getHitCenter();
      }


      //Write Track
      if(vec_trks.size()==0){
        TVector3 p3vec = vec_Pos*(  (EcalClusE+HcalClusE)/vec_Pos.Mag() );
        edm4hep::Vector3f p3(p3vec.x(), p3vec.y(), p3vec.z());

        m_pfo.setEnergy( EcalClusE+HcalClusE );
        m_pfo.setCharge( 0 );
        m_pfo.setMomentum( p3 );
      }
      else{
        double trk_maxP = -99;
        int trkIndex = -1;
        for(int itrk=0; itrk<vec_trks.size(); itrk++){
          if( trk_maxP<vec_trks[itrk]->getMomentum() ){
            trk_maxP = vec_trks[itrk]->getMomentum();
            trkIndex = itrk;
          }
        }
        if(trkIndex>=0){
          auto m_trk = vec_trks[trkIndex]->getOriginTrack();
          if( !m_trk.isAvailable() || settings.map_boolPars.at("UseTruthTrk") )
            m_trk = TruthTrack( vec_trks[trkIndex]->getLeadingMCP(), m_trkCol );

          m_pfo.addToTracks( m_trk );
          m_pfo.setCharge( vec_trks[trkIndex]->getCharge() );

          TVector3 p3vec = vec_trks[trkIndex]->getP3();
          edm4hep::Vector3f p3(p3vec.x(), p3vec.y(), p3vec.z());
          m_pfo.setMomentum(p3);
          m_pfo.setEnergy( vec_trks[trkIndex]->getMomentum() );
        }
        else{
          TVector3 p3vec = vec_Pos*(  (EcalClusE+HcalClusE)/vec_Pos.Mag() );
          edm4hep::Vector3f p3(p3vec.x(), p3vec.y(), p3vec.z());

          m_pfo.setEnergy( EcalClusE+HcalClusE );
          m_pfo.setCharge( 0 );
          m_pfo.setMomentum( p3 );
        }

      }

    }
/*
double totE = 0;
for(int i=0; i<m_pfocol->size(); i++){
  auto m_pfo = m_pfocol->at(i);
  if(m_pfo.getCharge()!=0) continue;
   cout<<"    PFO #"<<i<<": track size "<<m_pfo.tracks_size()<<", cluster size "<<m_pfo.clusters_size()<<", energy "<<m_pfo.getEnergy()<<endl;
   totE += m_pfo.getEnergy();
}
cout<<"-----Neutral cluster total energy: "<<totE<<endl;
totE = 0;
for(int i=0; i<m_pfocol->size(); i++){
  auto m_pfo = m_pfocol->at(i);
  if(m_pfo.getCharge()==0) continue;
   cout<<"    PFO #"<<i<<": track size "<<m_pfo.tracks_size()<<", cluster size "<<m_pfo.clusters_size()<<", energy "<<m_pfo.getEnergy()<<endl;
   totE += m_pfo.getEnergy();
}
cout<<"-----Charged cluster Ecal total energy: "<<totE<<endl;

std::cout<<"  Created PFO size: "<<m_pfocol->size()<<std::endl;
*/
    return StatusCode::SUCCESS;
  }



  edm4hep::Track OutputCreator::TruthTrack(edm4hep::MCParticle _mcp, edm4hep::TrackCollection* _trkCol ){
    
    auto m_track = _trkCol->create();
    if( _mcp.getGeneratorStatus()!=1 || _mcp.getCharge()==0 ) return m_track;

    edm4hep::Vector3f mcp_vertex(_mcp.getVertex().x, _mcp.getVertex().y, _mcp.getVertex().z);
    edm4hep::Vector3f mcp_p(_mcp.getMomentum().x, _mcp.getMomentum().y, _mcp.getMomentum().z);
    double mcp_pT = TMath::Sqrt(mcp_p.x*mcp_p.x + mcp_p.y*mcp_p.y);
    double charge = _mcp.getCharge();

    //Assign the track state at IP
    edm4hep::TrackState m_trkst;
    m_trkst.location = 1;  // At IP
    m_trkst.D0 = 0;
    m_trkst.Z0 = 0;
    m_trkst.phi = TMath::ATan2(mcp_p.x, mcp_p.x);
    m_trkst.tanLambda = mcp_p.z / mcp_pT;
    m_trkst.omega = 0.3 * settings.map_floatPars.at("BField") / 1000. / mcp_pT;
    m_trkst.referencePoint = mcp_vertex;
    m_track.addToTrackStates(m_trkst);

    ////Use helix to calculate the track endpoint
    //HelixClassD * TrkInit_Helix = new HelixClassD();
    //TrkInit_Helix->Initialize_Canonical(m_trkst.phi, m_trkst.D0, m_trkst.Z0, m_trkst.omega, m_trkst.tanLambda, settings.map_floatPars["BField"]);
    //float intPoint[3];
    //float refPoint[3] = {mcp_vertex.x, mcp_vertex.y, mcp_vertex.z};
    //TrkInit_Helix->getPointOnCircle(Cyber::CaloUnit::ecal_innerR, refPoint, intPoint);
    ////If track reach to the edge of barrel
    //if(fabs(intPoint[2])>ECALHalfZ ){
    //  TrkInit_Helix->getPointInZ(ECALHalfZ, refPoint, intPoint);
    //}

    ////Assign the begin and end tracker hits
    //auto m_hit_IP = m_trkHitCol->create();
    //m_hit_IP.setPosition(edm4hep::Vector3d(0.,0.,0.));
    //m_track.addToTrackerHits(m_hit_IP);

    //auto m_hit_End = m_trkHitCol->create();
    //m_hit_End.setPosition(edm4hep::Vector3d(intPoint[0], intPoint[1], intPoint[2]));
    //m_track.addToTrackerHits(m_hit_End);

    //delete TrkInit_Helix;
    return m_track; 
  }

}




#endif
