#ifndef READ_DIGI_C
#define READ_DIGI_C

#include "ReadDigiAlg.h"

DECLARE_COMPONENT(ReadDigiAlg)

ReadDigiAlg::ReadDigiAlg(const std::string& name, ISvcLocator* svcLoc)
    : Algorithm(name, svcLoc), 
     _nEvt(0)
{
  declareProperty("MCParticle",  m_MCParticleCol, "MCParticle collection (input)");

  //Tracker hit
  //declareProperty("SITRawHits", m_SITRawColHdl, "SIT Raw Hit Collection of SpacePoints");
  //declareProperty("SETRawHits", m_SETRawColHdl, "SET Raw Hit Collection of SpacePoints");
  //declareProperty("FTDRawHits", m_FTDRawColHdl, "FTD Raw Hit Collection of SpacePoints");
  //declareProperty("VTXTrackerHits", m_VTXTrackerHitColHdl, "VTX Hit Collection");
  //declareProperty("SITTrackerHits", m_SITTrackerHitColHdl, "SIT Hit Collection");
  //declareProperty("SETTrackerHits", m_SETTrackerHitColHdl, "SET Hit Collection");
  //declareProperty("TPCTrackerHits", m_TPCTrackerHitColHdl, "TPC Hit Collection");
  //declareProperty("FTDSpacePoints", m_FTDSpacePointColHdl, "FTD FTDSpacePoint Collection");
  //declareProperty("FTDPixelTrackerHits", m_FTDPixelTrackerHitColHdl, "handler of FTD Pixel Hit Collection");

  //Track
  declareProperty("FullTracks", m_fullTrk, "Full Track Collection");
  declareProperty("TPCTracks", m_TPCTrk, "TPC Track Collection");
  declareProperty("SiTracks", m_SiTrk, "Si Track Collection");

  declareProperty("TPCTracksAssociation", m_TPCTrkAssoHdl, "TPC Track - MCParticle Collection");
  declareProperty("FullTracksAssociation", m_fullTrkAssoHdl, "Full Track - MCParticle Collection");

  //declareProperty("EcalBarrelCollection",  m_ECalBarrelHitCol, "ECal Barrel");
  //declareProperty("EcalEndcapsCollection", m_ECalEndcapHitCol, "ECal Endcap");
  //declareProperty("HcalBarrelCollection",  m_HCalBarrelHitCol, "HCal Barrel");
  //declareProperty("HcalEndcapsCollection", m_HCalEndcapHitCol, "HCal Endcap");

}

StatusCode ReadDigiAlg::initialize()
{
  debug() << "begin initialize ReadDigiAlg" << endmsg;
  std::string s_outfile = _filename;
  m_wfile = new TFile(s_outfile.c_str(), "recreate");
  m_mctree = new TTree("MCParticle", "MCParticle");
  m_sitrktree = new TTree("RecoSiTrk", "RecoSiTrk");
  m_tpctrktree = new TTree("RecoTPCTrk", "RecoTPCTrk");
  m_fulltrktree = new TTree("RecoFullTrk", "RecoFullTrk");

  m_mctree->Branch("N_MCP", &N_MCP);
  m_mctree->Branch("MCP_px", &MCP_px);
  m_mctree->Branch("MCP_py", &MCP_py);
  m_mctree->Branch("MCP_pz", &MCP_pz);
  m_mctree->Branch("MCP_E", &MCP_E);
  m_mctree->Branch("MCP_VTX_x", &MCP_VTX_x);
  m_mctree->Branch("MCP_VTX_y", &MCP_VTX_y);
  m_mctree->Branch("MCP_VTX_z", &MCP_VTX_z);
  m_mctree->Branch("MCP_endPoint_x", &MCP_endPoint_x);
  m_mctree->Branch("MCP_endPoint_y", &MCP_endPoint_y);
  m_mctree->Branch("MCP_endPoint_z", &MCP_endPoint_z);
  m_mctree->Branch("MCP_pdgid", &MCP_pdgid);
  m_mctree->Branch("MCP_gStatus", &MCP_gStatus);

  //m_trktree->Branch("N_VTXhit", &N_VTXhit);
  //m_trktree->Branch("N_SIThit", &N_SIThit);
  //m_trktree->Branch("N_TPChit", &N_TPChit);
  //m_trktree->Branch("N_SEThit", &N_SEThit);
  //m_trktree->Branch("N_FTDhit", &N_FTDhit);
  //m_trktree->Branch("N_SITrawhit", &N_SITrawhit);
  //m_trktree->Branch("N_SETrawhit", &N_SETrawhit);
  m_sitrktree->Branch("N_SiTrk", &N_SiTrk);
  m_sitrktree->Branch("N_TPCTrk", &N_TPCTrk);
  m_sitrktree->Branch("N_fullTrk", &N_fullTrk);
  m_sitrktree->Branch("trk_px", &m_trk_px);
  m_sitrktree->Branch("trk_py", &m_trk_py);
  m_sitrktree->Branch("trk_pz", &m_trk_pz);
  m_sitrktree->Branch("trk_p", &m_trk_p);
  m_sitrktree->Branch("trk_Nhit", &m_trk_Nhit);
  m_sitrktree->Branch("trkstate_d0", &m_trkstate_d0 );
  m_sitrktree->Branch("trkstate_z0", &m_trkstate_z0 );
  m_sitrktree->Branch("trkstate_phi", &m_trkstate_phi );
  m_sitrktree->Branch("trkstate_tanL", &m_trkstate_tanL );
  m_sitrktree->Branch("trkstate_omega", &m_trkstate_omega );
  m_sitrktree->Branch("trkstate_kappa", &m_trkstate_kappa );
  m_sitrktree->Branch("trkstate_refx", &m_trkstate_refx );
  m_sitrktree->Branch("trkstate_refy", &m_trkstate_refy );
  m_sitrktree->Branch("trkstate_refz", &m_trkstate_refz );
  m_sitrktree->Branch("trkstate_tag", &m_trkstate_tag );
  m_sitrktree->Branch("trkstate_location", &m_trkstate_location );
  m_sitrktree->Branch("truthMC_tag", &m_truthMC_tag);
  m_sitrktree->Branch("truthMC_pid", &m_truthMC_pid);
  m_sitrktree->Branch("truthMC_px", &m_truthMC_px);
  m_sitrktree->Branch("truthMC_py", &m_truthMC_py);
  m_sitrktree->Branch("truthMC_pz", &m_truthMC_pz);
  m_sitrktree->Branch("truthMC_E", &m_truthMC_E);
  m_sitrktree->Branch("truthMC_EPx", &m_truthMC_EPx);
  m_sitrktree->Branch("truthMC_EPy", &m_truthMC_EPy);
  m_sitrktree->Branch("truthMC_EPz", &m_truthMC_EPz);
  m_sitrktree->Branch("truthMC_weight", &m_truthMC_weight);

  m_tpctrktree->Branch("N_SiTrk", &N_SiTrk);
  m_tpctrktree->Branch("N_TPCTrk", &N_TPCTrk);
  m_tpctrktree->Branch("N_fullTrk", &N_fullTrk);
  m_tpctrktree->Branch("trk_px", &m_trk_px);
  m_tpctrktree->Branch("trk_py", &m_trk_py);
  m_tpctrktree->Branch("trk_pz", &m_trk_pz);
  m_tpctrktree->Branch("trk_p", &m_trk_p);
  m_tpctrktree->Branch("trk_Nhit", &m_trk_Nhit);
  m_tpctrktree->Branch("trkstate_d0", &m_trkstate_d0 );
  m_tpctrktree->Branch("trkstate_z0", &m_trkstate_z0 );
  m_tpctrktree->Branch("trkstate_phi", &m_trkstate_phi );
  m_tpctrktree->Branch("trkstate_tanL", &m_trkstate_tanL );
  m_tpctrktree->Branch("trkstate_omega", &m_trkstate_omega );
  m_tpctrktree->Branch("trkstate_kappa", &m_trkstate_kappa );
  m_tpctrktree->Branch("trkstate_refx", &m_trkstate_refx );
  m_tpctrktree->Branch("trkstate_refy", &m_trkstate_refy );
  m_tpctrktree->Branch("trkstate_refz", &m_trkstate_refz );
  m_tpctrktree->Branch("trkstate_tag", &m_trkstate_tag );
  m_tpctrktree->Branch("trkstate_location", &m_trkstate_location );
  m_tpctrktree->Branch("truthMC_tag", &m_truthMC_tag);
  m_tpctrktree->Branch("truthMC_pid", &m_truthMC_pid);
  m_tpctrktree->Branch("truthMC_px", &m_truthMC_px);
  m_tpctrktree->Branch("truthMC_py", &m_truthMC_py);
  m_tpctrktree->Branch("truthMC_pz", &m_truthMC_pz);
  m_tpctrktree->Branch("truthMC_E", &m_truthMC_E);
  m_tpctrktree->Branch("truthMC_EPx", &m_truthMC_EPx);
  m_tpctrktree->Branch("truthMC_EPy", &m_truthMC_EPy);
  m_tpctrktree->Branch("truthMC_EPz", &m_truthMC_EPz);
  m_tpctrktree->Branch("truthMC_weight", &m_truthMC_weight);

  m_fulltrktree->Branch("N_SiTrk", &N_SiTrk);
  m_fulltrktree->Branch("N_TPCTrk", &N_TPCTrk);
  m_fulltrktree->Branch("N_fullTrk", &N_fullTrk);
  m_fulltrktree->Branch("trk_px", &m_trk_px);
  m_fulltrktree->Branch("trk_py", &m_trk_py);
  m_fulltrktree->Branch("trk_pz", &m_trk_pz);
  m_fulltrktree->Branch("trk_p", &m_trk_p);
  m_fulltrktree->Branch("trk_Nhit", &m_trk_Nhit);
  m_fulltrktree->Branch("trk_type", &m_trk_type);
  m_fulltrktree->Branch("trkstate_d0", &m_trkstate_d0 );
  m_fulltrktree->Branch("trkstate_z0", &m_trkstate_z0 );
  m_fulltrktree->Branch("trkstate_phi", &m_trkstate_phi );
  m_fulltrktree->Branch("trkstate_tanL", &m_trkstate_tanL );
  m_fulltrktree->Branch("trkstate_omega", &m_trkstate_omega );
  m_fulltrktree->Branch("trkstate_kappa", &m_trkstate_kappa );
  m_fulltrktree->Branch("trkstate_refx", &m_trkstate_refx );
  m_fulltrktree->Branch("trkstate_refy", &m_trkstate_refy );
  m_fulltrktree->Branch("trkstate_refz", &m_trkstate_refz );
  m_fulltrktree->Branch("trkstate_tag", &m_trkstate_tag );
  m_fulltrktree->Branch("trkstate_location", &m_trkstate_location );
  m_fulltrktree->Branch("truthMC_tag", &m_truthMC_tag);
  m_fulltrktree->Branch("truthMC_pid", &m_truthMC_pid);
  m_fulltrktree->Branch("truthMC_px", &m_truthMC_px);
  m_fulltrktree->Branch("truthMC_py", &m_truthMC_py);
  m_fulltrktree->Branch("truthMC_pz", &m_truthMC_pz);
  m_fulltrktree->Branch("truthMC_E", &m_truthMC_E);
  m_fulltrktree->Branch("truthMC_EPx", &m_truthMC_EPx);
  m_fulltrktree->Branch("truthMC_EPy", &m_truthMC_EPy);
  m_fulltrktree->Branch("truthMC_EPz", &m_truthMC_EPz);
  m_fulltrktree->Branch("truthMC_weight", &m_truthMC_weight);

  return Algorithm::initialize();
}

StatusCode ReadDigiAlg::execute()
{
  if(_nEvt==0) std::cout<<"ReadDigiAlg::execute Start"<<std::endl;
  debug() << "Processing event: "<<_nEvt<< endmsg; 

  Clear(); 

  try{
  const edm4hep::MCParticleCollection* const_MCPCol = m_MCParticleCol.get();
  if(const_MCPCol){
    N_MCP = const_MCPCol->size(); 
    for(int i=0; i<N_MCP; i++){
      edm4hep::MCParticle m_MCp = const_MCPCol->at(i);
      MCP_px.push_back(m_MCp.getMomentum().x);
      MCP_py.push_back(m_MCp.getMomentum().y);
      MCP_pz.push_back(m_MCp.getMomentum().z);
      MCP_E.push_back(m_MCp.getEnergy());
      MCP_VTX_x.push_back(m_MCp.getVertex().x);
      MCP_VTX_y.push_back(m_MCp.getVertex().y);
      MCP_VTX_z.push_back(m_MCp.getVertex().z);
      MCP_endPoint_x.push_back(m_MCp.getEndpoint().x); 
      MCP_endPoint_y.push_back(m_MCp.getEndpoint().y); 
      MCP_endPoint_z.push_back(m_MCp.getEndpoint().z); 
      MCP_pdgid.push_back(m_MCp.getPDG());
      MCP_gStatus.push_back(m_MCp.getGeneratorStatus());
    }
  }
  }catch(GaudiException &e){
    debug()<<"MC Particle is not available "<<endmsg;
  }
  m_mctree->Fill(); 

  //try{
  //if(m_VTXTrackerHitColHdl.get())
  //  N_VTXhit = m_VTXTrackerHitColHdl.get()->size();
  //}catch(GaudiException &e){
  //  debug()<<"VTX hit is not available "<<endmsg;
  //}

  //try{
  //if(m_SITTrackerHitColHdl.get())
  //  N_SIThit = m_SITTrackerHitColHdl.get()->size();
  //}catch(GaudiException &e){
  //  debug()<<"SIT hit is not available "<<endmsg;
  //}

  //try{
  //if(m_TPCTrackerHitColHdl.get())
  //  N_TPChit = m_TPCTrackerHitColHdl.get()->size();
  //}catch(GaudiException &e){
  //  debug()<<"TPC hit is not available "<<endmsg;
  //}

  //try{
  //if(m_SETTrackerHitColHdl.get())
  //  N_SEThit = m_SETTrackerHitColHdl.get()->size();
  //}catch(GaudiException &e){
  //  debug()<<"SET hit is not available "<<endmsg;
  //}

  //try{
  //if(m_FTDSpacePointColHdl.get())
  //  N_FTDhit = m_FTDSpacePointColHdl.get()->size();
  //}catch(GaudiException &e){
  //  debug()<<"FDT space point is not available "<<endmsg;
  //}

  //try{
  //if(m_SITRawColHdl.get())
  //  N_SITrawhit = m_SITRawColHdl.get()->size();
  //}catch(GaudiException &e){
  //  debug()<<"SIT raw hit is not available "<<endmsg;
  //}

  //try{
  //if(m_SETRawColHdl.get())
  //  N_SETrawhit = m_SETRawColHdl.get()->size();
  //}catch(GaudiException &e){
  //  debug()<<"SET raw hit is not available "<<endmsg;
  //}

  Clear(); 
  try{
  auto const_SiTrkCol = m_SiTrk.get();
  if(const_SiTrkCol){
    N_SiTrk = const_SiTrkCol->size();
    for(int i=0; i<N_SiTrk; i++){
      auto m_trk = const_SiTrkCol->at(i);
      if(m_trk.trackStates_size()==0) continue;
      if(m_trk.trackerHits_size()==0) continue;

      for(int istat=0; istat<m_trk.trackStates_size(); istat++){
        edm4hep::TrackState m_trkstate = m_trk.getTrackStates(istat);

        m_trkstate_d0.push_back( m_trkstate.D0 );
        m_trkstate_z0.push_back( m_trkstate.Z0 );
        m_trkstate_phi.push_back( m_trkstate.phi );
        m_trkstate_tanL.push_back( m_trkstate.tanLambda );
        //m_trkstate_kappa.push_back( m_trkstate.Kappa);
        m_trkstate_omega.push_back( m_trkstate.omega );
        m_trkstate_refx.push_back( m_trkstate.referencePoint.x );
        m_trkstate_refy.push_back( m_trkstate.referencePoint.y );
        m_trkstate_refz.push_back( m_trkstate.referencePoint.z );
        m_trkstate_location.push_back( m_trkstate.location );
        m_trkstate_tag.push_back(i);
      }

      edm4hep::TrackState m_trkstate = m_trk.getTrackStates(0);

      HelixClassD * TrkInit_Helix = new HelixClassD();
      TrkInit_Helix->Initialize_Canonical(m_trkstate.phi, m_trkstate.D0, m_trkstate.Z0, m_trkstate.omega, m_trkstate.tanLambda, _Bfield);
      TVector3 TrkMom(TrkInit_Helix->getMomentum()[0],TrkInit_Helix->getMomentum()[1],TrkInit_Helix->getMomentum()[2]);

      int NTrkHit = m_trk.trackerHits_size();
      m_trk_Nhit.push_back(NTrkHit);
      m_trk_px.push_back(TrkMom.x());
      m_trk_py.push_back(TrkMom.y());
      m_trk_pz.push_back(TrkMom.z());
      m_trk_p.push_back(TrkMom.Mag());

      delete TrkInit_Helix;
    }
  }
  }catch(GaudiException &e){
    debug()<<"Si track is not available "<<endmsg;
  }  
  m_sitrktree->Fill();

  Clear(); 
  try{
  auto const_TPCTrkCol = m_TPCTrk.get();
  auto const_TPCTrkLinkCol = m_TPCTrkAssoHdl.get();
  if(const_TPCTrkCol){
    N_TPCTrk = const_TPCTrkCol->size();
    for(int i=0; i<N_TPCTrk; i++){
      auto m_trk = const_TPCTrkCol->at(i);
      if(m_trk.trackStates_size()==0) continue;
      if(m_trk.trackerHits_size()==0) continue;
      int NTrkHit = m_trk.trackerHits_size();

      for(int istat=0; istat<m_trk.trackStates_size(); istat++){
        edm4hep::TrackState m_trkstate = m_trk.getTrackStates(istat);

        m_trkstate_d0.push_back( m_trkstate.D0 );
        m_trkstate_z0.push_back( m_trkstate.Z0 );
        m_trkstate_phi.push_back( m_trkstate.phi );
        m_trkstate_tanL.push_back( m_trkstate.tanLambda );
        //m_trkstate_kappa.push_back( m_trkstate.Kappa);
        m_trkstate_omega.push_back( m_trkstate.omega );
        m_trkstate_refx.push_back( m_trkstate.referencePoint.x );
        m_trkstate_refy.push_back( m_trkstate.referencePoint.y );
        m_trkstate_refz.push_back( m_trkstate.referencePoint.z );
        m_trkstate_location.push_back( m_trkstate.location );
        m_trkstate_tag.push_back(i);
      }

      if(const_TPCTrkLinkCol){
        for(auto iter: *const_TPCTrkLinkCol){
          if(iter.getRec()==m_trk){
            edm4hep::MCParticle m_mcp = iter.getSim();
            m_truthMC_pid.push_back(m_mcp.getPDG());
            m_truthMC_px.push_back(m_mcp.getMomentum().x);
            m_truthMC_py.push_back(m_mcp.getMomentum().y);
            m_truthMC_pz.push_back(m_mcp.getMomentum().z);
            m_truthMC_E.push_back(m_mcp.getEnergy());
            m_truthMC_EPx.push_back(m_mcp.getEndpoint().x);
            m_truthMC_EPy.push_back(m_mcp.getEndpoint().y);
            m_truthMC_EPz.push_back(m_mcp.getEndpoint().z);
            m_truthMC_weight.push_back(iter.getWeight()/NTrkHit);
            m_truthMC_tag.push_back(i);
          }
        }
      }

      edm4hep::TrackState m_trkstate = m_trk.getTrackStates(0);

      HelixClassD * TrkInit_Helix = new HelixClassD();
      TrkInit_Helix->Initialize_Canonical(m_trkstate.phi, m_trkstate.D0, m_trkstate.Z0, m_trkstate.omega, m_trkstate.tanLambda, _Bfield);
      TVector3 TrkMom(TrkInit_Helix->getMomentum()[0],TrkInit_Helix->getMomentum()[1],TrkInit_Helix->getMomentum()[2]);

      m_trk_Nhit.push_back(NTrkHit);
      m_trk_px.push_back(TrkMom.x());
      m_trk_py.push_back(TrkMom.y());
      m_trk_pz.push_back(TrkMom.z());
      m_trk_p.push_back(TrkMom.Mag());

      delete TrkInit_Helix;
    }
  }
  }catch(GaudiException &e){
    debug()<<"TPC track is not available "<<endmsg;
  }
  m_tpctrktree->Fill();


  Clear(); 
  try{
  auto const_fullTrkCol = m_fullTrk.get();
  auto const_fullTrkLinkCol = m_fullTrkAssoHdl.get();
  if(const_fullTrkCol){
    N_fullTrk = const_fullTrkCol->size();
    for(int i=0; i<N_fullTrk; i++){
      auto m_trk = const_fullTrkCol->at(i);
//cout<<"In track #"<<i<<": track state size "<<m_trk.trackStates_size()<<", track hit size "<<m_trk.trackerHits_size()<<endl;
      if(m_trk.trackStates_size()==0) continue;
      //if(m_trk.trackerHits_size()==0) continue;
      int NTrkHit = m_trk.trackerHits_size();

      for(int istat=0; istat<m_trk.trackStates_size(); istat++){
        edm4hep::TrackState m_trkstate = m_trk.getTrackStates(istat);

        m_trkstate_d0.push_back( m_trkstate.D0 );
        m_trkstate_z0.push_back( m_trkstate.Z0 );
        m_trkstate_phi.push_back( m_trkstate.phi );
        m_trkstate_tanL.push_back( m_trkstate.tanLambda );
        //m_trkstate_kappa.push_back( m_trkstate.Kappa);
        m_trkstate_omega.push_back( m_trkstate.omega );
        m_trkstate_refx.push_back( m_trkstate.referencePoint.x );
        m_trkstate_refy.push_back( m_trkstate.referencePoint.y );
        m_trkstate_refz.push_back( m_trkstate.referencePoint.z );
        m_trkstate_location.push_back( m_trkstate.location );
        m_trkstate_tag.push_back(i);
      }

      int NSihit = 0;
      int NTPChit = 0;
      int Nelse = 0;
      for(int ihit=0; ihit<NTrkHit; ihit++){
        auto m_hit = m_trk.getTrackerHits(ihit);
        if(m_hit.getType()==8) NSihit++;
        else if(m_hit.getType()==0) NTPChit++;
        else Nelse++;
      }
      //cout<<"In track #"<<i<<": Nhit "<<NTrkHit<<", Si Hit "<<NSihit<<", TPC hit "<<NTPChit<<", else "<<Nelse<<endl;

      if(const_fullTrkLinkCol){
        for(auto iter: *const_fullTrkLinkCol){
          if(iter.getRec()==m_trk){
            edm4hep::MCParticle m_mcp = iter.getSim();
            m_truthMC_pid.push_back(m_mcp.getPDG());
            m_truthMC_px.push_back(m_mcp.getMomentum().x);
            m_truthMC_py.push_back(m_mcp.getMomentum().y);
            m_truthMC_pz.push_back(m_mcp.getMomentum().z);
            m_truthMC_E.push_back(m_mcp.getEnergy());
            m_truthMC_EPx.push_back(m_mcp.getEndpoint().x);
            m_truthMC_EPy.push_back(m_mcp.getEndpoint().y);
            m_truthMC_EPz.push_back(m_mcp.getEndpoint().z);
            m_truthMC_weight.push_back(iter.getWeight()/NTrkHit);
            m_truthMC_tag.push_back(i);
          }
        }
      }

      edm4hep::TrackState m_trkstate = m_trk.getTrackStates(0);
      if(m_trkstate.location!=1){
        std::cout<<"ERROR: first track state is not IP! Will use this for track momentum "<<std::endl;
      }

      double m_pt = (0.299792458*_Bfield)/fabs(m_trkstate.omega*1000.);
      double m_phi = m_trkstate.phi;
      double m_pz = m_trkstate.tanLambda * m_pt;
      TVector3 TrkMom(m_pt*cos(m_phi), m_pt*sin(m_phi), m_pz);
      //HelixClassD * TrkInit_Helix = new HelixClassD();
      //TrkInit_Helix->Initialize_Canonical(m_trkstate.phi, m_trkstate.D0, m_trkstate.Z0, m_trkstate.omega, m_trkstate.tanLambda, _Bfield);
      //TVector3 TrkMom(TrkInit_Helix->getMomentum()[0],TrkInit_Helix->getMomentum()[1],TrkInit_Helix->getMomentum()[2]);

      m_trk_Nhit.push_back(NTrkHit);
      m_trk_type.push_back(m_trk.getType());
      m_trk_px.push_back(TrkMom.x());
      m_trk_py.push_back(TrkMom.y());
      m_trk_pz.push_back(TrkMom.z());
      m_trk_p.push_back(TrkMom.Mag());

      //delete TrkInit_Helix;
    }
  }
  }catch(GaudiException &e){
    debug()<<"Full track is not available "<<endmsg;
  }
  m_fulltrktree->Fill();

/*  const edm4hep::SimCalorimeterHitCollection* ECalBHitCol = m_ECalBarrelHitCol.get();
  Nhit_EcalB = ECalBHitCol->size();
  for(int i=0; i<ECalBHitCol->size(); i++){
    edm4hep::SimCalorimeterHit CaloHit = ECalBHitCol->at(i);

    CaloHit_type.push_back(0);
    CaloHit_x.push_back(CaloHit.getPosition().x);
    CaloHit_y.push_back(CaloHit.getPosition().y);
    CaloHit_z.push_back(CaloHit.getPosition().z);
    CaloHit_E.push_back(CaloHit.getEnergy());
    CaloHit_Eem.push_back(CaloHit.getEnergy());
    CaloHit_Ehad.push_back(0.);

    Etot += CaloHit.getEnergy();
    Etot_ecal += CaloHit.getEnergy();
  }
*/

/*  const edm4hep::SimCalorimeterHitCollection* ECalEHitCol = m_ECalEndcapHitCol.get();
  Nhit_EcalE = ECalEHitCol->size();
  for(int i=0; i<ECalEHitCol->size(); i++){
    edm4hep::SimCalorimeterHit CaloHit = ECalEHitCol->at(i);

    CaloHit_type.push_back(1);
    CaloHit_x.push_back(CaloHit.getPosition().x);
    CaloHit_y.push_back(CaloHit.getPosition().y);
    CaloHit_z.push_back(CaloHit.getPosition().z);
    CaloHit_E.push_back(CaloHit.getEnergy());
    CaloHit_Eem.push_back(CaloHit.getEnergy());
    CaloHit_Ehad.push_back(0.);
  }
*/
/*  const edm4hep::SimCalorimeterHitCollection* HCalBHitCol = m_HCalBarrelHitCol.get();
  Nhit_HcalB = HCalBHitCol->size();
  for(int i=0; i<HCalBHitCol->size(); i++){
    edm4hep::SimCalorimeterHit CaloHit = HCalBHitCol->at(i);
    //if(CaloHit.getEnergy()<0.0001) continue;

    CaloHit_x.push_back(CaloHit.getPosition().x);
    CaloHit_y.push_back(CaloHit.getPosition().y);
    CaloHit_z.push_back(CaloHit.getPosition().z);
    CaloHit_E.push_back(CaloHit.getEnergy());

    int Nconb = 0;
    double Econb = 0.;
    for(int iCont=0; iCont < CaloHit.contributions_size(); ++iCont){
      auto conb = CaloHit.getContributions(iCont);
      float conb_En = conb.getEnergy();
      if( !conb.isAvailable() ) continue;
      if(conb_En == 0) continue;

      Nconb++;
      Econb += conb_En;
    }

    Etot += CaloHit.getEnergy();
  }
*/

  _nEvt++; 
  return StatusCode::SUCCESS;
}

StatusCode ReadDigiAlg::finalize()
{
  debug() << "begin finalize ReadDigiAlg" << endmsg;
  m_wfile->cd();
  m_mctree->Write();
  m_sitrktree->Write();
  m_tpctrktree->Write();
  m_fulltrktree->Write();
  m_wfile->Close(); 

  return Algorithm::finalize();
}

StatusCode ReadDigiAlg::Clear()
{

  N_MCP = -99;
  MCP_px.clear();
  MCP_py.clear();
  MCP_pz.clear();
  MCP_E.clear();
  MCP_VTX_x.clear();
  MCP_VTX_y.clear();
  MCP_VTX_z.clear();
  MCP_endPoint_x.clear();
  MCP_endPoint_y.clear();
  MCP_endPoint_z.clear();
  MCP_pdgid.clear();
  MCP_gStatus.clear(); 

  N_SiTrk = -99;
  N_TPCTrk = -99;
  N_fullTrk = -99;
  //N_VTXhit = -99; 
  //N_SIThit = -99; 
  //N_TPChit = -99; 
  //N_SEThit = -99; 
  //N_FTDhit = -99; 
  //N_SITrawhit = -99;
  //N_SETrawhit = -99;
  m_trk_px.clear(); 
  m_trk_py.clear(); 
  m_trk_pz.clear();
  m_trk_p.clear(); 
  m_trk_Nhit.clear();
  m_trk_type.clear();
  m_trkstate_d0.clear();
  m_trkstate_z0.clear();
  m_trkstate_phi.clear();
  m_trkstate_tanL.clear();
  m_trkstate_omega.clear();
  m_trkstate_kappa.clear();
  m_trkstate_refx.clear();
  m_trkstate_refy.clear();
  m_trkstate_refz.clear();
  m_trkstate_tag.clear();
  m_trkstate_location.clear();
  m_truthMC_tag.clear();
  m_truthMC_pid.clear();
  m_truthMC_px.clear();
  m_truthMC_py.clear();
  m_truthMC_pz.clear();
  m_truthMC_E.clear();
  m_truthMC_EPx.clear();
  m_truthMC_EPy.clear();
  m_truthMC_EPz.clear();
  m_truthMC_weight.clear();

  return StatusCode::SUCCESS;
}

#endif
