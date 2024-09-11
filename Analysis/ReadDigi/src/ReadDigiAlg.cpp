#ifndef READ_DIGI_C
#define READ_DIGI_C

#include "ReadDigiAlg.h"

DECLARE_COMPONENT(ReadDigiAlg)

ReadDigiAlg::ReadDigiAlg(const std::string& name, ISvcLocator* svcLoc)
    : GaudiAlgorithm(name, svcLoc), 
     _nEvt(0)
{
  declareProperty("MCParticle",  m_MCParticleCol, "MCParticle collection (input)");

  //Tracker hit
  declareProperty("SITRawHits", m_SITRawColHdl, "SIT Raw Hit Collection of SpacePoints");
  declareProperty("SETRawHits", m_SETRawColHdl, "SET Raw Hit Collection of SpacePoints");
  declareProperty("FTDRawHits", m_FTDRawColHdl, "FTD Raw Hit Collection of SpacePoints");
  declareProperty("VTXTrackerHits", m_VTXTrackerHitColHdl, "VTX Hit Collection");
  declareProperty("SITTrackerHits", m_SITTrackerHitColHdl, "SIT Hit Collection");
  declareProperty("SETTrackerHits", m_SETTrackerHitColHdl, "SET Hit Collection");
  declareProperty("TPCTrackerHits", m_TPCTrackerHitColHdl, "TPC Hit Collection");
  declareProperty("FTDSpacePoints", m_FTDSpacePointColHdl, "FTD FTDSpacePoint Collection");
  declareProperty("FTDPixelTrackerHits", m_FTDPixelTrackerHitColHdl, "handler of FTD Pixel Hit Collection");

  //Track
  declareProperty("FullTracks", m_fullTrk, "Full Track Collection");
  declareProperty("TPCTracks", m_TPCTrk, "TPC Track Collection");
  declareProperty("SiTracks", m_SiTrk, "Si Track Collection");

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
  m_trktree = new TTree("RecoTrk", "RecoTrk");

  m_mctree->Branch("N_MCP", &N_MCP);
  m_mctree->Branch("MCP_px", &MCP_px);
  m_mctree->Branch("MCP_py", &MCP_py);
  m_mctree->Branch("MCP_pz", &MCP_pz);
  m_mctree->Branch("MCP_E", &MCP_E);
  m_mctree->Branch("MCP_endPoint_x", &MCP_endPoint_x);
  m_mctree->Branch("MCP_endPoint_y", &MCP_endPoint_y);
  m_mctree->Branch("MCP_endPoint_z", &MCP_endPoint_z);
  m_mctree->Branch("MCP_pdgid", &MCP_pdgid);
  m_mctree->Branch("MCP_gStatus", &MCP_gStatus);

  m_trktree->Branch("N_SiTrk", &N_SiTrk);
  m_trktree->Branch("N_TPCTrk", &N_TPCTrk);
  m_trktree->Branch("N_fullTrk", &N_fullTrk);
  m_trktree->Branch("N_VTXhit", &N_VTXhit);
  m_trktree->Branch("N_SIThit", &N_SIThit);
  m_trktree->Branch("N_TPChit", &N_TPChit);
  m_trktree->Branch("N_SEThit", &N_SEThit);
  m_trktree->Branch("N_FTDhit", &N_FTDhit);
  m_trktree->Branch("N_SITrawhit", &N_SITrawhit);
  m_trktree->Branch("N_SETrawhit", &N_SETrawhit);
  m_trktree->Branch("trk_px", &m_trk_px);
  m_trktree->Branch("trk_py", &m_trk_py);
  m_trktree->Branch("trk_pz", &m_trk_pz);
  m_trktree->Branch("trk_p", &m_trk_p);
  m_trktree->Branch("trk_startpoint_x", &m_trk_startpoint_x);
  m_trktree->Branch("trk_startpoint_y", &m_trk_startpoint_y);
  m_trktree->Branch("trk_startpoint_z", &m_trk_startpoint_z);
  m_trktree->Branch("trk_endpoint_x", &m_trk_endpoint_x);
  m_trktree->Branch("trk_endpoint_y", &m_trk_endpoint_y);
  m_trktree->Branch("trk_endpoint_z", &m_trk_endpoint_z);
  m_trktree->Branch("trk_type", &m_trk_type);
  m_trktree->Branch("trk_Nhit", &m_trk_Nhit);

  return GaudiAlgorithm::initialize();
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

  try{
  if(m_VTXTrackerHitColHdl.get())
    N_VTXhit = m_VTXTrackerHitColHdl.get()->size();
  }catch(GaudiException &e){
    debug()<<"VTX hit is not available "<<endmsg;
  }

  try{
  if(m_SITTrackerHitColHdl.get())
    N_SIThit = m_SITTrackerHitColHdl.get()->size();
  }catch(GaudiException &e){
    debug()<<"SIT hit is not available "<<endmsg;
  }

  try{
  if(m_TPCTrackerHitColHdl.get())
    N_TPChit = m_TPCTrackerHitColHdl.get()->size();
  }catch(GaudiException &e){
    debug()<<"TPC hit is not available "<<endmsg;
  }

  try{
  if(m_SETTrackerHitColHdl.get())
    N_SEThit = m_SETTrackerHitColHdl.get()->size();
  }catch(GaudiException &e){
    debug()<<"SET hit is not available "<<endmsg;
  }

  try{
  if(m_FTDSpacePointColHdl.get())
    N_FTDhit = m_FTDSpacePointColHdl.get()->size();
  }catch(GaudiException &e){
    debug()<<"FDT space point is not available "<<endmsg;
  }

  try{
  if(m_SITRawColHdl.get())
    N_SITrawhit = m_SITRawColHdl.get()->size();
  }catch(GaudiException &e){
    debug()<<"SIT raw hit is not available "<<endmsg;
  }

  try{
  if(m_SETRawColHdl.get())
    N_SETrawhit = m_SETRawColHdl.get()->size();
  }catch(GaudiException &e){
    debug()<<"SET raw hit is not available "<<endmsg;
  }

  try{
  auto const_SiTrkCol = m_SiTrk.get();
  if(const_SiTrkCol){
    N_SiTrk = const_SiTrkCol->size();
    for(int i=0; i<N_SiTrk; i++){
      auto m_trk = const_SiTrkCol->at(i);
      if(m_trk.trackStates_size()==0) continue;
      if(m_trk.trackerHits_size()==0) continue;
      edm4hep::TrackState m_trkstate = m_trk.getTrackStates(0);

      HelixClassD * TrkInit_Helix = new HelixClassD();
      TrkInit_Helix->Initialize_Canonical(m_trkstate.phi, m_trkstate.D0, m_trkstate.Z0, m_trkstate.omega, m_trkstate.tanLambda, _Bfield);
      TVector3 TrkMom(TrkInit_Helix->getMomentum()[0],TrkInit_Helix->getMomentum()[1],TrkInit_Helix->getMomentum()[2]);

      int NTrkHit = m_trk.trackerHits_size();
      TVector3 StartPointPos = TVector3((m_trk.getTrackerHits(0)).getPosition().x,(m_trk.getTrackerHits(0)).getPosition().y,(m_trk.getTrackerHits(0)).getPosition().z);
      TVector3 EndPointPos = TVector3((m_trk.getTrackerHits(NTrkHit - 1)).getPosition().x,(m_trk.getTrackerHits(NTrkHit - 1)).getPosition().y,(m_trk.getTrackerHits(NTrkHit - 1)).getPosition().z);

      m_trk_Nhit.push_back(NTrkHit);
      m_trk_px.push_back(TrkMom.x());
      m_trk_py.push_back(TrkMom.y());
      m_trk_pz.push_back(TrkMom.z());
      m_trk_p.push_back(TrkMom.Mag());
      m_trk_endpoint_x.push_back(EndPointPos.x());
      m_trk_endpoint_y.push_back(EndPointPos.y());
      m_trk_endpoint_z.push_back(EndPointPos.z());
      m_trk_startpoint_x.push_back(StartPointPos.x());
      m_trk_startpoint_y.push_back(StartPointPos.y());
      m_trk_startpoint_z.push_back(StartPointPos.z());
      m_trk_type.push_back(1);

      delete TrkInit_Helix;
    }
  }
  }catch(GaudiException &e){
    debug()<<"Si track is not available "<<endmsg;
  }  

  try{
  auto const_TPCTrkCol = m_TPCTrk.get();
  if(const_TPCTrkCol){
    N_TPCTrk = const_TPCTrkCol->size();
    for(int i=0; i<N_TPCTrk; i++){
      auto m_trk = const_TPCTrkCol->at(i);
      if(m_trk.trackStates_size()==0) continue;
      if(m_trk.trackerHits_size()==0) continue;
      edm4hep::TrackState m_trkstate = m_trk.getTrackStates(0);

      HelixClassD * TrkInit_Helix = new HelixClassD();
      TrkInit_Helix->Initialize_Canonical(m_trkstate.phi, m_trkstate.D0, m_trkstate.Z0, m_trkstate.omega, m_trkstate.tanLambda, _Bfield);
      TVector3 TrkMom(TrkInit_Helix->getMomentum()[0],TrkInit_Helix->getMomentum()[1],TrkInit_Helix->getMomentum()[2]);

      int NTrkHit = m_trk.trackerHits_size();
      TVector3 StartPointPos = TVector3((m_trk.getTrackerHits(0)).getPosition().x,(m_trk.getTrackerHits(0)).getPosition().y,(m_trk.getTrackerHits(0)).getPosition().z);
      TVector3 EndPointPos = TVector3((m_trk.getTrackerHits(NTrkHit - 1)).getPosition().x,(m_trk.getTrackerHits(NTrkHit - 1)).getPosition().y,(m_trk.getTrackerHits(NTrkHit - 1)).getPosition().z);

      m_trk_Nhit.push_back(NTrkHit);
      m_trk_px.push_back(TrkMom.x());
      m_trk_py.push_back(TrkMom.y());
      m_trk_pz.push_back(TrkMom.z());
      m_trk_p.push_back(TrkMom.Mag());
      m_trk_endpoint_x.push_back(EndPointPos.x());
      m_trk_endpoint_y.push_back(EndPointPos.y());
      m_trk_endpoint_z.push_back(EndPointPos.z());
      m_trk_startpoint_x.push_back(StartPointPos.x());
      m_trk_startpoint_y.push_back(StartPointPos.y());
      m_trk_startpoint_z.push_back(StartPointPos.z());
      m_trk_type.push_back(2);

      delete TrkInit_Helix;
    }
  }
  }catch(GaudiException &e){
    debug()<<"TPC track is not available "<<endmsg;
  }


  try{
  auto const_fullTrkCol = m_fullTrk.get();
  if(const_fullTrkCol){
    N_fullTrk = const_fullTrkCol->size();
    for(int i=0; i<N_fullTrk; i++){
      auto m_trk = const_fullTrkCol->at(i);
      if(m_trk.trackStates_size()==0) continue;
      if(m_trk.trackerHits_size()==0) continue;
      edm4hep::TrackState m_trkstate = m_trk.getTrackStates(0);

      HelixClassD * TrkInit_Helix = new HelixClassD();
      TrkInit_Helix->Initialize_Canonical(m_trkstate.phi, m_trkstate.D0, m_trkstate.Z0, m_trkstate.omega, m_trkstate.tanLambda, _Bfield);
      TVector3 TrkMom(TrkInit_Helix->getMomentum()[0],TrkInit_Helix->getMomentum()[1],TrkInit_Helix->getMomentum()[2]);   

      int NTrkHit = m_trk.trackerHits_size();
      TVector3 StartPointPos = TVector3((m_trk.getTrackerHits(0)).getPosition().x,(m_trk.getTrackerHits(0)).getPosition().y,(m_trk.getTrackerHits(0)).getPosition().z);
      TVector3 EndPointPos = TVector3((m_trk.getTrackerHits(NTrkHit - 1)).getPosition().x,(m_trk.getTrackerHits(NTrkHit - 1)).getPosition().y,(m_trk.getTrackerHits(NTrkHit - 1)).getPosition().z);

      m_trk_Nhit.push_back(NTrkHit);
      m_trk_px.push_back(TrkMom.x());
      m_trk_py.push_back(TrkMom.y());
      m_trk_pz.push_back(TrkMom.z());
      m_trk_p.push_back(TrkMom.Mag());
      m_trk_endpoint_x.push_back(EndPointPos.x());
      m_trk_endpoint_y.push_back(EndPointPos.y());
      m_trk_endpoint_z.push_back(EndPointPos.z());
      m_trk_startpoint_x.push_back(StartPointPos.x());
      m_trk_startpoint_y.push_back(StartPointPos.y());
      m_trk_startpoint_z.push_back(StartPointPos.z());
      m_trk_type.push_back(3);

      delete TrkInit_Helix;
    }
  }
  }catch(GaudiException &e){
    debug()<<"Full track is not available "<<endmsg;
  }

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

  m_mctree->Fill(); 
  m_trktree->Fill();
  _nEvt++; 
  return StatusCode::SUCCESS;
}

StatusCode ReadDigiAlg::finalize()
{
  debug() << "begin finalize ReadDigiAlg" << endmsg;
  m_wfile->cd();
  m_mctree->Write();
  m_trktree->Write();
  m_wfile->Close(); 

  return GaudiAlgorithm::finalize();
}

StatusCode ReadDigiAlg::Clear()
{

  N_MCP = -99;
  MCP_px.clear();
  MCP_py.clear();
  MCP_pz.clear();
  MCP_E.clear();
  MCP_endPoint_x.clear();
  MCP_endPoint_y.clear();
  MCP_endPoint_z.clear();
  MCP_pdgid.clear();
  MCP_gStatus.clear(); 

  N_SiTrk = -99;
  N_TPCTrk = -99;
  N_fullTrk = -99;
  N_VTXhit = -99; 
  N_SIThit = -99; 
  N_TPChit = -99; 
  N_SEThit = -99; 
  N_FTDhit = -99; 
  N_SITrawhit = -99;
  N_SETrawhit = -99;
  m_trk_px.clear(); 
  m_trk_py.clear(); 
  m_trk_pz.clear();
  m_trk_p.clear(); 
  m_trk_endpoint_x.clear();
  m_trk_endpoint_y.clear();
  m_trk_endpoint_z.clear();
  m_trk_startpoint_x.clear();
  m_trk_startpoint_y.clear();
  m_trk_startpoint_z.clear();
  m_trk_type.clear(); 
  m_trk_Nhit.clear();

  return StatusCode::SUCCESS;
}

#endif
