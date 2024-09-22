#include "TofRecAlg.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartDataPtr.h"

#include "DetInterface/IGeomSvc.h"
#include "DataHelper/HelixClass.h"

#include "DD4hep/Detector.h"
#include "DD4hep/DD4hepUnits.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include <math.h>
#include "UTIL/ILDConf.h"
#include "DetIdentifier/CEPCConf.h"
#include "UTIL/CellIDEncoder.h"

DECLARE_COMPONENT( TofRecAlg )

//------------------------------------------------------------------------------
TofRecAlg::TofRecAlg( const std::string& name, ISvcLocator* pSvcLocator )
    : Algorithm( name, pSvcLocator ) {
  declareProperty("MCParticleCollection", _inMCColHdl, "Handle of the Input MCParticle collection");
  declareProperty("TrackCollection", _inTrackColHdl, "Handle of the Input Track collection from CEPCSW");
}

//------------------------------------------------------------------------------
StatusCode TofRecAlg::initialize(){
  info() << "Booking Ntuple" << endmsg;

  NTuplePtr nt1(ntupleSvc(), "MyTuples/ToF");
  if ( !nt1 ) {
    m_tuple = ntupleSvc()->book("MyTuples/ToF",CLID_ColumnWiseTuple,"ToF result");
    if ( 0 != m_tuple ) {
      m_tuple->addItem ("ntrk",      m_nTracks, 0, 500 ).ignore();
      m_tuple->addIndexedItem("ToFt", m_nTracks, m_ToFt).ignore();
      m_tuple->addIndexedItem("ToFterr", m_nTracks, m_ToFterr).ignore();
      m_tuple->addIndexedItem("ToFx", m_nTracks, m_ToFx).ignore();
      m_tuple->addIndexedItem("ToFy", m_nTracks, m_ToFy).ignore();
      m_tuple->addIndexedItem("ToFz", m_nTracks, m_ToFz).ignore();
      m_tuple->addIndexedItem("length", m_nTracks, m_length).ignore();
      m_tuple->addIndexedItem("lengtherr", m_nTracks, m_lengtherr).ignore();
      m_tuple->addIndexedItem("exp_time", m_nTracks,  5, m_exp_time).ignore();
      m_tuple->addIndexedItem("exp_timeerr", m_nTracks, 5, m_exp_timeerr).ignore();//above need to be stored in edm4hep

      m_tuple->addIndexedItem("trackState0", m_nTracks, 15, trackState0).ignore();
      m_tuple->addIndexedItem("trackState1", m_nTracks, 15, trackState1).ignore();
      m_tuple->addIndexedItem("trackState2", m_nTracks, 15, trackState2).ignore();
      m_tuple->addIndexedItem("trackState3", m_nTracks, 15, trackState3).ignore();
      m_tuple->addIndexedItem("trackState4", m_nTracks, 15, trackState4).ignore();
      
      m_tuple->addIndexedItem("p_atIP", m_nTracks, m_p_atIP).ignore();
      m_tuple->addIndexedItem("perr_atIP", m_nTracks, m_perr_atIP).ignore();
      m_tuple->addIndexedItem("p_atLastHit", m_nTracks, m_p_atLast).ignore();
      m_tuple->addIndexedItem("perr_atLastHit", m_nTracks, m_perr_atLast).ignore();
    }
    else { // did not manage to book the N tuple....
      fatal() << "Cannot book MyTuples/ToF" <<endmsg; 
      return StatusCode::FAILURE;
    }
  }
  else{
    m_tuple = nt1;
  }

  auto geomSvc = service<IGeomSvc>("GeomSvc");
  if(geomSvc){
    const dd4hep::Direction& field = geomSvc->lcdd()->field().magneticField(dd4hep::Position(0,0,0));
    m_field = field.z()/dd4hep::tesla;
    info() << "Magnetic field will obtain from GeomSvc = " << m_field << " tesla" << endmsg;
  }
  else{
    info() << "Failed to find GeomSvc ..." << endmsg;
    info() << "Magnetic field will use what input through python option for this algorithm namse as Field, now " << m_field << " tesla" << endmsg;
  }

  _nEvt = 0;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode TofRecAlg::execute(){
  
  const edm4hep::TrackCollection* trackCols = nullptr;
  auto rectofCol = m_rectofCol.createAndPut();
  
  try {
    trackCols = _inTrackColHdl.get();
  }
  catch ( GaudiException &e ) {
    debug() << "Collection " << _inTrackColHdl.fullKey() << " is unavailable in event " << _nEvt << endmsg;
    return StatusCode::SUCCESS;
  }
  

  if(trackCols){

    m_nTracks = 0;
    
    for(auto track : *trackCols){
      // only take the first track
      // if(m_nTracks>=1)break;
      
	int NHits = track.trackerHits_size();
	int det_id = 0;
      	bool hasSETHit = false;
      	bool hasFTDHit = false;
	UTIL::BitField64 encoder( lcio::ILDCellID0::encoder_string );
	for ( int hit=0; hit<NHits; hit++ ) {
	   edm4hep::TrackerHit SimTHit = track.getTrackerHits(hit);
	   encoder.setValue(SimTHit.getCellID());
	   det_id  = encoder[lcio::ILDCellID0::subdet];
	   if ( det_id == lcio::ILDDetID::SET ){
          	hasSETHit = true;
          	m_ToFt[m_nTracks] = SimTHit.getTime();
          	m_ToFx[m_nTracks] = SimTHit.getPosition()[0];
          	m_ToFy[m_nTracks] = SimTHit.getPosition()[1];
          	m_ToFz[m_nTracks] = SimTHit.getPosition()[2];
           }
	   else if( det_id == lcio::ILDDetID::FTD ){
          	const int cellID = SimTHit.getCellID();
          	encoder.setValue(cellID);
          	int layer = encoder[lcio::ILDCellID0::layer];
          if(layer==4){
            	m_ToFt[m_nTracks] = SimTHit.getTime();
            	m_ToFx[m_nTracks] = SimTHit.getPosition()[0];
            	m_ToFy[m_nTracks] = SimTHit.getPosition()[1];
            	m_ToFz[m_nTracks] = SimTHit.getPosition()[2];
            	hasFTDHit = true;
          }else{
            	m_ToFt[m_nTracks] = -999;
            	m_ToFx[m_nTracks] = -999;
            	m_ToFy[m_nTracks] = -999;
            	m_ToFz[m_nTracks] = -999;
          	}
         }
         else {
          m_ToFt[m_nTracks] = -999;
          m_ToFx[m_nTracks] = -999;
          m_ToFy[m_nTracks] = -999;
          m_ToFz[m_nTracks] = -999;
	}
      }//end track hits
      if ( !hasSETHit && !hasFTDHit ) continue;

      auto rectof = rectofCol->create();
      std::array<float, 5> tmp_timeExp_list{ -99.9, -99.9, -99.9, -99.9, -99.9 };

      // smearing the measured time by gaussian 0.05 for intrinsic and 0.02 for bunchcrossing, giving the measured terr a constant
      m_ToFterr[m_nTracks] = std::sqrt(tof_resolution*tof_resolution + bunchcrossing*bunchcrossing);
      m_ToFt[m_nTracks] += normal_distribution_tof(generator_tof);
      m_ToFt[m_nTracks] += normal_distribution_bunch(generator_bunch);

      // found ToF hit, then to evaluate trajectory length
      for( std::vector<edm4hep::TrackState>::const_iterator it = track.trackStates_begin(); it != track.trackStates_end(); it++ ){
        
        edm4hep::TrackState trackState = *it;
        int _location = trackState.location;
        (*trackStateMap[_location])[m_nTracks][0] = _location;
        (*trackStateMap[_location])[m_nTracks][1] = trackState.D0;
        (*trackStateMap[_location])[m_nTracks][2] = trackState.phi;
        (*trackStateMap[_location])[m_nTracks][3] = trackState.omega;
        (*trackStateMap[_location])[m_nTracks][4] = trackState.Z0;
        (*trackStateMap[_location])[m_nTracks][5] = trackState.tanLambda;
        (*trackStateMap[_location])[m_nTracks][6] = trackState.time;
        (*trackStateMap[_location])[m_nTracks][7] = trackState.referencePoint[0];
        (*trackStateMap[_location])[m_nTracks][8] = trackState.referencePoint[1];
        (*trackStateMap[_location])[m_nTracks][9] = trackState.referencePoint[2];
        (*trackStateMap[_location])[m_nTracks][10] = std::sqrt(trackState.covMatrix[0]);
        (*trackStateMap[_location])[m_nTracks][11] = std::sqrt(trackState.covMatrix[2]);
        (*trackStateMap[_location])[m_nTracks][12] = std::sqrt(trackState.covMatrix[5]);
        (*trackStateMap[_location])[m_nTracks][13] = std::sqrt(trackState.covMatrix[9]);
        (*trackStateMap[_location])[m_nTracks][14] = std::sqrt(trackState.covMatrix[14]);
        
      }

      //expected time calculation
      if ( hasSETHit || hasFTDHit ){

        // trackState at IP
        float phi0 = (*trackStateMap[1])[m_nTracks][2];
        if (phi0 < 0) phi0 += 2*M_PI;
        float phi0err = (*trackStateMap[1])[m_nTracks][11];
        //float z0 = (*trackStateMap[1])[m_nTracks][4];
        //float z0err = (*trackStateMap[1])[m_nTracks][13];
        float omega0 = (*trackStateMap[1])[m_nTracks][3];
        float omega0err = (*trackStateMap[1])[m_nTracks][12];
        float totz0 = (*trackStateMap[1])[m_nTracks][4] + (*trackStateMap[1])[m_nTracks][9];
        float totz0err = (*trackStateMap[1])[m_nTracks][13];
        float tanL0 = (*trackStateMap[1])[m_nTracks][5];
        float tanL0err = (*trackStateMap[1])[m_nTracks][14];

        float radius0 = 1./std::abs(omega0);
        float radius0err = omega0err/(omega0*omega0);
        
	float pT_tmp = radius0 * m_field * c * 10E-13;
        float pTerr_tmp = radius0err * m_field * c * 10E-13;
        
	float pZ_tmp = pT_tmp*tanL0;
        float pZerr_tmp = std::sqrt( pT_tmp*pT_tmp*tanL0err*tanL0err + tanL0*tanL0*pTerr_tmp*pTerr_tmp );
        
	float p_atIP = std::sqrt( pT_tmp*pT_tmp + pZ_tmp*pZ_tmp );
        float perr_atIP = std::sqrt( (pT_tmp*pT_tmp / p_atIP*p_atIP)*(pTerr_tmp*pTerr_tmp) + (pZ_tmp*pZ_tmp / p_atIP*p_atIP)*(pZerr_tmp*pZerr_tmp) );

        // trackState at the last hit
        float phi1        = (*trackStateMap[3])[m_nTracks][2];
        if (phi1 < 0) phi1 += 2*M_PI;
        float phi1err     = (*trackStateMap[3])[m_nTracks][11];
        //float z1          = (*trackStateMap[3])[m_nTracks][4];
        //float z1err       = (*trackStateMap[3])[m_nTracks][13];
        float omega1      = (*trackStateMap[3])[m_nTracks][3];
        float omega1err   = (*trackStateMap[3])[m_nTracks][12];
        float totz1       = (*trackStateMap[3])[m_nTracks][4] + (*trackStateMap[3])[m_nTracks][9];
        float totz1err    = (*trackStateMap[3])[m_nTracks][13];
        float tanL1       = (*trackStateMap[3])[m_nTracks][5];
        float tanL1err    = (*trackStateMap[3])[m_nTracks][14];
	float positionx   = (*trackStateMap[3])[m_nTracks][7];
	float positiony   = (*trackStateMap[3])[m_nTracks][8];
	float positionz   = (*trackStateMap[3])[m_nTracks][9];

        float radius1     = 1./std::abs(omega1);
        float radius1err  = omega1err/(omega1*omega1);

        float pT_tmp1     = radius1 * m_field * c * 10E-13;
        float pTerr_tmp1  = radius1err * m_field * c * 10E-13;

        float pZ_tmp1     = pT_tmp1*tanL1;
        float pZerr_tmp1  = std::sqrt( pT_tmp1*pT_tmp1*tanL1err*tanL1err + tanL1*tanL1*pTerr_tmp1*pTerr_tmp1 );

        float p_atLast    = std::sqrt(pT_tmp1*pT_tmp1 + pZ_tmp1*pZ_tmp1);
        float perr_atLast = std::sqrt( (pT_tmp1*pT_tmp1 / p_atLast*p_atLast)*(pTerr_tmp1*pTerr_tmp1) + (pZ_tmp1*pZ_tmp1 / p_atLast*p_atLast)*(pZerr_tmp1*pZerr_tmp1) );
        
	//assum all tracks end in one circle, for multi cirlce tracks, the result does not make sense
	float deltaphi = phi1 -phi0;
	if ( omega0 * deltaphi > 0.0 ){
	    deltaphi = 2. * M_PI - std::abs( deltaphi );
	}

        // calculate the length
        float _length             = std::sqrt( radius0*radius0 * (deltaphi)*(deltaphi) + (totz1-totz0)*(totz1-totz0) );//in mm
        float _lengtherr          = 0.0;
        float _ldr2               = (radius0*radius0) * (deltaphi)*(deltaphi)*(deltaphi)*(deltaphi) / (_length*_length);
        float _ldphi02            = radius0*radius0*radius0*radius0 * (deltaphi)*(deltaphi) / (_length*_length);
        float _ldphi12            = _ldphi02;
        float _ldz02              = (totz1-totz0)*(totz1-totz0) / (_length*_length);
        float _ldz12              = _ldz02;
        
        _lengtherr               = std::sqrt( _ldr2*radius0err*radius0err + _ldphi02*phi0err*phi0err + _ldphi12*phi1err*phi1err + _ldz02*totz0err*totz0err + _ldz12*totz1err*totz1err );
        m_length[m_nTracks]      = _length;
        m_lengtherr[m_nTracks]   = _lengtherr;
        m_p_atIP[m_nTracks]      = p_atIP;
        m_perr_atIP[m_nTracks]   = perr_atIP;
        m_p_atLast[m_nTracks]    = p_atLast;
        m_perr_atLast[m_nTracks] = perr_atLast;
	      
        //momenta to kg m/s
        float p_tmp = p_atIP * GeV2kgms;
        float perr_tmp = perr_atIP * GeV2kgms;
        
	for (auto it = masses.begin(); it != masses.end(); ++it){//expected times and errors for e, mu, pi, k, proton
            int idx = it->first;
            double assum = it->second;
            double m_tmp = assum * GeV2kg;
            double E_tmp = std::sqrt(p_tmp * p_tmp * c * c + m_tmp * m_tmp * c * c * c * c);
            double v_tmp = p_tmp * c * c / E_tmp;
            double verr_tmp = perr_tmp * ( c * c * E_tmp - (p_tmp * p_tmp * c * c * c * c/E_tmp))/ (E_tmp * E_tmp);
            double t_tmp = 10e5 * _length / v_tmp;
            double terr_tmp = 10e5 * std::sqrt( (_lengtherr/v_tmp)*(_lengtherr/v_tmp) + ( (_length*_length)/(v_tmp*v_tmp*v_tmp*v_tmp) )*verr_tmp*verr_tmp );
            m_exp_time[m_nTracks][idx]  	= t_tmp;
            m_exp_timeerr[m_nTracks][idx] 	= terr_tmp;
	    tmp_timeExp_list[idx]		= (float) t_tmp;
            debug()<<"idx : "<< idx <<" assum: "<< assum <<" omega: "<<omega0<<" length: "<<_length<<" phi1: "<<phi1<<" phi0: "<<phi0<<" phi1-phi0: "<<deltaphi<<" exp_t: "<<t_tmp<<" obs_t: "<<m_ToFt[m_nTracks]<<endmsg;
        }
	rectof.setTimeExp( tmp_timeExp_list );
	rectof.setTime( m_ToFt[m_nTracks] );
	rectof.setSigma( m_ToFterr[m_nTracks] );
	rectof.setPosition( { positionx, positiony, positionz } );
	rectof.setPathLength( { _length, _length, _length, _length, _length } );

        }else{
          m_length[m_nTracks] = -999;
          m_lengtherr[m_nTracks] = -999;
          m_p_atIP[m_nTracks] = -999;
          m_perr_atIP[m_nTracks] = -999;
          m_p_atLast[m_nTracks] = -999;
          m_perr_atLast[m_nTracks] = -999;
          for (auto it = masses.begin(); it != masses.end(); ++it){
            int idx = it->first;
            m_exp_time[m_nTracks][idx] = -999;
            m_exp_timeerr[m_nTracks][idx] = -999;
          }
	  rectof.setTimeExp( tmp_timeExp_list );
	  rectof.setTime( -999 );
	  rectof.setSigma( -999 );
	  rectof.setPosition( { -999, -999, -999 } );
	  rectof.setPathLength( { -999, -999, -999, -999, -999 } );
      }
      m_nTracks++;

    }// end track iteration

  }//end track collection

  m_tuple->write();
  _nEvt++;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode TofRecAlg::finalize(){
  debug() << "Finalizing..." << endmsg;

  return StatusCode::SUCCESS;
}
