#include "FinalPIDAlg.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "DetInterface/IGeomSvc.h"

using namespace edm4hep;

DECLARE_COMPONENT( FinalPIDAlg )

//------------------------------------------------------------------------------
FinalPIDAlg::FinalPIDAlg( const std::string& name, ISvcLocator* pSvcLocator )
    : Algorithm( name, pSvcLocator ) {
  // input
  declareProperty("PFOCollection", m_inPFOCol, "Reconstructed particles to be PIDed");
  declareProperty("TOFPIDCollection", m_inTofCol, "TOF PID information");
  declareProperty("TPCPIDCollection", m_inDqdxCol, "TPC PID information");
  // output
  declareProperty("OutputPFOName", m_outPFOCol, "Reconstructed particles with PID information");
  // PID method
  declareProperty("Method", m_method = "TPC+TOF", "PID method: TPC, TPC+TOF, TPC+TOF+CALO"); 
}

//------------------------------------------------------------------------------
StatusCode FinalPIDAlg::initialize(){
  debug() << "Initializing..." << endmsg;
  if (m_method == "TPC" || m_method == "TPC+TOF" || m_method == "TPC+TOF+CALO" ) { // add muon later
    debug() << "PID method: " << m_method << endmsg;
  } else {
    error() << "Unknown PID method: " << m_method << endmsg;
    return StatusCode::FAILURE;
  }
  _nEvt = 0;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode FinalPIDAlg::execute(){
  
  const edm4hep::ReconstructedParticleCollection* inpfocol = nullptr;
  const edm4hep::RecTofCollection* tofcol = nullptr;
  const edm4hep::RecDqdxCollection* dqdxcol = nullptr;
  _hasTOF = true;
  _hasTPC = true;

  //auto outpfocol = m_outPFOCol.createAndPut();
  edm4hep::ReconstructedParticleCollection* outpfocol = m_outPFOCol.createAndPut();

  try {
    inpfocol = m_inPFOCol.get();
  }
  catch ( GaudiException &e ) {
    debug() << " Reconstructed particle " << m_inPFOCol.fullKey() << " is unavailable in event " << _nEvt << endmsg;
    _nEvt++;
    return StatusCode::SUCCESS;
  }
  try {
    tofcol = m_inTofCol.get();
  }
  catch ( GaudiException &e ) {
    _hasTOF = false;
  }
  try {
    dqdxcol = m_inDqdxCol.get();
  }
  catch ( GaudiException &e ) {
    _hasTPC = false;
  }
  debug()<<" has TOF : "<<_hasTOF<<" has TPC : "<<_hasTPC<<" TOF size : "<<tofcol->size()<<" dqdx size : "<<dqdxcol->size()<<endmsg;
  
  //if ( ( !_hasTPC && !_hasTOF ) || ( tofcol->size() == 0 && dqdxcol->size() == 0 ) ){
  //  
  //  debug() << "TPC and TOF PID information are not available, skip event " << _nEvt << endmsg;
  //  
  //  for ( auto pfo : *inpfocol ){
  //    auto outpfo = pfo.clone();
  //    outpfocol->push_back(outpfo);
  //  }
  //  
  //  _nEvt++;
  //  return StatusCode::SUCCESS;
  //}

  for ( auto pfo : *inpfocol ){
    auto outpfo = pfo.clone();
    outpfocol->push_back(outpfo);
    if(m_method.value().find("TPC+TOF")!=std::string::npos ){
      std::array<double, 5> chi2s; chi2s.fill(0);
      if( _hasTPC && dqdxcol->size()>0){
        FillTPCPID(dqdxcol, outpfo, chi2s);
      }
      if( _hasTOF && tofcol->size()>0){
        FillTOFPID(tofcol, outpfo, chi2s);
      }
    }
    if(m_method.value().find("CALO")!=std::string::npos ) 
      FillCaloPID(outpfo);

    debug()<<" cyber pfo pid : " << outpfo.getType() << " cyber pfo charge : " << outpfo.getCharge() << " cyber pfo energy "<< outpfo.getEnergy() << " cyber pfo mass "<< outpfo.getMass() << endmsg;
  }

  _nEvt++;

  return StatusCode::SUCCESS;
}// end execute

//------------------------------------------------------------------------------
StatusCode FinalPIDAlg::finalize(){
  debug() << "Finalizing..." << endmsg;
  return StatusCode::SUCCESS;
}


void FinalPIDAlg::FillTPCPID(const edm4hep::RecDqdxCollection* dqdxcol, edm4hep::MutableReconstructedParticle& pfo, std::array<double, 5>& chi2s){

    for (auto trk : pfo.getTracks()){

      for (auto dqdx : *dqdxcol){
        if (dqdx.getTrack() == trk){
          for (int i = 0; i < 5; i++){
            chi2s[i] = dqdx.getHypotheses(i).chi2;
          }
        }
      }

      int pdgid = 0;
      if ( std::all_of(chi2s.begin(), chi2s.end(), [](double x){return x == 0;}) ){
        pdgid = pfo.getCharge() * 211;
      }else{
        int minchi2idx = std::distance(chi2s.begin(), std::min_element(chi2s.begin(), chi2s.end()));
        pdgid = pfo.getCharge() * PDGIDs.at(minchi2idx);
      }
      pfo.setType( pdgid );
      pfo.setMass( ParticleMass.at( abs(pdgid) ) );
      pfo.setEnergy( sqrt(pfo.getMomentum()[0]*pfo.getMomentum()[0] + pfo.getMomentum()[1]*pfo.getMomentum()[1] + pfo.getMomentum()[2]*pfo.getMomentum()[2] + pfo.getMass()*pfo.getMass()) );
      debug() << " fill tpc pid:  chi2s : " << chi2s[0]<<" " <<chi2s[1]<<" " <<chi2s[2]<<" "<<chi2s[3]<<" "<<chi2s[4]<<"	id:"<<pfo.getType()<<" 	mass:"<<pfo.getMass()<<endmsg;
    }
    
}

void FinalPIDAlg::FillTOFPID(const edm4hep::RecTofCollection* tofcol, edm4hep::MutableReconstructedParticle& pfo, std::array<double, 5>& chi2s){

    for (auto trk : pfo.getTracks()){

      for (auto tof : *tofcol){
        if (tof.getTrack() == trk){
          double toft = tof.getTime();
          std::array<float, 5> tofexpts = tof.getTimeExp();
          double tofexpterr = tof.getSigma();
          std::array<double, 5> tofchi2s;
          for (int i = 0; i < 5; i++){
            tofchi2s[i] = std::pow( (tofexpts[i] - toft) / tofexpterr, 2);
            chi2s[i] += tofchi2s[i];
          }
        }
      }

      int pdgid = 0;
      if ( std::all_of(chi2s.begin(), chi2s.end(), [](double x){return x == 0;}) ){
        pdgid = pfo.getCharge() * 211;
      }else{
        int minchi2idx = std::distance(chi2s.begin(), std::min_element(chi2s.begin(), chi2s.end()));
        pdgid = pfo.getCharge() * PDGIDs.at(minchi2idx);
      }

      pfo.setType( pdgid );
      pfo.setMass( ParticleMass.at( abs(pdgid) ) );
      pfo.setEnergy( sqrt(pfo.getMomentum()[0]*pfo.getMomentum()[0] + pfo.getMomentum()[1]*pfo.getMomentum()[1] + pfo.getMomentum()[2]*pfo.getMomentum()[2] + pfo.getMass()*pfo.getMass()) );
      debug() << " fill tof pid: chi2 : " << chi2s[0]<<" " <<chi2s[1]<<" " <<chi2s[2]<<" "<<chi2s[3]<<" "<<chi2s[4]<<"	id:"<<pfo.getType()<<" 	mass:"<<pfo.getMass()<<endmsg;

    }

}


StatusCode FinalPIDAlg::FillCaloPID(edm4hep::MutableReconstructedParticle& pfo){
    //Use cluster position to do preliminary gamma/h0 PID. 
    //TODO: update with cluster type and energy in sub-detector. 

    int Ncluster = pfo.clusters_size(); 
    //std::cout<<"In PFO: total cluster size "<<Ncluster<<", current pid: "<<pfo.getType()<<std::endl;
    double Etot_cluster = 0.;
    TVector3 pfo_position(0., 0., 0.);
    for(auto clu : pfo.getClusters() ){
        if(!clu.isAvailable()) continue;
        //printf("  In cluster #: pos+E (%.3f, %.3f, %.3f, %.3f), type %d, sub-cluster size %d, hit size %d \n", 
        //    clu.getPosition().x, 
        //    clu.getPosition().y, 
        //    clu.getPosition().z, 
        //    clu.getEnergy(), 
        //    clu.getType(), 
        //    clu.clusters_size(), 
        //    clu.hits_size()  );

        Etot_cluster += clu.getEnergy();
        TVector3 clu_position(clu.getPosition().x, clu.getPosition().y, clu.getPosition().z);
        pfo_position += clu.getEnergy() * clu_position;
    }
    pfo_position = pfo_position*(1./Etot_cluster);

    debug() << "PFO position (" << pfo_position.Perp()<<", "<<pfo_position.z()<<"), total energy " << Etot_cluster << endmsg;

    if( pfo.getType()==0 ){ //Temp: do not consider combined PID from different sub-detectors. 
        if( fabs(pfo_position.Z())<EcalHalfZ && fabs(pfo_position.Perp())<EcalOuterR ){
            int pdgid = 22;
            pfo.setType( pdgid );
            pfo.setMass( ParticleMass.at( abs(pdgid) ) );
            double p_scale = sqrt( pfo.getEnergy()*pfo.getEnergy() - pfo.getMass()*pfo.getMass() ) / sqrt(pfo.getMomentum()[0]*pfo.getMomentum()[0] + pfo.getMomentum()[1]*pfo.getMomentum()[1] + pfo.getMomentum()[2]*pfo.getMomentum()[2] );
            pfo.setMomentum( Vector3f(pfo.getMomentum()[0]*p_scale, pfo.getMomentum()[1]*p_scale, pfo.getMomentum()[2]*p_scale) );
        }
        else{
            int pdgid = 130;
            pfo.setType( pdgid );
            pfo.setMass( ParticleMass.at( abs(pdgid) ) );
            double p_scale = sqrt( pfo.getEnergy()*pfo.getEnergy() - pfo.getMass()*pfo.getMass() ) / sqrt(pfo.getMomentum()[0]*pfo.getMomentum()[0] + pfo.getMomentum()[1]*pfo.getMomentum()[1] + pfo.getMomentum()[2]*pfo.getMomentum()[2] );
            pfo.setMomentum( Vector3f(pfo.getMomentum()[0]*p_scale, pfo.getMomentum()[1]*p_scale, pfo.getMomentum()[2]*p_scale) );

        }
    }

    return StatusCode::SUCCESS;
}
