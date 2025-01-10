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
  declareProperty("MCParticle",  m_MCParticleCol, "MCParticle collection (input)");
	//declareProperty("SimCaloHitCollection", r_SimCaloCol, "Handle of the Input SimCaloHit collection");
  
	// Output collections
	//declareProperty("CaloHitCollection", w_DigiCaloCol, "Handle of Digi CaloHit collection");
	//declareProperty("CaloAssociationCollection", w_CaloAssociationCol, "Handle of CaloAssociation collection");
  //declareProperty("CaloMCPAssociationCollection", w_MCPCaloAssociationCol, "Handle of CaloAssociation collection"); 
}

StatusCode HcalDigiAlg::initialize()
{
  // --- Initialize input and output collections
  for(auto& simhit : name_SimCaloHit){
    if(!simhit.empty())
     _inputSimHitCollection.push_back( new SimCaloType(simhit, Gaudi::DataHandle::Reader, this) );
  }

  // --- Geometry service, cellID decoder
  m_geosvc = service<IGeomSvc>("GeomSvc");
  if ( !m_geosvc )  throw "HcalDigiAlg :Failed to find GeomSvc ...";
  m_dd4hep = m_geosvc->lcdd();
  if ( !m_dd4hep )  throw "HcalDigiAlg :Failed to get dd4hep::Detector ...";
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
    t_simHit = new TTree("simHit", "simHit");

    
    t_simHit->Branch("MC_EPx", &m_MC_EPx);
    t_simHit->Branch("MC_EPy", &m_MC_EPy);
    t_simHit->Branch("MC_EPz", &m_MC_EPz);
    t_simHit->Branch("totE", &m_totE);
    t_simHit->Branch("totE_truth", &m_totE_truth);
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
    t_simHit->Branch("step_x", &m_step_x);
    t_simHit->Branch("step_y", &m_step_y);
    t_simHit->Branch("step_LY", &m_step_LY);
    
    t_simHit->Branch("simHit_system", &m_simHit_system);
    t_simHit->Branch("simHit_stave", &m_simHit_stave);
    t_simHit->Branch("simHit_layer", &m_simHit_layer);
    t_simHit->Branch("simHit_tile", &m_simHit_tile);
    t_simHit->Branch("simHit_idx", &m_simHit_idx);
    t_simHit->Branch("simHit_idy", &m_simHit_idy);
    t_simHit->Branch("simHit_row", &m_simHit_row);
    t_simHit->Branch("simHit_phi", &m_simHit_phi);
    t_simHit->Branch("simHit_cellID", &m_simHit_cellID);
  }

  //Glass tile non-uniformity map (due to attenuation length)
  TFile* map_file = new TFile(_TileLYMapFile.value().c_str(), "read");
  TString s_hitmap = TString("hitmap_") + TString(_EffAttenLength.value().c_str());
  GSTileResMap = (TH2D*)map_file->Get(s_hitmap);
  if(!GSTileResMap || !_UseTileLYMap ){
    error() << "HcalDigiAlg: Failed to get the GS tile response map. Create a uniform one" << endmsg;
    GSTileResMap = new TH2D("hitmap", "", 80, -20, 20, 80, -20, 20);
    for(int i=0; i<80; i++){
      for(int j=0; j<80; j++){
        GSTileResMap->SetBinContent(i+1, j+1, _MIPLY );
    }}
  }
  else{
    //Scale map to light yield
    TString s_h_Npe = TString("h_Npe_") + TString(_EffAttenLength.value().c_str());
    TH1D *h_Npe = (TH1D*)map_file->Get(s_h_Npe);
    h_Npe->Fit("landau","Q");
    GSTileResMap->Scale( _MIPLY / h_Npe->GetFunction("landau")->GetParameter(1) );
  }

  //SiPM dark noise cross talk model: Borel distribution.
  f_DarkNoise = new TF1("f_DarkNoise", "pow([0]*x, x-1) * exp(-[0]*x) / TMath::Factorial(x)");
  f_DarkNoise->SetParameter(0, _SiPMXTalk);

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
  m_totE_truth = 0.;
  for(int icol=0; icol<_inputSimHitCollection.size(); icol++){
    try{
      SimCaloType* r_SimCaloCol = _inputSimHitCollection[icol];
      CaloType* w_DigiCaloCol = _outputHitCollection[icol];
      CaloSimAssoType* w_CaloAssociationCol = _outputCaloSimAssoCol[icol];
      CaloParticleAssoType* w_MCPCaloAssociationCol = _outputCaloMCPAssoCol[icol];

      const edm4hep::MCParticleCollection* const_MCPCol = m_MCParticleCol.get();
      const edm4hep::SimCalorimeterHitCollection* SimHitCol =  r_SimCaloCol->get();
      std::vector<edm4hep::SimCalorimeterHit> m_simhitCol; m_simhitCol.clear();
      
      edm4hep::CalorimeterHitCollection* caloVec = w_DigiCaloCol->createAndPut();
      edm4hep::MCRecoCaloAssociationCollection* caloAssoVec = w_CaloAssociationCol->createAndPut();
      edm4hep::MCRecoCaloParticleAssociationCollection* caloMCPAssoVec = w_MCPCaloAssociationCol->createAndPut(); 
      
      if(const_MCPCol){
        int N_MCP = const_MCPCol->size();
        if(N_MCP==1){
          m_MC_EPx = const_MCPCol->at(0).getEndpoint().x;
          m_MC_EPy = const_MCPCol->at(0).getEndpoint().y;
          m_MC_EPz = const_MCPCol->at(0).getEndpoint().z;
        }
      }
      
      
      if(SimHitCol == 0) 
      {
      	std::cout<<"not found SimCalorimeterHitCollection"<< std::endl;
      	return StatusCode::SUCCESS;
      }
      
      //Mean cross talk probability in this event
      double mean_CT = 0;
      for (int i = 1; i < 10; i++)
      {
        mean_CT += (i-1)*f_DarkNoise->Eval(i);
      }
      
      MergeHits(*SimHitCol, m_simhitCol);
//cout<<"Merged simhit size: "<<m_simhitCol.size()<<endl;      
      for(int isim=0; isim<m_simhitCol.size(); isim++){
      
        auto simhit = m_simhitCol.at(isim);
        if(!simhit.isAvailable()) {cout<<"Sim hit is not available"<<endl; continue;}
        if(simhit.getEnergy()==0) {cout<<"Sim hit energy is 0"<<endl; continue;}
        m_totE_truth += simhit.getEnergy();
      
        unsigned long long id = simhit.getCellID();
        int hit_system =  map_readout_decoder[name_Readout[icol]]->get(id, "system");
        edm4hep::Vector3f hitpos = simhit.getPosition();
        TVector3 tilepos(hitpos.x, hitpos.y, hitpos.z); //cm to mm.    
        double rotPhi = 0;
        if(hit_system==22){
          int hit_stave = map_readout_decoder[name_Readout[icol]]->get(id, "stave");
          rotPhi = (hit_stave-5)*TMath::TwoPi()/16.;
          if(rotPhi<0) rotPhi += TMath::TwoPi();
        }
//cout<<"  Sim hit #"<<isim<<": system "<<hit_system<<", energy "<<simhit.getEnergy();      
        //printf("Hit #%d: stave %d, pos (%.2f, %.2f, %.2f), Energy %.2f, step size %d \n", isim, hit_stave, hitpos.x, hitpos.y, hitpos.z, simhit.getEnergy(), simhit.contributions_size());
        //Loop G4 steps to get the attenuated light yield.
        double Ehit = 0;
        double Ehit_truth = 0.;
        double Npe_att = 0;
        float tempVar = gRandom->Gaus(0, _TempVariation);
        float fLY_tempScale = (1 + tempVar * _TempCoef);
        for(int iCont=0; iCont < simhit.contributions_size(); ++iCont){
          auto conb = simhit.getContributions(iCont);
          if( !conb.isAvailable() ) { std::cout<<" Can not get SimHitContribution: "<<iCont<<std::endl; continue;}
          Ehit_truth += conb.getEnergy();
          TVector3 steppos(conb.getStepPosition().x, conb.getStepPosition().y, conb.getStepPosition().z);
      
          //double _distance = (tilepos-steppos).Mag(); //Simplified: use R(step-center) not R(step-SiPM) as distance. 
          //m_step_R.push_back(_distance);
          int ibinx = 0;
          int ibiny = 0;
          if(hit_system==22){
            TVector3 rot_tilepos = tilepos; rot_tilepos.RotateZ(rotPhi);
            TVector3 rot_steppos = steppos; rot_steppos.RotateZ(rotPhi);
            ibinx = ((rot_steppos-rot_tilepos).z()+20.)*2;
            ibiny = ((rot_steppos-rot_tilepos).y()+20.)*2;
            if(ibinx<0) ibinx = 0;
            if(ibiny<0) ibiny = 0;
            if(ibinx>79) ibinx = 79;
            if(ibiny>79) ibiny = 79;
            m_step_x.push_back((rot_steppos-rot_tilepos).z());
            m_step_y.push_back((rot_steppos-rot_tilepos).y());
          }
          else if(hit_system==30){
            if(ibinx<0) ibinx = 0;
            if(ibiny<0) ibiny = 0;
            if(ibinx>79) ibinx = 79;
            if(ibiny>79) ibiny = 79;
            ibinx = ((steppos-tilepos).x()+20.)*2;
            ibiny = ((steppos-tilepos).y()+20.)*2;
            m_step_x.push_back((steppos-tilepos).x());
            m_step_y.push_back((steppos-tilepos).y());
          }
          else error() << "Wrong cellID system: " << hit_system << endmsg;
      //printf("  Step #%d: En %.2f, rotate angle %.2f, tile pos after rotation (%.2f, %.2f, %.2f), ", iCont, conb.getEnergy(), rotPhi, rot_tilepos.x(), rot_tilepos.y(), rot_tilepos.z());
      //printf("rel pos after rotation (%.2f, %.2f, %.2f) \n", (rot_steppos-rot_tilepos).x(), (rot_steppos-rot_tilepos).y(), (rot_steppos-rot_tilepos).z());
      //printf("  Project to bin (%d, %d), LY %.3f \n", ibinx, ibiny, GSTileResMap->GetBinContent( ibinx, ibiny ));
      
          Npe_att += conb.getEnergy() / _MIPCali * GSTileResMap->GetBinContent( ibinx, ibiny ) * fLY_tempScale;     
          m_step_LY.push_back(GSTileResMap->GetBinContent( ibinx, ibiny ));
        }
        Ehit = Npe_att / _MIPLY / fLY_tempScale * _MIPCali;

        double sChargeOut = 0;
        double sChargeOutHG = 0;
        double sChargeOutLG = 0;
        double Npe_scint = 0;
        double Npe_SiPM = 0;
        //Digitization
        if(_UseRelDigi){
          // -- Scintillation (Energy -> MIP -> Np.e.)
          float fSiPMGainMean = _SiPMGainMean * (1 + tempVar * _SiPMGainTempCoef);
          float fSiPMDCR = _SiPMDCR * pow(10, _SiPMDCRTempCoef * tempVar);

          // -- Scintillation (Energy -> MIP -> Np.e.)
          int sPix = gRandom->Poisson(Npe_att);
          Npe_scint = sPix;      
          // -- SiPM dark noise and cross talk
          int darkcounts_mean = gRandom->Poisson(_SiPMDCR * _TimeInterval);
          int darkcounts_CT = 0;
          for(int i=0;i<darkcounts_mean;i++)
          {
            double darkcounts_rdm = gRandom->Uniform(0, 1);
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
          sPix += darkcounts_CT;      
      
          // -- SiPM Saturation (Np.e. -> Npixel)
          sPix = std::round(_Pixel * (1.0 - TMath::Exp(-sPix * 1.0 / _Pixel)));
          Npe_SiPM = sPix;
          // -- ADC response (Npixel -> ADC)
          float sSiPMGainMean = fSiPMGainMean;
          float sSiPMGainSigma = sSiPMGainMean * _SiPMGainSigma;
          float sSiPMNoiseSigma = _SiPMNoiseSigma;

          float sADCMean = sPix * sSiPMGainMean + _Pedestal;
          float sADCSigma = std::sqrt(sPix * sSiPMGainSigma * sSiPMGainSigma + sSiPMNoiseSigma * sSiPMNoiseSigma + _PedestalNoiseSigma * _PedestalNoiseSigma);
          float sADC = -1;
          sADC = std::round(gRandom->Gaus(sADCMean, sADCSigma));
          if(sADC < 0) sADC = 0;
          sChargeOut = sADC;
          if(sADC <= _ADCSwitch) //Gain 1
          {

            float sMIP = (sADC - _Pedestal) / _SiPMGainMean / _MIPLY / fLY_tempScale;
            if(sMIP < _Eth_Mip) Ehit = 0;
            Ehit = sMIP * _MIPCali;
          }
          else if(sADC > _ADCSwitch && int(sADC/_GainRatio_12) <= _ADCSwitch)
          {
            //Use_G2 = kTRUE;
            sSiPMGainMean = fSiPMGainMean / _GainRatio_12;
            sSiPMGainSigma = sSiPMGainMean * _SiPMGainSigma;
            sSiPMNoiseSigma = _SiPMNoiseSigma / _GainRatio_12;

            sADCMean = sPix * sSiPMGainMean + _Pedestal;
            sADCSigma = std::sqrt(sPix * sSiPMGainSigma * sSiPMGainSigma + sSiPMNoiseSigma * sSiPMNoiseSigma + _PedestalNoiseSigma * _PedestalNoiseSigma);
            sADC = std::round(gRandom->Gaus(sADCMean, sADCSigma));
            if(sADC < 0) sADC = 0;
            sChargeOutHG = sADC;

            // float sCaliGainMean = gRandom->Gaus(_SiPMGainMean, fSiPMGainUn * _SiPMGainMean);
            // float sMIP = (sADC - _Pedestal) / sCaliGainMean / _MIPLY;
            float sMIP = (sADC - _Pedestal) / _SiPMGainMean / _MIPLY / fLY_tempScale * _GainRatio_12;
            if(sMIP < _Eth_Mip) Ehit = 0;
            else Ehit = sMIP * _MIPCali;
          }
          else if(int(sADC/_GainRatio_12) > _ADCSwitch)
          {
            sSiPMGainMean = fSiPMGainMean / _GainRatio_12 / _GainRatio_23;
            sSiPMGainSigma = sSiPMGainMean * _SiPMGainSigma;
            sSiPMNoiseSigma = _SiPMNoiseSigma / _GainRatio_12 / _GainRatio_23;

            sADCMean = sPix * sSiPMGainMean + _Pedestal;
            sADCSigma = std::sqrt(sPix * sSiPMGainSigma * sSiPMGainSigma + sSiPMNoiseSigma * sSiPMNoiseSigma + _PedestalNoiseSigma * _PedestalNoiseSigma);
            sADC = std::round(gRandom->Gaus(sADCMean, sADCSigma));
            if(sADC < 0) sADC = 0;
            sChargeOutLG = sADC;

            // float sCaliGainMean = gRandom->Gaus(_SiPMGainMean, fSiPMGainUn * _SiPMGainMean);
            // float sMIP = (sADC - _Pedestal) / sCaliGainMean / _MIPLY;
            float sMIP = (sADC - _Pedestal) / _SiPMGainMean / _MIPLY / fLY_tempScale * _GainRatio_12 * _GainRatio_23;
            if(sMIP < _Eth_Mip) Ehit = 0;
            else Ehit = sMIP * _MIPCali;
          }
      
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
          m_simHit_Eatt.push_back(Npe_att);
          m_simHit_rawQ.push_back(sChargeOut);
          m_simHit_HG.push_back(sChargeOutHG);
          m_simHit_LG.push_back(sChargeOutLG);
          m_simHit_Npe_scint.push_back(Npe_scint);
          m_simHit_Npe_sipm.push_back(Npe_SiPM);
          m_simHit_steps.push_back(simhit.contributions_size());
          m_simHit_system.push_back(map_readout_decoder[name_Readout[icol]]->get(id, "system"));
          m_simHit_stave.push_back(map_readout_decoder[name_Readout[icol]]->get(id, "stave"));
          m_simHit_layer.push_back(map_readout_decoder[name_Readout[icol]]->get(id, "layer"));
          if(hit_system==22){
            m_simHit_tile.push_back(map_readout_decoder[name_Readout[icol]]->get(id, "tile"));
            m_simHit_idx.push_back(map_readout_decoder[name_Readout[icol]]->get(id, "x"));
            m_simHit_idy.push_back(map_readout_decoder[name_Readout[icol]]->get(id, "y"));
            m_simHit_row.push_back(-1);
            m_simHit_phi.push_back(-1);
          }
          if(hit_system==30){
            m_simHit_tile.push_back(-1);
            m_simHit_idx.push_back(-1);
            m_simHit_idy.push_back(-1);
            m_simHit_row.push_back(map_readout_decoder[name_Readout[icol]]->get(id, "row"));
            m_simHit_phi.push_back(map_readout_decoder[name_Readout[icol]]->get(id, "phi"));
          }
          m_simHit_cellID.push_back(id);
        }
      }
      m_simhitCol.clear();
    }catch(GaudiException &e){
      info()<<"SimCaloHit collection "<<name_SimCaloHit[icol]<<" is not available "<<endmsg;
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
  map_readout_decoder.clear();
	delete m_cellIDConverter, m_geosvc;
  delete f_DarkNoise, GSTileResMap;
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
  m_MC_EPx = 0;
  m_MC_EPy = 0;
  m_MC_EPz = 0;
  m_totE = -99;
  m_totE_truth = -99;
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
  m_simHit_system.clear();
  m_simHit_stave.clear();
  m_simHit_layer.clear();
  m_simHit_tile.clear();
  m_simHit_idx.clear();
  m_simHit_idy.clear();
  m_simHit_row.clear();
  m_simHit_phi.clear();
  m_simHit_cellID.clear();
  m_step_x.clear();
  m_step_y.clear();
  m_step_LY.clear();
}

