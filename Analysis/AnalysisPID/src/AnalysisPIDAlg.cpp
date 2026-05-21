#include "AnalysisPIDAlg.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartDataPtr.h"

#include "DetInterface/IGeomSvc.h"
#include "DataHelper/HelixClass.h"

#include "DD4hep/Detector.h"
#include "DD4hep/DD4hepUnits.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include <cmath>
#include "UTIL/ILDConf.h"
#include "DetIdentifier/CEPCConf.h"
#include "UTIL/CellIDEncoder.h"

using namespace edm4hep;

DECLARE_COMPONENT( AnalysisPIDAlg )

//------------------------------------------------------------------------------
AnalysisPIDAlg::AnalysisPIDAlg( const std::string& name, ISvcLocator* pSvcLocator )
    : Algorithm( name, pSvcLocator ) {
  declareProperty("CompleteTracks", _FultrkCol, "handler of the input complete track collection");
  declareProperty("CompleteTracksParticleAssociation", _FultrkParAssCol, "handler of the input track particle association collection");
  declareProperty("DndxTracks", _inDndxColHdl, "handler of the collection of dN/dx tracks");
  declareProperty("RecTofCollection", _inTofColHdl, "handler of the collection of tof tracks");
  // output
  declareProperty("OutputFile", m_outputFile = "pid.root", "output file name");
}

//------------------------------------------------------------------------------
StatusCode AnalysisPIDAlg::initialize(){

  info() << "Booking Ntuple" << endmsg;
  m_file = new TFile(m_outputFile.value().c_str(),"RECREATE");
  m_tree = new TTree("pid","pid");

  m_tree->Branch("Nevt",&_nEvt,"Nevt/I");
  m_tree->Branch("Ndndxtrk",&Ndndxtrk,"Ndndxtrk/I");
  m_tree->Branch("Ntoftrk",&Ntoftrk,"Ntoftrk/I");
  m_tree->Branch("Nfullass",&Nfullass,"Nfullass/I");
  m_tree->Branch("Nfulltrk",&Nfulltrk,"Nfulltrk/I");
  m_tree->Branch("tpcidx",&tpcidx);
  m_tree->Branch("tofidx",&tofidx);
  m_tree->Branch("matchedtpc",&matchedtpc);
  m_tree->Branch("matchedtof",&matchedtof);
  m_tree->Branch("truthidx",&truthidx);
  
  m_tree->Branch("tof_chi2s",&tof_chi2s);
  m_tree->Branch("tof_chis",&tof_chis);
  m_tree->Branch("tof_expt",&tof_expt);
  m_tree->Branch("tof_meast",&tof_meast);
  m_tree->Branch("tof_measterr",&tof_measterr);

  m_tree->Branch("tpc_chi2s",&tpc_chi2s);
  m_tree->Branch("tpc_chis",&tpc_chis);
  m_tree->Branch("tpc_expdndxs",&tpc_expdndxs);
  m_tree->Branch("tpc_measdndx",&tpc_measdndx);
  m_tree->Branch("tpc_measdndxerr",&tpc_measdndxerr);

  m_tree->Branch("tot_chi2s",&tot_chi2s);
  m_tree->Branch("recoPDG",&recoPDG);
  m_tree->Branch("tpcrecoPDG",&tpcrecoPDG);
  m_tree->Branch("tofrecoPDG",&tofrecoPDG);

  //gen trk parameters
  m_tree->Branch("genpx",&genpx);
  m_tree->Branch("genpy",&genpy);
  m_tree->Branch("genpz",&genpz);
  m_tree->Branch("genE",&genE);
  m_tree->Branch("genp",&genp);
  m_tree->Branch("genM",&genM);
  m_tree->Branch("genphi",&genphi);
  m_tree->Branch("gentheta",&gentheta);
  m_tree->Branch("endx",&endx);
  m_tree->Branch("endy",&endy);
  m_tree->Branch("endz",&endz);
  m_tree->Branch("endr",&endr);
  m_tree->Branch("PDG",&PDG);
  m_tree->Branch("genstatus",&genstatus);
  m_tree->Branch("simstatus",&simstatus);
  m_tree->Branch("isdecayintrker",&isdecayintrker);
  m_tree->Branch("iscreatedinsim",&iscreatedinsim);
  m_tree->Branch("isbackscatter",&isbackscatter);
  m_tree->Branch("isstopped",&isstopped);

  _nEvt = 0;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode AnalysisPIDAlg::execute(){
  
  const edm4hep::TrackCollection* trkCol = nullptr;
  const edm4hep::RecDqdxCollection* dndxCols = nullptr;
  const edm4hep::RecTofCollection* tofCols = nullptr;
  const edm4hep::MCRecoTrackParticleAssociationCollection* fultrkparassCols = nullptr;
  
  ClearVars();

  try {
    trkCol = _FultrkCol.get();
  }
  catch ( GaudiException &e ) {
    debug() << "Complete track collection " << _FultrkCol.fullKey() << " is unavailable in event " << _nEvt << endmsg;
    Nfulltrk = -1;
  }
  if ( trkCol->size() == 0 ) {
    debug() << "No full track found in event " << _nEvt << endmsg;
    Nfulltrk = 0;
  }
  else{
    Nfulltrk = trkCol->size();
  }

  try {
    fultrkparassCols = _FultrkParAssCol.get();
  }
  catch ( GaudiException &e ) {
    debug() << "Complete track particle association collection " << _FultrkParAssCol.fullKey() << " is unavailable in event " << _nEvt << endmsg;
    Nfullass = -1;
    m_tree->Fill();
    _nEvt++;
    return StatusCode::SUCCESS;
  }

  try {
    dndxCols = _inDndxColHdl.get();
  }
  catch ( GaudiException &e ) {
    debug() << "DndxTrack collection " << _inDndxColHdl.fullKey() << " is unavailable in event " << _nEvt << endmsg;
    Ndndxtrk = -1;
  }

  if ( dndxCols->size() == 0 ) {
    debug() << "No dndx track found in event " << _nEvt << endmsg;
    Ndndxtrk = 0;
  }
  else{
    Ndndxtrk = dndxCols->size();
  }

  try {
    tofCols = _inTofColHdl.get();
  }
  catch ( GaudiException &e ) {
    debug() << "TofTrack collection " << _inTofColHdl.fullKey() << " is unavailable in event " << _nEvt << endmsg;
    Ntoftrk = -1;
  }

  if ( tofCols->size() == 0 ) {
    debug() << "No tof track found in event " << _nEvt << endmsg;
    Ntoftrk = 0;
  }
  else{
    Ntoftrk = tofCols->size();
  }

  if ( fultrkparassCols->size() == 0 ) {
    debug() << "No full track particle association found in event " << _nEvt << endmsg;
    Nfullass = 0;
    m_tree->Fill();
    _nEvt++;
    return StatusCode::SUCCESS;
  }
  else{
    Nfullass = fultrkparassCols->size();
  }
  
  info() << "normal eventID: " << _nEvt << endmsg;
  for(auto track : *trkCol){
      //truth association match
      max_weight = -999.;
      max_weight_idx = -1;
      ass_idx = 0;
      for (auto ass : *fultrkparassCols) {
          if (ass.getRec() == track) {
              weight = ass.getWeight();
              if (weight > max_weight) {
                  max_weight = weight;
                  max_weight_idx = ass_idx;
              }
          }
          ass_idx++;
      }
      truthidx.push_back(max_weight_idx);
//      if (max_weight_idx < 0) continue;
      p1=fultrkparassCols->at(max_weight_idx).getSim().getMomentum()[0];
      p2=fultrkparassCols->at(max_weight_idx).getSim().getMomentum()[1];
      p3=fultrkparassCols->at(max_weight_idx).getSim().getMomentum()[2];
      genpx.push_back(p1);
      genpy.push_back(p2);
      genpz.push_back(p3);
      genp.push_back(std::sqrt(p1*p1 + p2*p2 + p3*p3));
      genE.push_back(fultrkparassCols->at(max_weight_idx).getSim().getEnergy());
      genM.push_back(fultrkparassCols->at(max_weight_idx).getSim().getMass());
      genphi.push_back(std::atan2(p2,p1));
      gentheta.push_back(std::acos(p3/std::sqrt(p1*p1 + p2*p2 + p3*p3)));
      x1=fultrkparassCols->at(max_weight_idx).getSim().getEndpoint()[0];
      y1=fultrkparassCols->at(max_weight_idx).getSim().getEndpoint()[1];
      endx.push_back(x1);
      endy.push_back(y1);
      endz.push_back(fultrkparassCols->at(max_weight_idx).getSim().getEndpoint()[2]);
      endr.push_back(std::sqrt(x1*x1 + y1*y1));
      PDG.push_back(fultrkparassCols->at(max_weight_idx).getSim().getPDG());
      genstatus.push_back(fultrkparassCols->at(max_weight_idx).getSim().getGeneratorStatus());
      simstatus.push_back(fultrkparassCols->at(max_weight_idx).getSim().getSimulatorStatus());
      isdecayintrker.push_back(fultrkparassCols->at(max_weight_idx).getSim().isDecayedInTracker());//getSimulatorStatus().isDecayInTracker();
      iscreatedinsim.push_back(fultrkparassCols->at(max_weight_idx).getSim().isCreatedInSimulation());
      isbackscatter.push_back(fultrkparassCols->at(max_weight_idx).getSim().isBackscatter());
      isstopped.push_back(fultrkparassCols->at(max_weight_idx).getSim().isStopped());

      //find corresponding dndx track
      edm4hep::RecDqdx dndxtrk;
      dndx_index = -1;
      matched1 = false;
      for(int i=0; i<Ndndxtrk; i++){
          if ( dndxCols->at(i).getTrack() == track ){
              dndxtrk = dndxCols->at(i);
              dndx_index = i;
              matched1 = true;
              break;
          }
      }
      tpcidx.push_back(dndx_index);
      tpc_chi2s_1.clear();tpc_expdndxs_1.clear();tpc_chis_1.clear();
      tpcdndx=-1;tpcdndxerr=-1;
      if( matched1 ) {
          tpcdndx = dndxtrk.getDQdx().value;
          tpcdndxerr = dndxtrk.getDQdx().error;
          debug()<<"tpc_measdndx = "<<dndxtrk.getDQdx().value<<endmsg;
          for (int idx=0;idx<5;idx++) {
              double tpc_chi2 = dndxtrk.getHypotheses(idx).chi2;
              double tpc_expdndx = dndxtrk.getHypotheses(idx).expected;
              double tpc_chi = ( tpcdndx - tpc_expdndx ) / tpcdndxerr;
              tpc_chi2s_1.push_back(tpc_chi2);
              tpc_chis_1.push_back(tpc_chi);
              tpc_expdndxs_1.push_back(tpc_expdndx);
              debug()<<" idx : "<< idx <<" tpc_chi2 : "<< tpc_chi2 <<endmsg;    
          }
      }
      else{
          tpc_chi2s_1={-999,-999,-999,-999,-999};
          tpc_chis_1={-999,-999,-999,-999,-999};
          tpc_expdndxs_1={-1,-1,-1,-1,-1};
      }
      tpc_measdndx.push_back(tpcdndx);
      tpc_measdndxerr.push_back(tpcdndxerr);
      matchedtpc.push_back(matched1);
      tpc_chi2s.push_back(tpc_chi2s_1);
      tpc_chis.push_back(tpc_chis_1);
      tpc_expdndxs.push_back(tpc_expdndxs_1);

      //find corresponding tof track
      edm4hep::RecTof toftrk;
      tof_index = -1;
      matched2 = false;
      for(int i=0; i<Ntoftrk; i++){
          if ( tofCols->at(i).getTrack() == track ){
              toftrk = tofCols->at(i);
              tof_index = i;
              matched2 = true;
              break;
          }
      }
      tofidx.push_back(tof_index);
      tof_chi2s_1.clear();tof_expt_1.clear();tof_chis_1.clear();
      toft=-1;tofterr=-1;
      if ( matched2 ) {
          toft = toftrk.getTime();
          std::array<float, 5> tofexpts = toftrk.getTimeExp();
          tofterr = toftrk.getSigma();
          debug()<<"tof_meast = "<<toftrk.getTime()<<endmsg;
          for (int idx=0;idx<5;idx++){
              double tof_chi = ( toft - tofexpts[idx] ) / tofterr;
              double tof_chi2 = tof_chi*tof_chi;
              tof_chi2s_1.push_back(tof_chi2);
              tof_chis_1.push_back(tof_chi);
              tof_expt_1.push_back(tofexpts[idx]);
              debug()<<" idx : "<< idx <<" tof_chi2 : "<< tof_chi2 <<endmsg;    
          }//end loop over masses
      }
      else{
          tof_chi2s_1={-999,-999,-999,-999,-999};
          tof_chis_1={-999,-999,-999,-999,-999};
          tof_expt_1={-1,-1,-1,-1,-1};
      }
      tof_meast.push_back(toft);
      tof_measterr.push_back(tofterr);
      matchedtof.push_back(matched2);
      tof_chi2s.push_back(tof_chi2s_1);
      tof_chis.push_back(tof_chis_1);
      tof_expt.push_back(tof_expt_1);

      tot_chi2s_1.clear();
      recpdg = 9999;tpcrecpdg = 9999;tofrecpdg = 9999;
      if(matched1){ 
          minchi2idx = std::distance(tpc_chi2s_1.begin(), std::min_element(tpc_chi2s_1.begin(), tpc_chi2s_1.end()));
          tpcrecpdg = PDGIDs.at(minchi2idx);
      }
      if(matched2){
          minchi2idx = std::distance(tof_chi2s_1.begin(), std::min_element(tof_chi2s_1.begin(), tof_chi2s_1.end()));
          tofrecpdg = PDGIDs.at(minchi2idx);
      }
      if(matched1 || matched2){
          if(matched1 && matched2){
          for(int i=0;i<5;i++){
              tot_chi2s_1.push_back(tpc_chi2s_1[i]+tof_chi2s_1[i]);
          }
          }
          if(matched1 && !matched2) tot_chi2s_1=tpc_chi2s_1;
          if(!matched1 && matched2) tot_chi2s_1=tof_chi2s_1;
          minchi2idx = std::distance(tot_chi2s_1.begin(), std::min_element(tot_chi2s_1.begin(), tot_chi2s_1.end()));
          recpdg = PDGIDs.at(minchi2idx);
      }
      else{ 
          tot_chi2s_1={-999,-999,-999,-999,-999};
      }
      int charge = track.getTrackStates(1).omega/fabs(track.getTrackStates(1).omega);
      tot_chi2s.push_back(tot_chi2s_1);
      recoPDG.push_back(charge*recpdg);
      tpcrecoPDG.push_back(charge*tpcrecpdg);
      tofrecoPDG.push_back(charge*tofrecpdg);
  }
m_tree->Fill();
_nEvt++;
return StatusCode::SUCCESS;
}// end execute

//------------------------------------------------------------------------------
StatusCode AnalysisPIDAlg::finalize(){
  debug() << "Finalizing..." << endmsg;
  m_file->cd();
  m_tree->Write();
  return StatusCode::SUCCESS;
}
