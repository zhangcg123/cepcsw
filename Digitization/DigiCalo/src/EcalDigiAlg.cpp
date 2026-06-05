/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
// Unit in code: mm, ns. 
// NOTE: This digitialization highly matches detector geometry CRDEcalBarrel_v01/LongCrystalBarBarrelCalorimeter32Polygon_v01.
// TODO: read geometry info automatically.  
#include "EcalDigiAlg.h"

#include "edm4hep/SimCalorimeterHit.h"
#include "edm4hep/CalorimeterHit.h"
#include "edm4hep/Vector3f.h"
#include "edm4hep/Cluster.h"

#include "DD4hep/Detector.h"
#include <DD4hep/Objects.h>
#include <DDRec/CellIDPositionConverter.h>

#include "TVector3.h"
#include "TRandom3.h"
#include <math.h>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <map>
#include <random>

// #include <fstream>
// #include <ctime>

#define C 299.79  // unit: mm/ns
#define PI 3.141592653
using namespace std;
using namespace dd4hep;
using dd4hep::rec::LayeredCalorimeterData;
using dd4hep::rec::LayeredCalorimeterStruct;

DECLARE_COMPONENT( EcalDigiAlg )

EcalDigiAlg::EcalDigiAlg(const std::string& name, ISvcLocator* svcLoc)
	: Algorithm(name, svcLoc),
    _nEvt(0)
{
  
	// Input collections
	//declareProperty("SimCaloHitCollection", r_SimCaloCol, "Handle of the Input SimCaloHit collection");
  
	// Output collections
	//declareProperty("CaloHitCollection", w_DigiCaloCol, "Handle of Digi CaloHit collection");
	//declareProperty("CaloAssociationCollection", w_CaloAssociationCol, "Handle of CaloAssociation collection");
	//declareProperty("CaloMCPAssociationCollection", w_MCPCaloAssociationCol, "Handle of CaloAssociation collection");
   
}

StatusCode EcalDigiAlg::initialize()
{
  // --- Initialize input and output collections
  for(auto& simhit : name_SimCaloHit){
    if(!simhit.empty())
     _inputSimHitCollection.push_back( new SimCaloType(simhit, Gaudi::DataHandle::Reader, this) );
  }
  
  // --- Geometry service, cellID decoder
  m_geosvc = service<IGeomSvc>("GeomSvc");
  if ( !m_geosvc )  throw "EcalDigiAlg :Failed to find GeomSvc ...";
  m_dd4hep = m_geosvc->lcdd();
  if ( !m_dd4hep )  throw "EcalDigiAlg :Failed to get dd4hep::Detector ...";
  m_cellIDConverter = new dd4hep::rec::CellIDPositionConverter(*m_dd4hep);

  for(unsigned int i=0; i<name_Readout.size(); i++){
    if(name_Readout[i].empty()) continue;
    dd4hep::DDSegmentation::BitFieldCoder* tmp_decoder = m_geosvc->getDecoder(name_Readout[i]);
    if (!tmp_decoder) {
      error() << "Failed to get the decoder for: " << name_Readout[i] << endmsg;
      return StatusCode::FAILURE;
    }
    map_readout_decoder[name_SimCaloHit[i]] = tmp_decoder;
  }
  
  m_volumeManager = m_dd4hep->volumeManager();

  // --- Output collection
  for(auto& digihit : name_CaloHit){
    if(!digihit.empty())
      _outputHitCollection.push_back( new CaloType(digihit, Gaudi::DataHandle::Writer, this) );
  }

  for(auto& link : name_CaloAsso){
    if(!link.empty())
      _outputCaloSimAssoCol.push_back( new CaloSimAssoType(link, Gaudi::DataHandle::Writer, this) );
  }

  for(auto& link : name_CaloMCPAsso){
    if(!link.empty())
      _outputCaloMCPAssoCol.push_back( new CaloParticleAssoType(link, Gaudi::DataHandle::Writer, this) );
  }

  // --- Ntuple 
	if(_writeNtuple){
		std::string s_outfile = _filename;
		m_wfile = new TFile(s_outfile.c_str(), "recreate");
		t_SimCont = new TTree("SimStep", "SimStep");
		t_SimBar = new TTree("SimBarHit", "SimBarHit");
		t_SimCont->Branch("step_x", &m_step_x);
		t_SimCont->Branch("step_y", &m_step_y);
		t_SimCont->Branch("step_z", &m_step_z);
		t_SimCont->Branch("step_t", &m_step_t);			// yyy: time of each step
		t_SimCont->Branch("stepBar_x", &m_stepBar_x);
		t_SimCont->Branch("stepBar_y", &m_stepBar_y);
		t_SimCont->Branch("stepBar_z", &m_stepBar_z);
		t_SimCont->Branch("step_E", &m_step_E);
		t_SimCont->Branch("step_T1", &m_step_T1);
		t_SimCont->Branch("step_T2", &m_step_T2);
		t_SimBar->Branch("totE_Truth", &totE_Truth);
		t_SimBar->Branch("totE_Truth_MIP", &totE_Truth_MIP);
		t_SimBar->Branch("totE_Digi", &totE_Digi);
		t_SimBar->Branch("mean_CT", &mean_CT);
		t_SimBar->Branch("ECALTemp", &ECALTemp);
		t_SimBar->Branch("simBar_x", &m_simBar_x);
		t_SimBar->Branch("simBar_y", &m_simBar_y);
		t_SimBar->Branch("simBar_z", &m_simBar_z);
		t_SimBar->Branch("simBar_T1", &m_simBar_T1);
		t_SimBar->Branch("simBar_T2", &m_simBar_T2);
		t_SimBar->Branch("simBar_E_Truth", &m_simBar_E_Truth);
		t_SimBar->Branch("simBar_ChannelTemp", &m_simBar_ChannelTemp);
		t_SimBar->Branch("simBar_CryTID", &m_simBar_CryTID);
		t_SimBar->Branch("simBar_SiPMNIEL", &m_simBar_SiPMNIEL);
		t_SimBar->Branch("simBar_CryIntLY", &m_simBar_CryIntLY);
		t_SimBar->Branch("simBar_SiPMGain", &m_simBar_SiPMGain);
		t_SimBar->Branch("simBar_SiPMDCR", &m_simBar_SiPMDCR);
		t_SimBar->Branch("simBar_ScinGen", &m_simBar_Scint);
		t_SimBar->Branch("simBar_LO1", &m_simBar_LO1 );
		t_SimBar->Branch("simBar_NDC1", &m_simBar_NDC1 );
		t_SimBar->Branch("simBar_NDetPE1", &m_simBar_NDetPE1 );
		t_SimBar->Branch("simBar_Pedestal1", &m_simBar_Pedestal1 );
		t_SimBar->Branch("simBar_ADC1", &m_simBar_ADC1 );
		t_SimBar->Branch("simBar_ADCGain1", &m_simBar_ADCGain1 );
		t_SimBar->Branch("simBar_LO2", &m_simBar_LO2 );
		t_SimBar->Branch("simBar_NDC2", &m_simBar_NDC2 );
		t_SimBar->Branch("simBar_NDetPE2", &m_simBar_NDetPE2 );
		t_SimBar->Branch("simBar_Pedestal2", &m_simBar_Pedestal2 );
		t_SimBar->Branch("simBar_ADC2", &m_simBar_ADC2 );
		t_SimBar->Branch("simBar_ADCGain2", &m_simBar_ADCGain2 );
		t_SimBar->Branch("simBar_Q1_Att", &m_simBar_Q1_Att);
		t_SimBar->Branch("simBar_Q2_Att", &m_simBar_Q2_Att);
		t_SimBar->Branch("simBar_Q1_Digi", &m_simBar_Q1_Digi);
		t_SimBar->Branch("simBar_Q2_Digi", &m_simBar_Q2_Digi);
		t_SimBar->Branch("simBar_length", &m_simBar_length);
		t_SimBar->Branch("simBar_system", &m_simBar_system);
		t_SimBar->Branch("simBar_module", &m_simBar_module);
		t_SimBar->Branch("simBar_stave", &m_simBar_stave);
		t_SimBar->Branch("simBar_part", &m_simBar_part);
		t_SimBar->Branch("simBar_dlayer", &m_simBar_dlayer);
		t_SimBar->Branch("simBar_slayer", &m_simBar_slayer);
		t_SimBar->Branch("simBar_bar", &m_simBar_bar);
		t_SimBar->Branch("simBar_cellID", &m_simBar_cellID);
	}

  std::cout<<"Set Random seed: "<<_seed<<endl;
	rndm.SetSeed(_seed);

	// SiPM response function
  f_SiPMResponse = new TF1("f_SiPMResponse", "((1-[1])*[0]*(1-exp(-x/[0]))+[1]*x)*([2]+1)/( [2]+x / ([0]*(1-exp(-x/[0]))) )*(1+[3]*exp(-x/[0]))", 0, 1e+9);
  f_SiPMResponse->SetParameters(6.19783e+06, 5.08847e-01, 1.27705e+01, fEcalSiPMCT);
  f_SiPMSigmaDet = new TF1("f_SiPMSigmaDet", "8.336e-01*sqrt(x+0.2379)", 0, 1e+9);
  f_SiPMSigmaRecp = new TF1("f_SiPMSigmaRecp", "f_SiPMResponse+f_SiPMSigmaDet", 0, 1e+9);
  f_SiPMSigmaRecm = new TF1("f_SiPMSigmaRecm", "f_SiPMResponse-f_SiPMSigmaDet", 0, 1e+9);
  f_AsymGauss = new TF1("AsymGauss", [](double *x, double *par) -> double 
  {
      double val = x[0];
      double mean = par[0];
      double sigma_left = par[1];
      double sigma_right = par[2];

      if (val < mean) {
          return exp(-0.5 * pow((val - mean) / sigma_left, 2));
      } else {
          return exp(-0.5 * pow((val - mean) / sigma_right, 2));
      }
  }, -10, 10, 3);

	// SiPM noise model: Borel distribution
	f_DarkNoise = new TF1("f_DarkNoise", "pow([0]*x, x-1) * exp(-[0]*x) / TMath::Factorial(x)");
	f_DarkNoise->SetParameter(0, fEcalSiPMCT);

  //ADC non-linearity: a simplified quadratic function, with maximum non-linearity 1%
  f_ADCNonLin = new TF1("f_ADCNonLin", "pol2", 0, fADC);
  f_ADCNonLin->SetParameter(0, fADCNonLin/2.);
  f_ADCNonLin->SetParameter(1, -4*fADCNonLin/fADC);
  f_ADCNonLin->SetParameter(2, 4*fADCNonLin/fADC/fADC);

	g_CryLYRatio_vs_TID = new TGraph();
	float TID [8] = {50, 230.043, 731.681, 3448.96, 118953, 1.01164e+7, 1.02341e+8, 2.04907e+8};
	float CryLYRatio_TID[8] = {0.99, 0.774091, 0.689545, 0.633636, 0.588636, 0.549091, 0.401818, 0.374545};
	for (int i = 0; i < 8; i++) {
		g_CryLYRatio_vs_TID->SetPoint(i, TID[i], CryLYRatio_TID[i]);
	} 

	g_SiPMDCR_vs_NIEL = new TGraph();
	float NIEL[24] = { 9.9e+6,
		1.00000e+7, 2.49756e+7, 5.25408e+7, 1.10530e+8, 2.11374e+8,
		4.04227e+8, 6.63661e+8, 1.02902e+9, 1.68944e+9, 2.77373e+9,
		4.38351e+9, 6.41879e+9, 9.22154e+9, 1.40281e+10, 2.01534e+10,
		3.12481e+10, 4.48925e+10, 7.09466e+10, 1.33114e+11, 1.91238e+11,
		3.13975e+11, 4.51070e+11, 6.48029e+11
	};
	float DCR_NIEL[24] = { 1e+5,
		105925, 145378, 230409, 354813, 595662,
		1.09018e+6, 1.72783e+6, 2.58523e+6, 4.21697e+6, 6.68344e+6,
		1.02920e+7, 1.58489e+7, 2.44062e+7, 3.98107e+7, 5.95662e+7,
		9.44061e+7, 1.37246e+8, 2.17520e+8, 3.65174e+8, 5.01187e+8,
		7.28618e+8, 9.44061e+8, 1.18850e+9
	};
	for (int i = 0; i < 24; i++) {
		DCR_NIEL[i] = DCR_NIEL[i] / 1e+5; // unit: MHz/cm^2
		g_SiPMDCR_vs_NIEL->SetPoint(i, NIEL[i], DCR_NIEL[i]);
	}

	std::cout<<"EcalDigiAlg::initialize"<<std::endl;
	return Algorithm::initialize();
}

StatusCode EcalDigiAlg::execute()
{
	// clock_t yyy_start, yyy_enddigi;
	// yyy_start = clock(); // 记录开始时间

	if(_nEvt==0) std::cout<<"EcalDigiAlg::execute Start"<<std::endl;
	std::cout<<"Processing event: "<<_nEvt<<std::endl;
   	if(_nEvt<_Nskip){ _nEvt++; return StatusCode::SUCCESS; }

	Clear();
  totE_Truth=0;
  totE_Digi=0;

  for(int icol=0; icol<_inputSimHitCollection.size(); icol++){
    try{
      SimCaloType* r_SimCaloCol = _inputSimHitCollection[icol];
      CaloType* w_DigiCaloCol = _outputHitCollection[icol];
      CaloSimAssoType* w_CaloAssociationCol = _outputCaloSimAssoCol[icol];
      CaloParticleAssoType* w_MCPCaloAssociationCol = _outputCaloMCPAssoCol[icol];

      const edm4hep::SimCalorimeterHitCollection* SimHitCol =  r_SimCaloCol->get();
      edm4hep::CalorimeterHitCollection* caloVec = w_DigiCaloCol->createAndPut();
      auto* caloAssoVec = w_CaloAssociationCol->createAndPut();
      auto* caloMCPAssoVec = w_MCPCaloAssociationCol->createAndPut();
      std::vector<edm4hep::SimCalorimeterHit> m_simhitCol; m_simhitCol.clear();
      std::vector<CaloBar> m_barCol; m_barCol.clear(); 
      
      if(SimHitCol == 0){
      	std::cout<<"not found SimCalorimeterHitCollection"<< std::endl;
      	return StatusCode::SUCCESS;
      }
      
      	if(_Debug>=1) std::cout<<"digi, input sim hit size="<< SimHitCol->size() <<std::endl;
      

      totE_Truth=0;
      totE_Truth_MIP=0;
      totE_Digi=0;
      mean_CT = 0;
      for (int i = 1; i < 10; i++)
      {
      	mean_CT += (i-1)*f_DarkNoise->Eval(i);
      }
      float sEcalTempRef = rndm.Uniform(fEcalTempRef - fEcalTempFluc, fEcalTempRef + fEcalTempFluc);
      ECALTemp = sEcalTempRef;
      
      //Merge input simhit(steps) to real simhit(bar).
      MergeHits(*SimHitCol, m_simhitCol);
      if(_Debug>=1) std::cout<<"Finish Hit Merge, with Nhit: "<<m_simhitCol.size()<<std::endl;
      
      //Loop in SimHit, digitalize SimHit to DigiBar
      for(int i=0;i<m_simhitCol.size();i++)
      {
      	auto SimHit = m_simhitCol.at(i);
      	if(!SimHit.isAvailable()) {cout<<"Sim hit is not available"<<endl; continue;}
      	if(SimHit.getEnergy()==0) {cout<<"Sim hit energy is 0"<<endl; continue;}
      
      	totE_Truth += SimHit.getEnergy();
      	unsigned long long id = SimHit.getCellID();
      	CaloBar hitbar;
      	hitbar.setcellID( id);
        int id_part = -1; 
        int id_system = map_readout_decoder[name_Readout[icol]]->get(id, "system");
        if(id_system==29) id_part = map_readout_decoder[name_Readout[icol]]->get(id, "part"); //ECAL endcap
      	hitbar.setcellID(	map_readout_decoder[name_Readout[icol]]->get(id, "system"), 
      										map_readout_decoder[name_Readout[icol]]->get(id, "module"), 
      										map_readout_decoder[name_Readout[icol]]->get(id, "stave"), 
                          id_part, 
      										map_readout_decoder[name_Readout[icol]]->get(id, "dlayer"), 
      										map_readout_decoder[name_Readout[icol]]->get(id, "slayer"),
      										map_readout_decoder[name_Readout[icol]]->get(id, "bar"));
      	// double Lbar = GetBarLength(hitbar);  //NOTE: Is fixed with geometry LongCrystalBarBarrelCalorimeter32Polygon_v01.
      	double Lbar = GetBarLengthFromGeo(id);
        hitbar.setLength(Lbar);

      	dd4hep::Position hitpos = m_cellIDConverter->position(id);
      	TVector3 barpos(10*hitpos.x(), 10*hitpos.y(), 10*hitpos.z()); //cm to mm.
      	hitbar.setPosition(barpos);
      
      	//printf("in bar #%d: cellID [%d, %d, %d, %d], position (%.3f, %.3f, %.3f), energy %.3f \n", hitbar.getModule(), hitbar.getStave(), hitbar.getDlayer(), hitbar.getSlayer(), hitbar.getBar(), 
      	//       10*hitpos.x(), 10*hitpos.y(), 10*hitpos.z(), SimHit.getEnergy() );
      
      	MCParticleToEnergyWeightMap MCPEnMap; MCPEnMap.clear();
      	std::vector<HitStep> DigiLvec; DigiLvec.clear();
      	std::vector<HitStep> DigiRvec; DigiRvec.clear();
      	double totQ1_Truth = 0;
      	double totQ1_Att = 0;
      	double totQ2_Att = 0;
      	double totQ1_Digi = 0;
      	double totQ2_Digi = 0;
      	double totQ1 = 0;
      	double totQ2 = 0;
      
      	//Loop in all SimHitContribution(G4Step). 
      	//if(_Debug>=2) std::cout<<"SimHit contribution size: "<<SimHit.contributions_size()<<std::endl;
      	for(int iCont=0; iCont < SimHit.contributions_size(); ++iCont){
      		auto conb = SimHit.getContributions(iCont);
      		if( !conb.isAvailable() ) { std::cout<<"EcalDigiAlg  Can not get SimHitContribution: "<<iCont<<std::endl; continue;}
      
      		double en = conb.getEnergy();
      		if(en == 0) continue;
      		totQ1_Truth += en;
      
      		auto mcp = conb.getParticle();
      		MCPEnMap[mcp] += en;
      		TVector3 steppos(conb.getStepPosition().x, conb.getStepPosition().y, conb.getStepPosition().z);
      		TVector3 rpos = steppos-hitbar.getPosition();
      		float step_time = conb.getTime();		// yyy: step time
      
      		// m_step_x.push_back(steppos.x());
      		// m_step_y.push_back(steppos.y());
      		// m_step_z.push_back(steppos.z());
      		// m_step_t.push_back(step_time);			// yyy: push back step time
      		// m_step_E.push_back(en);
      		// m_stepBar_x.push_back(hitbar.getPosition().x());
      		// m_stepBar_y.push_back(hitbar.getPosition().y());
      		// m_stepBar_z.push_back(hitbar.getPosition().z());
      
      		if(_Debug>=3){
      			cout<<"Cell Pos: "<<hitbar.getPosition().x()<<'\t'<<hitbar.getPosition().y()<<'\t'<<hitbar.getPosition().z()<<endl;
      			cout<<"step pos: "<<steppos.x()<<'\t'<<steppos.y()<<'\t'<<steppos.z()<<endl;
      			cout<<"Relative pos: "<<rpos.x()<<'\t'<<rpos.y()<<'\t'<<rpos.z()<<endl;
      			cout<<"Cell: "<<hitbar.getModule()<<"  "<<hitbar.getDlayer()<<"  "<<hitbar.getSlayer()<<endl;
      		}
      
      		//Get digitalized signal(Q1, Q2, T1, T2) from step
      		//Define: 1 is left, 2 is right, clockwise direction in phi. 
      
      		int sign=-999;
          if(id_system == 20){ //ECAL barrel
        		if(hitbar.getSlayer()==1) sign = rpos.z()==0 ? 1 : rpos.z()/fabs(rpos.z());
        		else{
        			int _module = hitbar.getModule(); 
        			if(_module<=7 || _module>=25) sign = rpos.x()==0 ?  1: rpos.x()/fabs(rpos.x());
      	  		if(_module>=9 && _module<=23) sign = rpos.x()==0 ? -1:-rpos.x()/fabs(rpos.x());
      		  	else if(_module==8)  sign = rpos.y()==0 ?  1: rpos.y()/fabs(rpos.y());
      			  else if(_module==24) sign = rpos.y()==0 ? -1:-rpos.y()/fabs(rpos.y());
      		  }
          }
          else if(id_system == 29){ //ECAL endcaps
            sign = rpos.x()==0 ? 1 : rpos.x()/fabs(rpos.x());
          }
      		if(!fabs(sign)) {std::cout<<"ERROR: Wrong bar direction/position!"<<std::endl; continue;}
      
      		// ####### For Charge Digitization #######
      		// non-uniformity = 0
      		// double Ratio_left = exp(-(Lbar/2 + sign*sqrt(rpos.Mag2()))/Latt) / (exp(-(Lbar/2 + sign*sqrt(rpos.Mag2()))/Latt) + exp(-(Lbar/2 - sign*sqrt(rpos.Mag2()))/Latt));
      		// double Ratio_right = 1 - Ratio_left;
      		// totQ1_Truth += en*Ratio_left;
      		// totQ2_Truth += en*Ratio_right;
      
      		// non-uniformity != 0
      		double Ratio_left = exp(-(Lbar/2 + sign*sqrt(rpos.Mag2()))/Latt) / (2 * exp(-(Lbar/2)/Latt));
      		double Ratio_right = exp(-(Lbar/2 - sign*sqrt(rpos.Mag2()))/Latt) / (2 * exp(-(Lbar/2)/Latt));
      		totQ1_Att += en*Ratio_left;
      		totQ2_Att += en*Ratio_right;
      
      		// ####### For Time Digitization #######
      		double Qi_left = en*exp(-(Lbar/2 + sign*sqrt(rpos.Mag2()))/Latt);	
      		double Qi_right = en*exp(-(Lbar/2 - sign*sqrt(rpos.Mag2()))/Latt);
      
      		if(_Debug>=3){
      			cout<<Qi_left<<'\t'<<Qi_right<<endl;
      			cout<<Lbar<<'\t'<<sign*sqrt(rpos.Mag2())<<endl;
      		}
      
      		double Ti_left = -1; int looptime=0;
      		while(Ti_left<0){ 
      			// Ti_left = Tinit + rndm.Gaus(nMat*(Lbar/2 + sign*sqrt(rpos.Mag2()))/C, Tres); 
      			Ti_left = Tinit + rndm.Gaus(nMat*(Lbar/2 + sign*sqrt(rpos.Mag2()))/C, Tres) + step_time;  // yyy: add step time 
      			looptime++;
      			if(looptime>500){ std::cout<<"ERROR: Step "<<iCont<<" can not get a positive left-side time!"<<std::endl; break;}
      		}
      		if(looptime>500) continue;		
      		double Ti_right = -1; looptime=0;
      		while(Ti_right<0){ 
      			// Ti_right = Tinit + rndm.Gaus(nMat*(Lbar/2 - sign*sqrt(rpos.Mag2()))/C, Tres); 
      			Ti_right = Tinit + rndm.Gaus(nMat*(Lbar/2 - sign*sqrt(rpos.Mag2()))/C, Tres) + step_time;  // yyy: add step time 
      			looptime++;
      			if(looptime>500){ 
      			std::cout<<"ERROR: Step "<<iCont<<" can not get a positive right-side time!"<<std::endl; 
      			std::cout<<"  Initial time "<<Tinit<<", transport time central value "<<nMat*(Lbar/2 - sign*sqrt(rpos.Mag2()))/C<<std::endl;
      			break;
      			}
      		}
      		if(looptime>500) continue;		
      
      		m_step_T1.push_back(Ti_left);
      		m_step_T2.push_back(Ti_right);
      		totQ1 += Qi_left;
      		totQ2 += Qi_right;
      
      		HitStep stepoutL, stepoutR;
      		stepoutL.setQ(Qi_left); stepoutL.setT(Ti_left);
      		stepoutR.setQ(Qi_right); stepoutR.setT(Ti_right);
      		DigiLvec.push_back(stepoutL);
      		DigiRvec.push_back(stepoutR);
      	}
      
      	// #######################################
      	// ####### Ideal Time Digitization #######
      	// #######################################
      
      	//if(_Debug>=2) std::cout<<"Time Digitalize: time at Q >"<<_Qthfrac<<"*totQ"<<std::endl;
      	std::sort(DigiLvec.begin(), DigiLvec.end());
      	std::sort(DigiRvec.begin(), DigiRvec.end());
      	double thQ1=0;
      	double thQ2=0;
      	double thT1, thT2; 
      	for(int iCont=0;iCont<DigiLvec.size();iCont++){
      		thQ1 += DigiLvec[iCont].getQ();
      		if(thQ1>totQ1*_Qthfrac){ 
      			thT1 = DigiLvec[iCont].getT(); 
      			if(_Debug>=3) std::cout<<"Get T1 at index: "<<iCont<<std::endl;
      			break;
      		}
      	}
      	for(int iCont=0;iCont<DigiRvec.size();iCont++){
      		thQ2 += DigiRvec[iCont].getQ();
      		if(thQ2>totQ2*_Qthfrac){ 
      			thT2 = DigiRvec[iCont].getT(); 
      			if(_Debug>=3) std::cout<<"Get T2 at index: "<<iCont<<std::endl;
      			break;
      		}
      	}
      
      	int ScinGen; 
      	int outLO1, outNDC1, outNDetPE1, outLO2, outNDC2, outNDetPE2;
      	float outADC1, outADCGain1, outPedestal1, outPedestal2, outADC2, outADCGain2;
      	float ChannelTemp, CryTID, SiPMNIEL, CryIntLY, SiPMGain, SiPMDCR;
      	if(_UseRelDigi){
      		// #############################################
      		// ####### Realistic Charge Digitization #######
      		// #############################################
      
      		//TODO: fEcalMIPEnergy should depends on crystal size. 
          //TODO: Barrel and endcap should use different temperature map. 
      		int layer = (hitbar.getDlayer() - 1) * 2 + hitbar.getSlayer();
      		float temp_dif = (27 - layer) * fEcalTempGrad + sEcalTempRef - fEcalTempRef;// compared with fEcalTempRef
      		ChannelTemp = temp_dif + fEcalTempRef;
      		CryTID = fEcalCryTID;
      		SiPMNIEL = fEcalSiPMNIEL;
      		// float sEcalCryIntLY = rndm.Gaus(fEcalCryIntLY, fEcalCryIntLYFlu * fEcalCryIntLY);
      		float sEcalCryIntLY = fEcalCryIntLY;
      		float sEcalSiPMGainMean = fEcalSiPMGainMean;
      		float sEcalSiPMDCR = fEcalSiPMDCR;
      
      		// Effects on crystal 
      		if(fUseCryTID && fEcalCryTID >= 50){
      			sEcalCryIntLY = sEcalCryIntLY * g_CryLYRatio_vs_TID->Eval(fEcalCryTID);
      		}
      		if(fUseCryTemp) {
      			sEcalCryIntLY = sEcalCryIntLY * (1 + temp_dif * fEcalBGOTempCoef);
      		}
      		CryIntLY = sEcalCryIntLY;
      		if(fUseDigiScint){
      			ScinGen = std::round(rndm.Poisson((totQ1_Att+totQ2_Att) * 1000 * sEcalCryIntLY));
      		}
      		else{
      			ScinGen = (totQ1_Att+totQ2_Att) * 1000 * sEcalCryIntLY;
      		}
      //cout<<endl;
      //cout<<"In Hit #"<<i<<": truth energy "<<totQ1_Att+totQ2_Att<<", used intrinsic LY "<<sEcalCryIntLY<<", ph "<<ScinGen;
      
      		// Remove corrections
      		if(fUseCryTemp && !fUseCryTempCor) {
      			sEcalCryIntLY = sEcalCryIntLY / (1 + temp_dif * fEcalBGOTempCoef);
      		}
      		if((fUseCryTID && fEcalCryTID >= 50) && !fUseCryTIDCor) {
      			sEcalCryIntLY = sEcalCryIntLY / g_CryLYRatio_vs_TID->Eval(fEcalCryTID);
      		}
      
      		// Effects on SiPM
      		if(fUseSiPMNIEL && fEcalSiPMNIEL >= 1e+7) {
      			sEcalSiPMDCR = sEcalSiPMDCR * g_SiPMDCR_vs_NIEL->Eval(fEcalSiPMNIEL);
      		}
      		if(fUseSiPMTemp) {
      			sEcalSiPMGainMean = sEcalSiPMGainMean * (1 + temp_dif * fEcalSiPMGainTempCoef);
      			sEcalSiPMDCR = sEcalSiPMDCR * pow(10, fEcalSiPMDCRTempCoef * temp_dif);
      		}
      		SiPMGain = sEcalSiPMGainMean;
      		SiPMDCR = sEcalSiPMDCR;
      		// Remove corrections
      		if(fUseSiPMTemp && ! fUseSiPMTempCor) {
      			sEcalSiPMGainMean = sEcalSiPMGainMean / (1 + temp_dif * fEcalSiPMGainTempCoef);
      		}
      		// if((fUseSiPMNIEL && fEcalSiPMNIEL >= 1e+7) && !fUseSiPMNIELCor) {
      		// }
      //cout<<", SiPM gain "<<sEcalSiPMGainMean<<endl;
      
      		totQ1_Digi = EnergyDigi(ScinGen*totQ1_Att/(totQ1_Att+totQ2_Att), sEcalCryIntLY, sEcalSiPMGainMean, sEcalSiPMDCR, 
      								f_SiPMResponse, f_SiPMSigmaDet, f_SiPMSigmaRecp, f_SiPMSigmaRecm, f_AsymGauss, f_DarkNoise, f_ADCNonLin,
      								outLO1, outNDC1, outNDetPE1, outPedestal1, outADC1, outADCGain1)/1000;
      		totQ2_Digi = EnergyDigi(ScinGen*totQ2_Att/(totQ1_Att+totQ2_Att), sEcalCryIntLY, sEcalSiPMGainMean, sEcalSiPMDCR, 
      								f_SiPMResponse, f_SiPMSigmaDet, f_SiPMSigmaRecp, f_SiPMSigmaRecm, f_AsymGauss, f_DarkNoise, f_ADCNonLin,
      								outLO2, outNDC2, outNDetPE2, outPedestal2, outADC2, outADCGain2)/1000;
      	}
      	else{
      		if( (totQ1_Att+totQ2_Att)!=0 ){
      			totQ1_Digi = totQ1_Att;
      			totQ2_Digi = totQ2_Att;
      		}
      		if( totQ1_Digi*1000./fEcalMIPEnergy < fEcalMIP_Thre ) totQ1_Digi = 0;
      		if( totQ2_Digi*1000./fEcalMIPEnergy < fEcalMIP_Thre ) totQ2_Digi = 0;
      	}
      
      	// cout<<"bar truth Q1: "<<totQ1_Att*1000<<",\t bar digi  Q1: "<<totQ1_Digi*1000<<endl;
      	if(totQ1_Digi==0 && totQ2_Digi==0) continue;
      
      	hitbar.setQ(totQ1_Digi, totQ2_Digi);
      	hitbar.setT(thT1, thT2);
      	// cout<<"bar thT1: "<<thT1<<",\t bar thT2: "<<thT2<<endl;
      
      	// ##################################
      	// ####### Some associations  #######
      	// ##################################
      
      	//2 hits with double-readout time. 
      	edm4hep::Vector3f m_pos(hitbar.getPosition().X(), hitbar.getPosition().Y(), hitbar.getPosition().Z());
      	auto digiHit1 = caloVec->create();
      	digiHit1.setCellID(hitbar.getcellID());
      	digiHit1.setEnergy(hitbar.getQ1());
      	digiHit1.setTime(hitbar.getT1());
      	digiHit1.setPosition(m_pos);
      	auto digiHit2 = caloVec->create();
      	digiHit2.setCellID(hitbar.getcellID());
      	digiHit2.setEnergy(hitbar.getQ2());
      	digiHit2.setTime(hitbar.getT2());
      	digiHit2.setPosition(m_pos);
      
      
      	//SimHit - CaloHit association
      	auto rel1 = caloAssoVec->create();
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
      	rel1.setFrom(digiHit1);
      	rel1.setTo(SimHit);
#else
      	rel1.setRec(digiHit1);
      	rel1.setSim(SimHit);
#endif
      	rel1.setWeight( hitbar.getQ1()/(hitbar.getQ1()+hitbar.getQ2()) );
      	auto rel2 = caloAssoVec->create();
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
      	rel2.setFrom(digiHit2);
      	rel2.setTo(SimHit);
#else
      	rel2.setRec(digiHit2);
      	rel2.setSim(SimHit);
#endif
      	rel2.setWeight( hitbar.getQ2()/(hitbar.getQ1()+hitbar.getQ2()) );
      
      
      	//MCParticle - CaloHit association
      	//float maxMCE = -99.;
      	//edm4hep::MCParticle selMCP; 
      	for(auto iter : MCPEnMap){
      	//if(iter.second>maxMCE){
      	//  maxMCE = iter.second;
      	//  selMCP = iter.first;
      	//}
      	auto rel_MCP1 = caloMCPAssoVec->create();
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
      	rel_MCP1.setFrom(digiHit1);
      	rel_MCP1.setTo(iter.first);
#else
      	rel_MCP1.setRec(digiHit1);
      	rel_MCP1.setSim(iter.first);
#endif
      	rel_MCP1.setWeight(iter.second/SimHit.getEnergy());
      	auto rel_MCP2 = caloMCPAssoVec->create();
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
      	rel_MCP2.setFrom(digiHit2);
      	rel_MCP2.setTo(iter.first);
#else
      	rel_MCP2.setRec(digiHit2);
      	rel_MCP2.setSim(iter.first);
#endif
      	rel_MCP2.setWeight(iter.second/SimHit.getEnergy());      
      	}
      
      
      	//if(selMCP.isAvailable()){
      	//  auto rel_MCP1 = caloMCPAssoVec->create();
      	//  rel_MCP1.setRec(digiHit1);
      	//  rel_MCP1.setSim(selMCP);
      	//  rel_MCP1.setWeight(1.);
      	//  auto rel_MCP2 = caloMCPAssoVec->create();
      	//  rel_MCP2.setRec(digiHit2);
      	//  rel_MCP2.setSim(selMCP);
      	//  rel_MCP2.setWeight(1.);
      	//}
      
      	// ########################################
      	// ####### Temp: write into trees.  #######
      	// ########################################
      	
        	m_barCol.push_back(hitbar);
      	if(hitbar.getQ1()>=0 && hitbar.getQ2()>=0) totE_Digi += (hitbar.getQ1() + hitbar.getQ2());
        if (totQ1_Att > (0.001 * fEcalMIPEnergy * fEcalMIP_Thre) && totQ2_Att > (0.001 * fEcalMIPEnergy * fEcalMIP_Thre))
        {
//            cout << "Truth Energy:" << totQ1_Att << "   " << totQ2_Att << endl;
            totE_Truth_MIP += (totQ1_Att + totQ2_Att);
        }
      
      
      	if(_writeNtuple){
      		m_simBar_x.push_back(hitbar.getPosition().x());
      		m_simBar_y.push_back(hitbar.getPosition().y());
      		m_simBar_z.push_back(hitbar.getPosition().z());
      		m_simBar_E_Truth.push_back(totQ1_Truth);
      		m_simBar_Scint.push_back(ScinGen);
      		m_simBar_ChannelTemp.push_back(ChannelTemp);
      		m_simBar_CryTID.push_back(CryTID);
      		m_simBar_SiPMNIEL.push_back(SiPMNIEL);
      		m_simBar_CryIntLY.push_back(CryIntLY);
      		m_simBar_SiPMGain.push_back(SiPMGain);
      		m_simBar_SiPMDCR.push_back(SiPMDCR);
      		m_simBar_LO1.push_back(outLO1);
      		m_simBar_LO2.push_back(outLO2);
      		m_simBar_NDC1.push_back(outNDC1);
      		m_simBar_NDC2.push_back(outNDC2);
      		m_simBar_NDetPE1.push_back(outNDetPE1);
      		m_simBar_NDetPE2.push_back(outNDetPE2);
      		m_simBar_Pedestal1.push_back(outPedestal1);
      		m_simBar_Pedestal2.push_back(outPedestal2);
      		m_simBar_ADC1.push_back(outADC1);
      		m_simBar_ADC2.push_back(outADC2);
      		m_simBar_ADCGain1.push_back(outADCGain1);
      		m_simBar_ADCGain2.push_back(outADCGain2);
      		m_simBar_Q1_Att.push_back(totQ1_Att);
      		m_simBar_Q2_Att.push_back(totQ2_Att);
      		m_simBar_Q1_Digi.push_back(hitbar.getQ1());
      		m_simBar_Q2_Digi.push_back(hitbar.getQ2());
      		m_simBar_T1.push_back(hitbar.getT1());
      		m_simBar_T2.push_back(hitbar.getT2());
          m_simBar_length.push_back(hitbar.getLength());
          m_simBar_system.push_back(hitbar.getSystem());
      		m_simBar_module.push_back(hitbar.getModule());
      		m_simBar_stave.push_back(hitbar.getStave());
      		m_simBar_part.push_back(hitbar.getPart());
      		m_simBar_dlayer.push_back(hitbar.getDlayer());
      		m_simBar_slayer.push_back(hitbar.getSlayer());
          m_simBar_bar.push_back(hitbar.getBar());
      		m_simBar_cellID.push_back(hitbar.getcellID());
      	}
      }

  	  m_simhitCol.clear();
    }catch(GaudiException &e){
      info()<<"SimCaloHit collection "<<name_SimCaloHit[icol]<<" is not available "<<endmsg;
    }
  }

	if(_writeNtuple){
		// t_SimCont->Fill();
		t_SimBar->Fill();
	}

	if(_Debug>=1) 
	{
		std::cout<<"End Loop: Bar Digitalization!"<<std::endl;
		std::cout<<"Total Truth Energy: "<<totE_Truth<<std::endl;
        std::cout << "Total Truth Energy with " << (double) fEcalMIP_Thre << " MIP Threshold: " << totE_Truth_MIP << std::endl;
		std::cout<<"Total Digi Energy: "<<totE_Digi<<std::endl;
	}

	// yyy_enddigi = clock();
	// double duration_digi = double(yyy_enddigi - yyy_start) / CLOCKS_PER_SEC;
	// // 将时间输出到txt文件中
	// std::ofstream outfile("runtime_ecaldigi.txt", std::ios::app);
	// outfile << _nEvt << "    " << duration_digi << std::endl;
	// outfile.close();

  	_nEvt ++ ;
  	//delete SimHitCol, caloVec, caloAssoVec; 
	return StatusCode::SUCCESS;
}

StatusCode EcalDigiAlg::finalize()
{
	if(_writeNtuple){
  		m_wfile->cd();
	  	//t_SimCont->Write();
  		t_SimBar->Write();
	  	m_wfile->Close();
    	delete m_wfile, t_SimCont, t_SimBar; 
  	}
	info() << "Processed " << _nEvt << " events " << endmsg;
  map_readout_decoder.clear();
	delete m_cellIDConverter, m_geosvc;
	delete f_SiPMResponse, f_SiPMSigmaDet, f_SiPMSigmaRecp, f_SiPMSigmaRecm, f_AsymGauss, f_DarkNoise, f_ADCNonLin, g_SiPMDCR_vs_NIEL, g_CryLYRatio_vs_TID;
	return Algorithm::finalize();
}

float EcalDigiAlg::EnergyDigi(float ScinGen, float sEcalCryIntLY, float sEcalSiPMGainMean, float sEcalSiPMDCR, 
								TF1* f_SiPMResponse, TF1* f_SiPMSigmaDet, TF1* f_SiPMSigmaRecp, TF1* f_SiPMSigmaRecm, TF1* f_AsymGauss, TF1* f_DarkNoise, TF1* f_ADCNonLin, 
								int& outLO, int& outNDC, int& outNDetPE, float& outPedestal, float& outADC, float& outADCGain)
{
	// float sEcalSiPMPDE = rndm.Gaus(fEcalSiPMPDE, fEcalSiPMPDEFlu * fEcalSiPMPDE);
	float sEcalSiPMPDE = fEcalSiPMPDE;
  float fEcalCryAtt = fEcalCryMipLY/(fEcalCryIntLY*fEcalSiPMPDE*fEcalMIPEnergy);
	int sEcalCryAttLO = std::round(ScinGen * fEcalCryAtt * sEcalSiPMPDE);
	if (sEcalCryAttLO < 0) sEcalCryAttLO = 0;
	float sEcalCaliMIPLY = sEcalCryIntLY * fEcalCryAtt * sEcalSiPMPDE * fEcalMIPEnergy;
	// sEcalCaliMIPLY = rndm.Gaus(sEcalCaliMIPLY, fEcalCryLYUn * sEcalCaliMIPLY);
	outLO = sEcalCryAttLO;
//cout<<"  Real Digi: output Light "<<sEcalCryAttLO<<" = round ("<<ScinGen<<" * "<<fEcalCryAtt<<" * "<<sEcalSiPMPDE<<")";	

	int sNLin = sEcalCryAttLO;
	int sNDet = 0;
	float sNDetMean = 0;
	float sNDetSigma = 0;
	if(fSiPMDigiVerbose==0 || sNLin<100)
	{
		sNDetMean = sNLin;
		//sNDetSigma = f_SiPMSigmaDet->Eval(sNDetMean);
    sNDetSigma = 0;
		sNDet = std::round(rndm.Gaus(sNDetMean, sNDetSigma));
	}
	else
	{
		sNDetMean = f_SiPMResponse->Eval(sNLin);
		sNDetSigma = f_SiPMSigmaDet->Eval(sNDetMean);
		sNDet = std::round(rndm.Gaus(sNDetMean, sNDetSigma));
	}
	//SiPM dark noise: dark counts and acossiated crosstalks, the crossstalk follows the Borel distribution
	int darkcounts_mean = rndm.Poisson(sEcalSiPMDCR * fEcalTimeInterval);
	int darkcounts_CT = 0;
	for(int i=0;i<darkcounts_mean;i++)
	{
		double darkcounts_rdm = rndm.Uniform(0, 1);
		int sum_darkcounts = 1;
		if(! (darkcounts_rdm <= f_DarkNoise->Eval(sum_darkcounts)))
		{
			float prob = f_DarkNoise->Eval(sum_darkcounts);
			while(darkcounts_rdm > prob)
			{
				sum_darkcounts++;
				prob += f_DarkNoise->Eval(sum_darkcounts);
			}
		}
		darkcounts_CT += sum_darkcounts;
	}
	outNDC = darkcounts_CT;
	sNDet += darkcounts_CT;
	if (sNDet < 0) sNDet = 0;
	outNDetPE = sNDet;

//cout<<", SiPM pixel "<<sNDet<<", dark count + Xtalk "<<darkcounts_CT;

	Bool_t Use_G1 = kFALSE;
	Bool_t Use_G2 = kFALSE;
	Bool_t Use_G3 = kFALSE;
	// sEcalSiPMGainMean = rndm.Gaus(sEcalSiPMGainMean, sEcalSiPMGainMean * fEcalSiPMGainMeanFlu);
	float sEcalSiPMGainSigma = sEcalSiPMGainMean * fEcalSiPMGainSigma;
	float sPedestal = fPedestal;
	float sEcalFEENoiseSigma = fEcalFEENoiseSigma;
	float sEcalASICNoiseSigma = fEcalASICNoiseSigma;
	float sADCMean = sNDet * sEcalSiPMGainMean + sPedestal;
	float sADCSigma = std::sqrt(sNDet * sEcalSiPMGainSigma * sEcalSiPMGainSigma + sEcalFEENoiseSigma * sEcalFEENoiseSigma + fEcalASICNoiseSigma * fEcalASICNoiseSigma);
	float sADC = -1;
	sADC = std::round(rndm.Gaus(sADCMean, sADCSigma));
	if(sADC < 0) sADC = 0;
	outPedestal = sPedestal;
	outADC = sADC;

//cout<<", raw ADC "<<sADC<<" = Gaus ( "<<sADCMean<<" +- "<<sADCSigma<<" )"<<endl;
//cout<<"    ADC mean = "<<sNDet<<" * "<<sEcalSiPMGainMean<<" + "<<sPedestal;
//cout<<", ADC sigma = sqrt("<<sNDet<<" * "<<sEcalSiPMGainSigma<<"^2 + "<<sEcalFEENoiseSigma<<"^2 + "<<fEcalASICNoiseSigma.value()<<"^2 )"<<endl;
	
	if(sADC <= fADCSwitch)
	{
		Use_G1 = kTRUE;

		if(fSiPMDigiVerbose==2 && sNDet>=100)
		{
			float sNRecMean = f_SiPMResponse->GetX((sADC-sPedestal)/sEcalSiPMGainMean);
			float sNRecSigma = sNDetSigma;
			float NRec = rndm.Gaus(sNRecMean, sNRecSigma);
			sADC = NRec * sEcalSiPMGainMean + sPedestal;
		}
		else if(fSiPMDigiVerbose==3 && sNDet>=100)
		{
			float sNADCDet = (sADC - sPedestal) / sEcalSiPMGainMean;
			float sNRecMean = f_SiPMResponse->GetX(sNADCDet);
			float sNRecSigmap = sNRecMean - f_SiPMSigmaRecp->GetX(sNADCDet);
			float sNRecSigmam = f_SiPMSigmaRecm->GetX(sNADCDet) - sNRecMean;
			f_AsymGauss->SetRange(sNRecMean-3*sNRecSigmap, sNRecMean+3*sNRecSigmam);
			f_AsymGauss->SetParameters(sNRecMean, sNRecSigmap, sNRecSigmam);
			float NRec = f_AsymGauss->GetRandom();
			sADC = NRec * sEcalSiPMGainMean + sPedestal;
		}
    sADC = (f_ADCNonLin->Eval(sADC)+1) * sADC;
		outPedestal = sPedestal;
		outADCGain = sADC;
		sPedestal = fPedestal + sEcalSiPMDCR * fEcalTimeInterval * (1 + mean_CT) * sEcalSiPMGainMean;

        // float sEcalCaliGainMean = rndm.Gaus(sEcalSiPMGainMean, fEcalSiPMGainUn * sEcalSiPMGainMean);
		// float sMIP = (sADC - sPedestal) / sEcalCaliGainMean / sEcalCaliMIPLY;
		float sMIP = (sADC - sPedestal) / sEcalSiPMGainMean / sEcalCaliMIPLY;

//cout<<", after gain "<<sADC<<", Nmip "<<sMIP<<" = ("<<sADC<<" - "<<sPedestal<<") / "<<sEcalSiPMGainMean<<" / "<<sEcalCaliMIPLY<<endl;

		if(sMIP < fEcalMIP_Thre) return 0;
		return sMIP * fEcalMIPEnergy;
	}
	else if(sADC > fADCSwitch && int(sADC/fGainRatio_12) <= fADCSwitch)
	{
		Use_G2 = kTRUE;
		sEcalSiPMGainMean = sEcalSiPMGainMean / fGainRatio_12;
		sPedestal = fPedestal;
		sEcalSiPMGainSigma = sEcalSiPMGainMean * fEcalSiPMGainSigma;
		sEcalFEENoiseSigma = fEcalFEENoiseSigma / fGainRatio_12;

		sADCMean = sNDet * sEcalSiPMGainMean + sPedestal;
		sADCSigma = std::sqrt(sNDet * sEcalSiPMGainSigma * sEcalSiPMGainSigma + fEcalASICNoiseSigma * fEcalASICNoiseSigma + sEcalFEENoiseSigma * sEcalFEENoiseSigma);
		sADC = std::round(rndm.Gaus(sADCMean, sADCSigma));
		if(sADC < 0) sADC = 0;

		if(fSiPMDigiVerbose==2)
		{
			float sNRecMean = f_SiPMResponse->GetX((sADC-sPedestal)/sEcalSiPMGainMean);
			float sNRecSigma = sNDetSigma;
			float NRec = rndm.Gaus(sNRecMean, sNRecSigma);
			sADC = NRec * sEcalSiPMGainMean + sPedestal;
		}
		else if(fSiPMDigiVerbose==3)
		{
			float sNADCDet = (sADC - sPedestal) / sEcalSiPMGainMean;
			float sNRecMean = f_SiPMResponse->GetX(sNADCDet);
			float sNRecSigmap = sNRecMean - f_SiPMSigmaRecp->GetX(sNADCDet);
			float sNRecSigmam = f_SiPMSigmaRecm->GetX(sNADCDet) - sNRecMean;
			f_AsymGauss->SetRange(sNRecMean-3*sNRecSigmap, sNRecMean+3*sNRecSigmam);
			f_AsymGauss->SetParameters(sNRecMean, sNRecSigmap, sNRecSigmam);
			float NRec = f_AsymGauss->GetRandom();
			sADC = NRec * sEcalSiPMGainMean + sPedestal;
		}
    sADC = (f_ADCNonLin->Eval(sADC)+1) * sADC;
		outPedestal = sPedestal;
		outADCGain = sADC;
		sPedestal = fPedestal + sEcalSiPMDCR * fEcalTimeInterval * (1 + mean_CT) * sEcalSiPMGainMean;

        // float sEcalCaliGainMean = rndm.Gaus(sEcalSiPMGainMean, fEcalSiPMGainUn * sEcalSiPMGainMean);
		// float sMIP = (sADC - sPedestal) / sEcalCaliGainMean / sEcalCaliMIPLY;
		float sMIP = (sADC - sPedestal) / sEcalSiPMGainMean / sEcalCaliMIPLY;
//cout<<", after gain "<<sADC<<", Nmip "<<sMIP<<endl;

		if(sMIP < fEcalMIP_Thre) return 0;
		return sMIP * fEcalMIPEnergy;
	}
	else if(int(sADC/fGainRatio_12) > fADCSwitch)
	{
		Use_G3 = kTRUE;
		sEcalSiPMGainMean = sEcalSiPMGainMean / fGainRatio_12 / fGainRatio_23;
		sPedestal = fPedestal;
		sEcalSiPMGainSigma = sEcalSiPMGainMean * fEcalSiPMGainSigma;
		sEcalFEENoiseSigma = fEcalFEENoiseSigma / fGainRatio_12 / fGainRatio_23;

		sADCMean = sNDet * sEcalSiPMGainMean + sPedestal;
		sADCSigma = std::sqrt(sNDet * sEcalSiPMGainSigma * sEcalSiPMGainSigma + fEcalASICNoiseSigma * fEcalASICNoiseSigma + sEcalFEENoiseSigma * sEcalFEENoiseSigma);
		sADC = std::round(rndm.Gaus(sADCMean, sADCSigma));
		if(sADC < 0) sADC = 0;

		if (sADC > fADC-1)
		{
			sADC = fADC-1;
		}

		if(fSiPMDigiVerbose==2)
		{
			float sNRecMean = f_SiPMResponse->GetX((sADC-sPedestal)/sEcalSiPMGainMean);
			float sNRecSigma = sNDetSigma;
			float NRec = rndm.Gaus(sNRecMean, sNRecSigma);
			sADC = NRec * sEcalSiPMGainMean + sPedestal;
		}
		else if(fSiPMDigiVerbose==3)
		{
			float sNADCDet = (sADC - sPedestal) / sEcalSiPMGainMean;
			float sNRecMean = f_SiPMResponse->GetX(sNADCDet);
			float sNRecSigmap = sNRecMean - f_SiPMSigmaRecp->GetX(sNADCDet);
			float sNRecSigmam = f_SiPMSigmaRecm->GetX(sNADCDet) - sNRecMean;
			f_AsymGauss->SetRange(sNRecMean-3*sNRecSigmap, sNRecMean+3*sNRecSigmam);
			f_AsymGauss->SetParameters(sNRecMean, sNRecSigmap, sNRecSigmam);
			float NRec = f_AsymGauss->GetRandom();
			sADC = NRec * sEcalSiPMGainMean + sPedestal;
		}
    sADC = (f_ADCNonLin->Eval(sADC)+1) * sADC;
		outPedestal = sPedestal;
		outADCGain = sADC;
		sPedestal = fPedestal + sEcalSiPMDCR * fEcalTimeInterval * (1 + mean_CT) * sEcalSiPMGainMean;

        // float sEcalCaliGainMean = rndm.Gaus(sEcalSiPMGainMean, fEcalSiPMGainUn * sEcalSiPMGainMean);
		// float sMIP = (sADC - sPedestal) / sEcalCaliGainMean / sEcalCaliMIPLY;
		float sMIP = (sADC - sPedestal) / sEcalSiPMGainMean / sEcalCaliMIPLY;
//cout<<", after gain "<<sADC<<", Nmip "<<sMIP<<endl;
		if(sMIP < fEcalMIP_Thre) return 0;
		return sMIP * fEcalMIPEnergy;
	}
}

StatusCode EcalDigiAlg::MergeHits( const edm4hep::SimCalorimeterHitCollection& m_col, std::vector<edm4hep::SimCalorimeterHit>& m_hits ){

  m_hits.clear(); 
	std::vector<edm4hep::MutableSimCalorimeterHit> m_mergedhit;
	m_mergedhit.clear();

	for(int iter=0; iter<m_col.size(); iter++){
		edm4hep::SimCalorimeterHit m_step = m_col[iter];
		if(!m_step.isAvailable()){ cout<<"ERROR HIT!"<<endl; continue;}
		if(m_step.getEnergy()==0 || m_step.contributions_size()<1) continue;
		unsigned long long cellid = m_step.getCellID();
		dd4hep::Position hitpos = m_cellIDConverter->position(cellid);
		edm4hep::Vector3f pos(hitpos.x()*10, hitpos.y()*10, hitpos.z()*10);

		edm4hep::MutableCaloHitContribution conb;
		conb.setEnergy(m_step.getEnergy());
		conb.setStepPosition(m_step.getPosition());
		conb.setParticle( m_step.getContributions(0).getParticle() );
		conb.setTime(m_step.getContributions(0).getTime());

		edm4hep::MutableSimCalorimeterHit m_hit = find(m_mergedhit, cellid);
		if(m_hit.getCellID()==0){
			m_hit.setCellID(cellid);
			m_hit.setPosition(pos);
			m_mergedhit.push_back(m_hit);
		}
		m_hit.addToContributions(conb);
		m_hit.setEnergy(m_hit.getEnergy()+m_step.getEnergy());
	}

  for(auto iter = m_mergedhit.begin(); iter!=m_mergedhit.end(); iter++){
    edm4hep::SimCalorimeterHit constsimhit = *iter; 
    m_hits.push_back( constsimhit );  
  }
  return StatusCode::SUCCESS; 
}

double EcalDigiAlg::GetBarLength(CaloBar& bar){
    //TODO: reading bar length from geosvc. 
    if(bar.getSlayer()==1) return 374.667;
    else{
        if(bar.getModule()%2 == 0){
            // return 288 + (bar.getDlayer()-1)*12.7080;
						return 288 + (bar.getDlayer()-1)*19.062;   // TODO: Get the correct length from the geometry
        }
        else{
            // return 409 - (bar.getDlayer()-1)*4.6670;
						return 409 - (bar.getDlayer()-1)*7.001;    // TODO: Get the correct length from the geometry
        }
        
    }
}

double EcalDigiAlg::GetBarLengthFromGeo(unsigned long long id){

	dd4hep::PlacedVolume ipv = m_volumeManager.lookupVolumePlacement(id);
	dd4hep::Volume ivol = ipv.volume();
	std::vector< double > iVolParam = ivol.solid().dimensions();
	// cout<<"iVolParam: "<<iVolParam.size()<<" "<<iVolParam[0]<<" "<<iVolParam[1]<<" "<<iVolParam[2]<<endl;
	auto maxElement = std::max_element(iVolParam.begin(), iVolParam.end());
	// std::cout << "bar length: " << *maxElement * 20 << std::endl; 
	iVolParam.clear();
	return *maxElement * 20; // mm
}

/*
dd4hep::Position EcalDigiAlg::GetCellPos(dd4hep::Position& pos, CaloBar& bar){
	dd4hep::Position rpos = pos-bar.getPosition();
	TVector3 vec(0,0,0); 
	if(bar.getSlayer()==1) vec.SetXYZ(0, 0, floor(rpos.z()/10)*10+5 );
	else if(bar.getSlayer()==0){
		if((bar.getModule()==0||bar.getModule()==4) && bar.getDlayer()%2==1) vec.SetXYZ(floor(rpos.x()/10)*10+5,0,0);
		if((bar.getModule()==0||bar.getModule()==4) && bar.getDlayer()%2==0) vec.SetXYZ(floor((rpos.x()-5)/10)*10+10,0,0);
		if((bar.getModule()==2||bar.getModule()==6) && bar.getDlayer()%2==1) vec.SetXYZ(0, floor(rpos.y()/10)*10+5,0);
		if((bar.getModule()==2||bar.getModule()==6) && bar.getDlayer()%2==0) vec.SetXYZ(0, floor((rpos.y()-5)/10)*10+10,0);
		if(bar.getModule()==1 || bar.getModule()==5){
			TVector3 unitv(1./sqrt(2), -1./sqrt(2), 0);
			if(bar.getDlayer()%2==1) vec = (floor(rpos.Dot(unitv)/10)*10+5)*unitv;
			if(bar.getDlayer()%2==0) vec = (floor((rpos.Dot(unitv)-5)/10)*10+10)*unitv;
		}
		if(bar.getModule()==3 || bar.getModule()==7){
			TVector3 unitv(1./sqrt(2), 1./sqrt(2), 0);
			if(bar.getDlayer()%2==1) vec = (floor(rpos.Dot(unitv)/10)*10+5)*unitv;
			if(bar.getDlayer()%2==0) vec = (floor((rpos.Dot(unitv)-5)/10)*10+10)*unitv;
		}
	}
	dd4hep::Position relv(vec.x(), vec.y(), vec.z());
	return relv+bar.getPosition();
}


edm4hep::MutableSimCalorimeterHit EcalDigiAlg::find(edm4hep::SimCalorimeterHitCollection& m_col, dd4hep::Position& pos){
   for(int i=0;i<m_col.size();i++){
    edm4hep::MutableSimCalorimeterHit hit = m_col[i];
		dd4hep::Position ipos(hit.getPosition().x, hit.getPosition().y, hit.getPosition().z);
		if(ipos==pos) return hit;
	}
   edm4hep::MutableSimCalorimeterHit hit;
   hit.setCellID(0);
   return hit;
}
*/
edm4hep::MutableSimCalorimeterHit EcalDigiAlg::find(const std::vector<edm4hep::MutableSimCalorimeterHit>& m_col, unsigned long long& cellid) const{
  for(int i=0;i<m_col.size();i++){
		edm4hep::MutableSimCalorimeterHit hit=m_col.at(i);
		if(hit.getCellID() == cellid) return hit;
	}
	edm4hep::MutableSimCalorimeterHit hit ;
	hit.setCellID(0);
	return hit;
}

void EcalDigiAlg::Clear(){
  totE_Truth = -99;
	totE_Digi = -99;
	mean_CT = -99;
	ECALTemp = -99;
	m_step_x.clear();
	m_step_y.clear();
	m_step_z.clear();
	m_step_t.clear();   // yyy: clear
	m_step_E.clear();
	m_stepBar_x.clear();
	m_stepBar_y.clear();
	m_stepBar_z.clear();
	m_step_T1.clear();
	m_step_T2.clear();
	m_simBar_x.clear();
	m_simBar_y.clear();
	m_simBar_z.clear();
	m_simBar_T1.clear();
	m_simBar_T2.clear();
	m_simBar_E_Truth.clear();
	m_simBar_Scint.clear();
	m_simBar_ChannelTemp.clear();
	m_simBar_CryTID.clear();
	m_simBar_SiPMNIEL.clear();
	m_simBar_CryIntLY.clear();
	m_simBar_SiPMGain.clear();
	m_simBar_SiPMDCR.clear();
	m_simBar_LO1.clear();
	m_simBar_LO2.clear();
	m_simBar_NDC1.clear();
	m_simBar_NDC2.clear();
	m_simBar_NDetPE1.clear();
	m_simBar_NDetPE2.clear();
	m_simBar_Pedestal1.clear();
	m_simBar_Pedestal2.clear();
	m_simBar_ADC1.clear();
	m_simBar_ADC2.clear();
	m_simBar_ADCGain1.clear();
	m_simBar_ADCGain2.clear();
	m_simBar_Q1_Att.clear();
	m_simBar_Q2_Att.clear();
	m_simBar_Q1_Digi.clear();
	m_simBar_Q2_Digi.clear();
	m_simBar_length.clear();
	m_simBar_system.clear();
	m_simBar_module.clear();
	m_simBar_stave.clear();
	m_simBar_part.clear();
	m_simBar_dlayer.clear();
	m_simBar_slayer.clear();
	m_simBar_bar.clear();
  m_simBar_cellID.clear();
}
