// /* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
// // Unit in code: mm, ns. 

#include "HcalDigiAlg.h" 

#include "edm4hep/SimCalorimeterHit.h"
#include "edm4hep/CalorimeterHit.h"
#include "edm4hep/Vector3f.h"
#include "edm4hep/Cluster.h"

#include "DD4hep/Detector.h"
#include <DD4hep/Objects.h>
#include <DDRec/CellIDPositionConverter.h>

#include "TVector3.h"
#include <math.h>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <map>

// #include <fstream>
// #include <ctime>

#define C 299.79  // unit: mm/ns
#define PI 3.141592653
using namespace std;
using namespace dd4hep;

DECLARE_COMPONENT( HcalDigiAlg )

HcalDigiAlg::HcalDigiAlg(const std::string& name, ISvcLocator* svcLoc)
  : GaudiAlgorithm(name, svcLoc),
    _nEvt(0)
{
  
	// Input collections
	declareProperty("SimCaloHitCollection", r_SimCaloCol, "Handle of the Input SimCaloHit collection");
  
	// Output collections
	declareProperty("CaloHitCollection", w_DigiCaloCol, "Handle of Digi CaloHit collection");
	declareProperty("CaloAssociationCollection", w_CaloAssociationCol, "Handle of CaloAssociation collection");
  declareProperty("CaloMCPAssociationCollection", w_MCPCaloAssociationCol, "Handle of CaloAssociation collection"); 
}

StatusCode HcalDigiAlg::initialize()
{
  if(_writeNtuple){
    std::string s_outfile = _filename;
    m_wfile = new TFile(s_outfile.c_str(), "recreate");
    t_simHit = new TTree("simHit", "simHit");

    
    t_simHit->Branch("totE", &m_totE);
    t_simHit->Branch("simHit_x", &m_simHit_x);
    t_simHit->Branch("simHit_y", &m_simHit_y);
    t_simHit->Branch("simHit_z", &m_simHit_z);
    t_simHit->Branch("simHit_E", &m_simHit_E);
    t_simHit->Branch("simHit_Etruth", &m_simHit_Etruth);
    t_simHit->Branch("simHit_Eatt", &m_simHit_Eatt);
    t_simHit->Branch("simHit_Npe_scint", &m_simHit_Npe_scint);
    t_simHit->Branch("simHit_Npe_sipm", &m_simHit_Npe_sipm);
    t_simHit->Branch("simHit_rawQ", &m_simHit_rawQ);
    t_simHit->Branch("simHit_HG", &m_simHit_HG);
    t_simHit->Branch("simHit_LG", &m_simHit_LG);
    t_simHit->Branch("simHit_steps", &m_simHit_steps);
    
    t_simHit->Branch("simHit_module", &m_simHit_module);
    t_simHit->Branch("simHit_stave", &m_simHit_stave);
    t_simHit->Branch("simHit_layer", &m_simHit_layer);
    t_simHit->Branch("simHit_tower", &m_simHit_tower);
    t_simHit->Branch("simHit_slice", &m_simHit_slice);
    t_simHit->Branch("simHit_cellID", &m_simHit_cellID);
  }
	std::cout<<"HcalDigiAlg::m_scale="<<m_scale<<std::endl;
	m_geosvc = service<IGeomSvc>("GeomSvc");
	if ( !m_geosvc )  throw "HcalDigiAlg :Failed to find GeomSvc ...";
	dd4hep::Detector* m_dd4hep = m_geosvc->lcdd();
	if ( !m_dd4hep )  throw "HcalDigiAlg :Failed to get dd4hep::Detector ...";
	m_cellIDConverter = new dd4hep::rec::CellIDPositionConverter(*m_dd4hep);
   	m_decoder = m_geosvc->getDecoder(_readoutName);
	if (!m_decoder) {
		error() << "Failed to get the decoder. " << endmsg;
		return StatusCode::FAILURE;
	}


	rndm.SetSeed(_seed);
	std::cout<<"HcalDigiAlg::initialize"<<std::endl;
	return GaudiAlgorithm::initialize();
}

StatusCode HcalDigiAlg::execute()
{
// clock_t yyy_start, yyy_enddigi;
// yyy_start = clock(); // 记录开始时间

	if(_nEvt==0) std::cout<<"HcalDigiAlg::execute Start"<<std::endl;
	std::cout<<"Processing event: "<<_nEvt<<std::endl;
   	if(_nEvt<_Nskip){ _nEvt++; return StatusCode::SUCCESS; }

	Clear();
  m_totE = 0.;
 	const edm4hep::SimCalorimeterHitCollection* SimHitCol =  r_SimCaloCol.get();
  std::vector<edm4hep::SimCalorimeterHit> m_simhitCol; m_simhitCol.clear();

	edm4hep::CalorimeterHitCollection* caloVec = w_DigiCaloCol.createAndPut();
	edm4hep::MCRecoCaloAssociationCollection* caloAssoVec = w_CaloAssociationCol.createAndPut();
  edm4hep::MCRecoCaloParticleAssociationCollection* caloMCPAssoVec = w_MCPCaloAssociationCol.createAndPut(); 

	if(SimHitCol == 0) 
	{
		std::cout<<"not found SimCalorimeterHitCollection"<< std::endl;
		return StatusCode::SUCCESS;
	}
  if(_Debug>=1) std::cout<<"digi, input sim hit size="<< SimHitCol->size() <<std::endl;

  MergeHits(*SimHitCol, m_simhitCol);

  for(int isim=0; isim<m_simhitCol.size(); isim++){

    auto simhit = m_simhitCol.at(isim);
    if(!simhit.isAvailable()) continue;
    if(simhit.getEnergy()==0) continue;

    unsigned long long id = simhit.getCellID();
    edm4hep::Vector3f hitpos = simhit.getPosition();
    TVector3 tilepos(hitpos.x, hitpos.y, hitpos.z); //cm to mm.    

    //Loop G4 steps to get the attenuated light yield.
    double Ehit_truth = 0.;
    double Ehit = 0.;
    for(int iCont=0; iCont < simhit.contributions_size(); ++iCont){
      auto conb = simhit.getContributions(iCont);
      if( !conb.isAvailable() ) { std::cout<<" Can not get SimHitContribution: "<<iCont<<std::endl; continue;}
      TVector3 steppos(conb.getStepPosition().x, conb.getStepPosition().y, conb.getStepPosition().z);

      double _distance = (tilepos-steppos).Mag(); //Simplified: use R(step-center) not R(step-SiPM) as distance. 
      Ehit_truth += conb.getEnergy();
      Ehit += conb.getEnergy()*exp(-1.*_distance/_EffAttenLength);
    }
    double Ehit_att = Ehit;

    double sChargeOut = 0;
    double sChargeOutHG = 0;
    double sChargeOutLG = 0;
    double Npe_scint = 0;
    double Npe_SiPM = 0;
    //Digitization
    if(_UseRelDigi){
      // -- Scintillation (Energy -> MIP -> Np.e.)
      int sPix = gRandom->Poisson(Ehit / _MIPCali * (_MIPADC / _PeADCMean));
      Npe_scint = sPix;
      // -- Tile non-uniformity 
      sPix = sPix * (1.0 + gRandom->Uniform(-_TileRes, _TileRes));
      // -- SiPM Saturation (Np.e. -> Npixel)
      sPix = std::round(_Pixel * (1.0 - TMath::Exp(-sPix * 1.0 / _Pixel)));
      Npe_SiPM = sPix;
      // -- ADC response (Npixel -> ADC)
      double sChargeMean = sPix * _PeADCMean;
      double sChargeSigma = sqrt(sPix * _PeADCSigma * _PeADCSigma);
      sChargeOut = gRandom->Gaus(sChargeMean, sChargeSigma);
      // -- (ADC->MIP)
      sChargeOutHG = sChargeOut + gRandom->Gaus(_BaselineHG, _BaselineSigmaHG);
      sChargeOutLG = sChargeOut / _HLRatio + gRandom->Gaus(_BaselineLG, _BaselineSigmaLG);
      sChargeOutHG = std::round(gRandom->Gaus(sChargeOutHG, sChargeOutHG * _ADCError));
      sChargeOutLG = std::round(gRandom->Gaus(sChargeOutLG, sChargeOutLG * _ADCError));
      if (sChargeOutLG > _ADCLimit)
          sChargeOutLG = _ADCLimit;
      Double_t sMIP = 0;
      if (sChargeOutHG > _ADCSwitch)
      {
          sMIP = (sChargeOutLG - _BaselineLG) * _HLRatio / _MIPADC;
          sChargeOutHG = _ADCSwitch;
      }
      else
      {
          sMIP = (sChargeOutHG - _BaselineHG) / _MIPADC;
      }
      Ehit = sMIP * _MIPCali;
    }
    if(Ehit<_MIPCali*_Eth_Mip) continue;

    //Global calibration. 
    //TODO: add more digitization terms here. 
    double Ehit_cali = Ehit*r_cali;

    //Loop contributions to get hit time and MCParticle. 
    double Thit_ave = 0.;
    double Ehit_raw = 0.;
    MCParticleToEnergyWeightMap MCPEnMap; MCPEnMap.clear();
    for(int iConb=0; iConb<simhit.contributions_size(); ++iConb){
      auto conb = simhit.getContributions(iConb);
      if(!conb.isAvailable()) continue;
      if(conb.getEnergy()==0) continue;

      Thit_ave += conb.getTime();
      
      auto mcp = conb.getParticle();
      MCPEnMap[mcp] += conb.getEnergy();
      Ehit_raw += conb.getEnergy();
    }
    Thit_ave = Thit_ave/simhit.contributions_size();
    //Create DigiHit
    auto digiHit = caloVec->create();
    digiHit.setCellID(id);
    digiHit.setEnergy(Ehit_cali);
    digiHit.setTime(Thit_ave);
    digiHit.setPosition(simhit.getPosition());

    //Create SimHit-DigiHit association. 
    auto rel = caloAssoVec->create();
    rel.setRec(digiHit);
    rel.setSim(simhit);
    rel.setWeight(1.);

    //Create DigiHit-MCParticle association.
    for(auto iter : MCPEnMap){
      auto rel_MC = caloMCPAssoVec->create();
      rel_MC.setRec(digiHit);
      rel_MC.setSim(iter.first);
      rel_MC.setWeight(iter.second/Ehit_raw);
    }

    if(_writeNtuple){
      m_totE += digiHit.getEnergy();
      m_simHit_x.push_back(digiHit.getPosition().x);
      m_simHit_y.push_back(digiHit.getPosition().y);
      m_simHit_z.push_back(digiHit.getPosition().z);
      m_simHit_E.push_back(digiHit.getEnergy());
      m_simHit_Etruth.push_back(Ehit_truth);
      m_simHit_Eatt.push_back(Ehit_att);
      m_simHit_rawQ.push_back(sChargeOut);
      m_simHit_HG.push_back(sChargeOutHG);
      m_simHit_LG.push_back(sChargeOutLG);
      m_simHit_Npe_scint.push_back(Npe_scint);
      m_simHit_Npe_sipm.push_back(Npe_SiPM);
      m_simHit_steps.push_back(simhit.contributions_size());
      //m_simHit_module.push_back(m_decoder->get(id, "stave"));
      //m_simHit_stave.push_back(m_decoder->get(id, "layer"));
      //m_simHit_layer.push_back(m_decoder->get(id, "tile"));
      //m_simHit_slice.push_back(m_decoder->get(id, "x"));
      //m_simHit_tower.push_back(m_decoder->get(id, "y"));
      m_simHit_cellID.push_back(id);
    }
  }

	if(_writeNtuple) t_simHit->Fill();

	_nEvt ++ ;
	return StatusCode::SUCCESS;
}

StatusCode HcalDigiAlg::finalize()
{
  if(_writeNtuple){
	  m_wfile->cd();
	  t_simHit->Write();
    m_wfile->Close();
	  delete m_wfile, t_simHit; 
  }

	info() << "Processed " << _nEvt << " events " << endmsg;
	delete m_cellIDConverter, m_decoder, m_geosvc;
	return GaudiAlgorithm::finalize();
}


StatusCode HcalDigiAlg::MergeHits( const edm4hep::SimCalorimeterHitCollection& m_col, std::vector<edm4hep::SimCalorimeterHit>& m_hits ){

  m_hits.clear();
  std::vector<edm4hep::MutableSimCalorimeterHit> m_mergedhit;
  m_mergedhit.clear();

  for(int iter=0; iter<m_col.size(); iter++){
    edm4hep::SimCalorimeterHit m_step = m_col[iter];
    if(!m_step.isAvailable()){ cout<<"ERROR HIT!"<<endl; continue;}
    if(m_step.getEnergy()==0 || m_step.contributions_size()<1) continue;
    unsigned long long cellid = m_step.getCellID();
    //edm4hep::Vector3f pos = m_step.getPosition();;
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


edm4hep::MutableSimCalorimeterHit HcalDigiAlg::find(const std::vector<edm4hep::MutableSimCalorimeterHit>& m_col, unsigned long long& cellid) const{
  for(int i=0;i<m_col.size();i++){
    edm4hep::MutableSimCalorimeterHit hit=m_col.at(i);
    if(hit.getCellID() == cellid) return hit;
  }
  edm4hep::MutableSimCalorimeterHit hit ;
  hit.setCellID(0);
  return hit;
}

void HcalDigiAlg::Clear(){
  m_totE = -99;
  m_simHit_x.clear();
  m_simHit_y.clear();
  m_simHit_z.clear();
  m_simHit_E.clear();
  m_simHit_Eatt.clear();
  m_simHit_Etruth.clear();
  m_simHit_rawQ.clear();
  m_simHit_HG.clear();
  m_simHit_LG.clear();
  m_simHit_Npe_scint.clear();
  m_simHit_Npe_sipm.clear();
  m_simHit_steps.clear();
  m_simHit_module.clear();
  m_simHit_stave.clear();
  m_simHit_layer.clear();
  m_simHit_slice.clear();
  m_simHit_tower.clear();
  m_simHit_cellID.clear();
}

