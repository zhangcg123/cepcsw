#ifndef OUTPUT_CREATOR_C
#define OUTPUT_CREATOR_C

#include "Tools/OutputCreator.h"

namespace Cyber{

  OutputCreator::OutputCreator( const Settings& m_settings ): settings(m_settings){

  }

  StatusCode OutputCreator::CreatePFOSnapshot(
      const CyberDataCol& data, edm4hep::ReconstructedParticleCollection& pfos,
      edm4hep::ClusterCollection& ecal, edm4hep::CalorimeterHitCollection& ecalHits,
      edm4hep::ClusterCollection& hcal, edm4hep::CalorimeterHitCollection& hcalHits,
      edm4hep::TrackCollection& truthTracks) {
    const auto source = data.map_PFObjects.find(settings.map_stringPars.at("OutputPFO"));
    if (source == data.map_PFObjects.end()) return StatusCode::FAILURE;
    std::map<const Calo3DCluster*, edm4hep::Cluster> savedEcal, savedHcal;
    std::map<const CaloHit*, edm4hep::CalorimeterHit> savedEcalHits, savedHcalHits;
    std::map<const Track*, edm4hep::Track> savedTracks;

    auto saveCluster = [&](const Calo3DCluster* cluster, bool isEcal) {
      auto& saved = isEcal ? savedEcal : savedHcal;
      auto found = saved.find(cluster);
      if (found != saved.end()) return found->second;
      auto output = (isEcal ? ecal : hcal).create();
      const auto position = isEcal ? cluster->getShowerCenter() : cluster->getHitCenter();
      output.setEnergy(isEcal ? cluster->getLongiE() : cluster->getHitsE());
      output.setPosition({float(position.X()), float(position.Y()), float(position.Z())});
      auto view = cluster->Clone();
      // ECAL merges leave the cached hit list incomplete; do not mutate the source.
      if (isEcal) view->setCaloHitsFrom2DCluster();
      auto& savedHits = isEcal ? savedEcalHits : savedHcalHits;
      for (const auto* hit : view->getCaloHits()) {
        auto existing = savedHits.find(hit);
        if (existing == savedHits.end()) {
          auto copied = isEcal ? ecalHits.create() : hit->getOriginHit().clone();
          if (!isEcal) hcalHits.push_back(copied);
          const auto point = hit->getPosition();
          copied.setEnergy(hit->getEnergy());
          copied.setPosition({float(point.X()), float(point.Y()), float(point.Z())});
          if (isEcal) {
            copied.setCellID((hit->getModule() << 5) + hit->getLayer());
            copied.setType(hit->getSystem());
          }
          existing = savedHits.emplace(hit, copied).first;
        }
        output.addToHits(existing->second);
      }
      return saved.emplace(cluster, output).first->second;
    };

    for (const auto& pfo : source->second) {
      auto output = pfos.create();
      double neutralEnergy = 0.;
      TVector3 weightedPosition;
      for (bool isEcal : {true, false}) {
        const double calibration = settings.map_floatPars.at(
            isEcal ? "ECALNeutralCalib" : "HCALNeutralCalib");
        for (const auto* cluster : isEcal ? pfo->getECALClusters() : pfo->getHCALClusters()) {
          auto copied = saveCluster(cluster, isEcal);
          output.addToClusters(copied);
          const double energy = calibration *
              (isEcal ? cluster->getLongiE() : cluster->getHitsE());
          neutralEnergy += energy;
          weightedPosition += energy *
              (isEcal ? cluster->getShowerCenter() : cluster->getHitCenter());
        }
      }
      const Track* leading = nullptr;
      for (const auto* track : pfo->getTracks()) {
        auto found = savedTracks.find(track);
        if (found == savedTracks.end()) {
          auto original = track->getOriginTrack();
          if (!original.isAvailable() || settings.map_boolPars.at("UseTruthTrk") ||
              settings.map_boolPars.at("UseTruthMatchTrk"))
            original = TruthTrack(track->getLeadingMCP(), &truthTracks);
          found = savedTracks.emplace(track, original).first;
        }
        output.addToTracks(found->second);
        if (!leading || track->getMomentum() > leading->getMomentum()) leading = track;
      }
      if (leading) {
        // Same pre-final-PID charged kinematics convention as the standard exporter.
        const auto momentum = leading->getP3();
        output.setMomentum({float(momentum.X()), float(momentum.Y()), float(momentum.Z())});
        output.setCharge(leading->getCharge());
        output.setMass(0.139570);
        output.setEnergy(std::hypot(leading->getMomentum(), 0.139570));
        output.setType(pfo->getPID());
      } else {
        // Trackless PFObject::m_pid is not initialized by PFOCreating; leave type=0.
        output.setEnergy(neutralEnergy);
        if (weightedPosition.Mag() > 0.) {
          const auto momentum = weightedPosition.Unit() * neutralEnergy;
          output.setMomentum({float(momentum.X()), float(momentum.Y()), float(momentum.Z())});
        }
      }
    }
    return StatusCode::SUCCESS;
  }

  StatusCode OutputCreator::CreateClusterSnapshot(
      const CyberDataCol& data,
      const std::string& sourceName,
      DataHandle<edm4hep::ClusterCollection>& clusters,
      DataHandle<edm4hep::CalorimeterHitCollection>& hits) {
    auto* outputClusters = clusters.createAndPut();
    auto* outputHits = hits.createAndPut();
    const auto source = data.map_CaloCluster.find(sourceName);
    if (source == data.map_CaloCluster.end()) return StatusCode::SUCCESS;

    // No PFO-dependent calibration or track-momentum rescaling is applied here.
    for (const auto& cluster : source->second) {
      auto output = outputClusters->create();
      const auto position = cluster->getShowerCenter();
      output.setEnergy(cluster->getLongiE());
      output.setPosition({float(position.X()), float(position.Y()), float(position.Z())});
      // mergeCluster appends shower pieces without refreshing the cached 3D-hit list.
      auto view = cluster->Clone();
      view->setCaloHitsFrom2DCluster();
      for (const auto* hit : view->getCaloHits()) {
        auto copied = outputHits->create();
        const auto point = hit->getPosition();
        copied.setEnergy(hit->getEnergy());
        copied.setPosition({float(point.X()), float(point.Y()), float(point.Z())});
        // As in the existing fine-grained output, this is a display ID, not a DD4hep cell ID.
        copied.setCellID((hit->getModule() << 5) + hit->getLayer());
        copied.setType(hit->getSystem());
        output.addToHits(copied);
      }
    }
    return StatusCode::SUCCESS;
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
    if(settings.map_boolPars.at("UseTruthTrk") || settings.map_boolPars.at("UseTruthMatchTrk") ) m_trkCol = m_outTrkHandler.createAndPut();


    //PFO
    std::vector<std::shared_ptr<Cyber::PFObject>> p_pfos = m_DataCol.map_PFObjects[settings.map_stringPars.at("OutputPFO")];

    for(int ip=0; ip<p_pfos.size(); ip++){
      auto m_pfo = m_pfocol->create();
      std::vector<const Track*> vec_trks = p_pfos[ip]->getTracks();
      std::vector<const Calo3DCluster*> vec_Ecalclus = p_pfos[ip]->getECALClusters();
      std::vector<const Calo3DCluster*> vec_Hcalclus = p_pfos[ip]->getHCALClusters();

      double ecalcalib = 1.;
      double hcalcalib = vec_trks.size()==0 ? settings.map_floatPars.at("HCALNeutralCalib") : settings.map_floatPars.at("HCALChargedCalib");

      //TODO: this is a calibration based on PID. May need to move this step into the algorithms. 
      if(vec_trks.size()==0) ecalcalib = settings.map_floatPars.at("ECALNeutralCalib");
      else{
        if( p_pfos[ip]->getTrackMomentum() < 15. || p_pfos[ip]->getECALClusterEnergy()*ecalcalib/p_pfos[ip]->getTrackMomentum() < 0.8 ) ecalcalib = settings.map_floatPars.at("ECALChargedCalib");
        else ecalcalib = settings.map_floatPars.at("ECALNeutralCalib");
      }

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
          _hit.setEnergy( p_hit->getEnergy()*ecalcalib );
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
          _hit.setEnergy( p_core->getEnergy()*ecalcalib );
          _hit.setPosition( pos );
          _hit.setType(1); //Ecal barrel
          m_core.addToHits(_hit);
          totE += p_core->getEnergy()*ecalcalib;
        }
   
        double tmp_clusE = p_clus->getLongiE()*ecalcalib;
        TVector3 clus_pos = p_clus->getShowerCenter();
        edm4hep::Vector3f pos( clus_pos.x(), clus_pos.y(), clus_pos.z() );
        //double tmp_phi = std::atan2(clus_pos.y(), clus_pos.x())* 180.0 / M_PI; //TODO: use TVector3 to calculate
        //if (tmp_phi < 0) tmp_phi += 360.0;
        //double tmp_theta = std::atan2(clus_pos.z(), clus_pos.Perp())* 180.0 / M_PI + 90;
        //tmp_clusE = m_DataCol.EnergyCorrSvc->energyCorrection(tmp_clusE, tmp_phi, tmp_theta);
        //totE = m_DataCol.EnergyCorrSvc->energyCorrection(totE, tmp_phi, tmp_theta);

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
   
        double tmp_clusE = p_clus->getHitsE()*hcalcalib;
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
          if( !m_trk.isAvailable() || settings.map_boolPars.at("UseTruthTrk") || settings.map_boolPars.at("UseTruthMatchTrk"))
            m_trk = TruthTrack( vec_trks[trkIndex]->getLeadingMCP(), m_trkCol );

          m_pfo.addToTracks( m_trk );
          m_pfo.setCharge( vec_trks[trkIndex]->getCharge() );

          TVector3 p3vec = vec_trks[trkIndex]->getP3();
          edm4hep::Vector3f p3(p3vec.x(), p3vec.y(), p3vec.z());
          m_pfo.setMomentum(p3);
          m_pfo.setMass(0.139570); //TODO: all charged particles are set to pion mass. 
          m_pfo.setEnergy( sqrt(vec_trks[trkIndex]->getMomentum()*vec_trks[trkIndex]->getMomentum() + 0.139570*0.139570) );
        }
        else{
          TVector3 p3vec = vec_Pos*(  (EcalClusE+HcalClusE)/vec_Pos.Mag() );
          edm4hep::Vector3f p3(p3vec.x(), p3vec.y(), p3vec.z());

          m_pfo.setEnergy( EcalClusE+HcalClusE );
          m_pfo.setCharge( 0 );
          m_pfo.setMomentum( p3 );
        }

      }
//printf("  Create PFO #%d: charge %.1f, p4 (%.7f, %.7f, %.7f, %.7f), mass %.3f \n", ip, m_pfo.getCharge(), m_pfo.getMomentum().x, m_pfo.getMomentum().y, m_pfo.getMomentum().z, m_pfo.getEnergy(), m_pfo.getMass() );
    }
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
    m_trkst.omega = 0.299792458 * settings.map_floatPars.at("BField") / 1000. / mcp_pT;
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
