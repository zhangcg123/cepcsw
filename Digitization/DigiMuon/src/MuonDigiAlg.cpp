#include "MuonDigiAlg.h"

#include "edm4hep/Vector3f.h"

#include "DD4hep/Detector.h"
#include <DD4hep/Objects.h>
#include "DD4hep/DD4hepUnits.h"
#include "DDRec/Vector3D.h"

#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IRndmGen.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"

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
#include <random>
#include <iostream>
#include <vector>


using namespace std ;

DECLARE_COMPONENT( MuonDigiAlg )

MuonDigiAlg::MuonDigiAlg(const std::string& name, ISvcLocator* svcLoc)
: GaudiAlgorithm(name, svcLoc)
{
  // Input collections
  declareProperty("MuonBarrelHitsCollection", m_inputMuonBarrel, "Handle of the Input SimTrackerHit collection");
  declareProperty("MuonEndcapHitsCollection", m_inputMuonEndcap, "Handle of the Input SimTrackerHit collection");


  // Output collections
  declareProperty("MuonBarrelTrackerHits", m_outputMuonBarrel, "Handle of the output TrackerHit collection");
  declareProperty("MuonEndcapTrackerHits", m_outputMuonEndcap, "Handle of the output TrackerHit collection");
  //declareProperty("MuonBarrelTrackerHitAssociationCollection", m_assMuonBarrel, "Handle of the Association collection between SimTrackerHit and TrackerHit");
}


StatusCode MuonDigiAlg::initialize()
{
  // set rand seed;
  m_randSvc = service<IRndmGenSvc>("RndmGenSvc");

  // 
  if(_writeNtuple)
  {
    std::string s_outfile = _filename;
    m_wfile = new TFile(s_outfile.c_str(), "recreate");
    m_tree = new TTree("tree", "muon digi tree");
    m_tree->Branch("n_hit", &n_hit, "n_hit/I");
    m_tree->Branch("hit_cellid", hit_cellid, "hit_cellid[n_hit]/l");
    m_tree->Branch("hit_posx", hit_posx, "hit_posx[n_hit]/D");
    m_tree->Branch("hit_posy", hit_posy, "hit_posy[n_hit]/D");
    m_tree->Branch("hit_posz", hit_posz, "hit_posz[n_hit]/D");
    m_tree->Branch("hit_edep", hit_edep, "hit_edep[n_hit]/D");
    m_tree->Branch("hit_layer", hit_layer, "hit_layer[n_hit]/I");
    m_tree->Branch("hit_slayer", hit_slayer, "hit_slayer[n_hit]/I");
    m_tree->Branch("hit_strip", hit_strip, "hit_strip[n_hit]/I");
    m_tree->Branch("hit_fe", hit_fe, "hit_fe[n_hit]/I");
    m_tree->Branch("hit_env", hit_env, "hit_env[n_hit]/I");
    m_tree->Branch("n_cell", &n_cell, "n_cell/I");
    m_tree->Branch("cell_cellid", cell_cellid, "cell_cellid[n_cell]/l");
    m_tree->Branch("cell_edep", cell_edep, "cell_edep[n_cell]/D");
    m_tree->Branch("cell_adc", cell_adc, "cell_adc[n_cell]/D");
    m_tree->Branch("cell_posx", cell_posx, "cell_posx[n_cell]/D");
    m_tree->Branch("cell_posy", cell_posy, "cell_posy[n_cell]/D");
    m_tree->Branch("cell_posz", cell_posz, "cell_posz[n_cell]/D");
    m_tree->Branch("cell_layer", cell_layer, "cell_layer[n_cell]/I");
    m_tree->Branch("cell_slayer", cell_slayer, "cell_slayer[n_cell]/I");
    m_tree->Branch("cell_strip", cell_strip, "cell_strip[n_cell]/I");
    m_tree->Branch("cell_fe", cell_fe, "cell_fe[n_cell]/I");
    m_tree->Branch("cell_env", cell_env, "cell_env[n_cell]/I");
    m_tree->Branch("cell_superlayernumber", &cell_superlayernumber, "cell_superlayernumber/I");
    m_tree->Branch("cell_pt", &cell_pt, "cell_pt/I");
  }


  m_geosvc = service<IGeomSvc>("GeomSvc");
  if(!m_geosvc)
  {
    error() << "Failed to get the GeomSvc" << endmsg;
    return StatusCode::FAILURE;
  }
  auto m_dd4hep = m_geosvc->lcdd();
  if(!m_dd4hep)
  {
    error() << "Failed to get the Detector from GeomSvc" << endmsg;
    return StatusCode::FAILURE;
  }
  m_cellIDConverter = new dd4hep::rec::CellIDPositionConverter(*m_dd4hep);
  if(!m_cellIDConverter)
  {
    error() << "Failed to get the m_cellIDConverter." << endmsg;
    return StatusCode::FAILURE;
  }

  std::string readout_name_barrel = m_inputMuonBarrel.objKey(); 
  debug() << "Readout name: " << readout_name_barrel << endmsg;
  m_decoder_barrel = m_geosvc->getDecoder(readout_name_barrel);
  if(!m_decoder_barrel)
  {
    error() << "Failed to get the decoder. " << endmsg;
    return StatusCode::FAILURE;
  }

  std::string readout_name_endcap = m_inputMuonEndcap.objKey(); 
  debug() << "Readout name: " << readout_name_endcap << endmsg;
  m_decoder_endcap = m_geosvc->getDecoder(readout_name_endcap);
  if(!m_decoder_endcap)
  {
    error() << "Failed to get the decoder. " << endmsg;
    return StatusCode::FAILURE;
  }

  debug() << "m_hitEff: " << m_hitEff << endmsg;
 
  info() << "MuonDigiAlg::initialized" << endmsg;
  return GaudiAlgorithm::initialize();
}


StatusCode MuonDigiAlg::execute()
{
  const edm4hep::SimTrackerHitCollection* STHCol;
//MuonBarrelRun:
  Clear();
  trkhitVec = m_outputMuonBarrel.createAndPut();
  STHCol = nullptr;
  try 
  {
    STHCol = m_inputMuonBarrel.get();
  }
  catch(GaudiException &e)
  {
    debug() << "Collection " << m_inputMuonBarrel.fullKey() << " is unavailable in event " << m_nEvt << endmsg;
  }
  if(STHCol->size()==0) goto MuonEndcapRun;

  for(auto simhit : *STHCol)
  {
    GetSimHit(simhit, m_decoder_barrel);
    Fe = m_decoder_barrel->get(cellid, "Fe");
    if ( Edep > m_hit_Edep_max || Edep < m_hit_Edep_min ) continue;                 // Cut1: Edep of one hit
    if ( xydist < 5162 && std::abs(mcppos[2]) < 5475 || abspdgid != 13 ) continue;  // Cut2: The particle decays in the Muon Detector or is not a muon
    std::cout<<"Barrel Simulation Hit::::::: "<<Fe<<" "<<slayer<<" "<<layer<<"        "<<mcppos[0]<<" "<<mcppos[1]<<" "<<mcppos[2]<<" "<<abspdgid<<std::endl;
    hit_sipm_length = Gethit_sipm_length_Barrel();
    EdeptoADC();
    SaveData_mapcell();
  }

  if(map_cell_edep.size()==0) goto MuonEndcapRun;
  Cut3();

  for (const auto& item1 : map_cell_edep) 
  {
    key1 = item1.first;
    if(!Mapcell_todata(all_message1, key1)) continue;
    Save_pos(1);
    anotherlayer_cell_num = 0;   
    for (const auto& item2 : map_cell_edep) 
    {
      key2 = item2.first;
      if(!Mapcell_todata(all_message2, key2)) continue;
      if(all_message1[1]!=all_message2[1] || all_message1[2]!=all_message2[2] || all_message1[3]!=all_message2[3] || all_message1[0]==all_message2[0] || all_message1[0]==2) continue;
      Save_pos(2);
      std::cout<<"::::::  "<<cellid1<<" "<<key1[0]<<std::endl;
      std::cout<<ddpos1.x()<<" "<<ddpos1.y()<<std::endl;
      std::cout<<pos1[0]<<" "<<pos1[1]<<std::endl;
      Renew_strip(key1, key2);
      Find_anotherlayer(2, pos1, pos2, ddpos2.z(), all_message2[5]);
    }

    if(anotherlayer_cell_num == 0) continue;
    for(int i=1; i<=anotherlayer_cell_num; i++)
    {
      true_pdgid = 0;
      true_pdgid = Get_true_pdgid(i, all_message1[5]);
      pos = edm4hep::Vector3d(ddpos1.x()*10,ddpos1.y()*10,map_muonhit[i]*10);//mm
std::cout<<pos[0]<<" "<<pos[1]<<" "<<pos[2]<<std::endl;
      Save_trkhit(trkhitVec, key1, true_pdgid, cellid1, pos);
    }
  }
  Save_onelayer_signal(trkhitVec);

MuonEndcapRun:
  Clear();
  trkhitVec = m_outputMuonEndcap.createAndPut();
  STHCol = nullptr;
  try 
  {
    STHCol = m_inputMuonEndcap.get();
  }
  catch(GaudiException &e)
  {
    debug() << "Collection " << m_inputMuonEndcap.fullKey() << " is unavailable in event " << m_nEvt << endmsg;
  }
  if(STHCol->size()==0) return StatusCode::SUCCESS;

  for(auto simhit : *STHCol)
  {
    GetSimHit(simhit, m_decoder_endcap);
    Fe = m_decoder_endcap->get(cellid, "Endcap");
    if ( Edep > m_hit_Edep_max || Edep < m_hit_Edep_min ) continue;                 // Cut1: Edep of one hit
    if ( xydist < 5162 && std::abs(mcppos[2]) < 5475 || abspdgid != 13 ) continue;  // Cut2: The particle decays in the Muon Detector or is not a muon
    std::cout<<"Endcap Simulation Hit::::::: "<<Env<<" "<<slayer<<" "<<layer<<"          "<<mcppos[0]<<" "<<mcppos[1]<<" "<<mcppos[2]<<" "<<abspdgid<<std::endl;
    hit_sipm_length = Gethit_sipm_length_Endcap();
    EdeptoADC();
    SaveData_mapcell();
  }  

  if(map_cell_edep.size()==0) return StatusCode::SUCCESS;
  Cut3();

  for (const auto& item1 : map_cell_edep) 
  {
    key1 = item1.first;
    if(!Mapcell_todata(all_message1, key1)) continue;
    Save_pos(1);
    anotherlayer_cell_num = 0;  
    for (const auto& item2 : map_cell_edep) 
    {
      key2 = item2.first;
      if(!Mapcell_todata(all_message2, key2)) continue;
      if(all_message1[1]!=all_message2[1] || all_message1[3]!=all_message2[3] || all_message1[0]==all_message2[0] || all_message1[0]==2) continue;
      if(std::abs(all_message1[2] - all_message2[2]) == 2) continue;
      Save_pos(2);
      Renew_strip(key1, key2);
      Find_anotherlayer(0, pos1, pos2, ddpos2.x(), all_message2[5]);  
    }
    if(anotherlayer_cell_num == 0) continue;
    for(int i=1; i<=anotherlayer_cell_num; i++)
    {
      true_pdgid = 0;
      true_pdgid = Get_true_pdgid(i, all_message1[5]);
      pos = edm4hep::Vector3d(map_muonhit[i]*10,ddpos1.y()*10,ddpos1.z()*10);//mm
      Save_trkhit(trkhitVec, key1, true_pdgid, cellid1, pos);      
    }
  }
  Save_onelayer_signal(trkhitVec);

  m_nEvt++;
  return StatusCode::SUCCESS;
}

StatusCode MuonDigiAlg::finalize()
{

  if(_writeNtuple)
  {
    m_wfile->cd();
    m_tree->Write();
    m_wfile->Close();
    delete m_wfile, m_tree;
  } 
  info() << "Processed " << m_nEvt << " events " << endmsg;
  return GaudiAlgorithm::finalize();
}

void MuonDigiAlg::Clear()
{
  n_hit = 0;
  n_cell = 0;
  map_cell_edep.clear();
  map_cell_adc.clear();
  map_cell_layer.clear();
  map_cell_slayer.clear();
  map_cell_strip.clear();
  map_cell_fe.clear();
  map_cell_env.clear();
  map_cell_pos.clear();
  map_muonhit.clear();
  map_cell_pdgid.clear();
  map_pdgid.clear();
}

void MuonDigiAlg::GetSimHit(edm4hep::SimTrackerHit _simhit, dd4hep::DDSegmentation::BitFieldCoder* _m_decoder)
{
  auto mcp = _simhit.getMCParticle();
  cellid = _simhit.getCellID();
  mcppos = mcp.getEndpoint();
  pdgid = mcp.getPDG();    
  abspdgid = std::abs(mcp.getPDG());    
  key = std::array<unsigned long long, 2>{cellid, abspdgid};
  Edep   = _simhit.getEDep();//GeV
  layer  = _m_decoder->get(cellid, "Layer");
  slayer = _m_decoder->get(cellid, "Superlayer");
  strip  = _m_decoder->get(cellid, "Stripe");
  Env    = _m_decoder->get(cellid, "Env");
  pos = _simhit.getPosition();
  ddpos = m_cellIDConverter->position(cellid);  
  xydist = std::sqrt(mcppos[0]*mcppos[0]+mcppos[1]*mcppos[1]);
}

double MuonDigiAlg::Gethit_sipm_length_Barrel()
{
  //calculate hit strip length
  hit_strip_length = std::sqrt((ddpos.x()-pos[0]*0.1)*(ddpos.x()-pos[0]*0.1)+
                               (ddpos.y()-pos[1]*0.1)*(ddpos.y()-pos[1]*0.1)+
                               (ddpos.z()-pos[2]*0.1)*(ddpos.z()-pos[2]*0.1));
  if(layer == 1)
  {
    int tempnum = Env + slayer;
    if(tempnum%2 == 0)
    {
      // 115 is the number of strips perpendicular 
      // to the beam axis in long-half-barrel
      return (115.5 * 4.0)/2 - hit_strip_length;
    }
    else
    {
      // 106 is the number of strips perpendicular 
      // to the beam axis in short-half-barrel
      return (106.5 * 4.0)/2 - hit_strip_length;
    }
  } 
  if(layer == 2)
  {
    // number of strips parallel to beam direction 
    // in each slayer, each strip width=4cm, so 
    // the number * 4 cm gives the length of strips 
    // in each slayer parpendicular to the beam axis
    return (strip_length[slayer-1] * 4)/2 - hit_strip_length;
  }  
}

double MuonDigiAlg::Gethit_sipm_length_Endcap()
{
  //calculate hit strip length
  hit_strip_length = std::sqrt((ddpos.x()-pos[0]*0.1)*(ddpos.x()-pos[0]*0.1)+(ddpos.y()-pos[1]*0.1)*(ddpos.y()-pos[1]*0.1)+(ddpos.z()-pos[2]*0.1)*(ddpos.z()-pos[2]*0.1));
  strip--; // strip id begin at 1
  return endcap_strip_length[strip] * 0.5 * 100 - hit_strip_length;//cm  
}

void MuonDigiAlg::EdeptoADC()
{
  // digitize to ADC 
  ADCmean = 34*Edep*47.09/(23*0.00141);//mV
  if(hit_sipm_length>10)
  {
    ADCmean = (16.0813*std::exp(-1*hit_sipm_length/50.8147)+19.5474)*Edep*47.09/(23*0.00141);//mV
  }
  ADC = m_randSvc->generator(Rndm::Landau(ADCmean,7.922))->shoot();
  if(ADC<0)
  {
    ADC = 0;
  }
}

void MuonDigiAlg::SaveData_mapcell()
{
  // fill the cell loop
  map_cell_edep[key] += Edep;
  map_cell_adc[key] += ADC;
  map_cell_layer[key] = layer;
  map_cell_slayer[key] = slayer;
  map_cell_strip[key] = strip;
  map_cell_fe[key] = Fe;
  map_cell_env[key] = Env;
  map_cell_pos[key] = pos;
  map_cell_pdgid[key] = pdgid;
}

bool MuonDigiAlg::Mapcell_todata(int _message[6], std::array<unsigned long long, 2> _key)
{
  Edep = map_cell_edep[_key];
  if ( Edep < m_EdepMin ) return false;  
  _message[0] = map_cell_layer[_key];
  _message[1] = map_cell_slayer[_key];
  _message[2] = map_cell_env[_key];
  _message[3] = map_cell_fe[_key];
  _message[4] = map_cell_strip[_key];
  _message[5] = map_cell_pdgid[_key];
  return true;
}

void MuonDigiAlg::Cut3()
{
  //some criteria
  for(const auto& item : map_cell_edep)
  {
    key = item.first;
    if (map_cell_edep[key] < m_EdepMin || m_randSvc->generator(Rndm::Flat(0, 1))->shoot() > m_hitEff || map_cell_adc[key] == 0)
    {
      map_cell_edep[key] = 0;
    }
  }
}

void MuonDigiAlg::Save_trkhit(edm4hep::TrackerHitCollection* _trkhitVec, std::array<unsigned long long, 2> _key, int _pdgid, unsigned long long _cellid, edm4hep::Vector3d _pos)
{
  edm4hep::MutableTrackerHit trkHit;
  trkHit.setEDep(map_cell_adc[_key]);
  trkHit.setQuality(_pdgid);
  trkHit.setCellID(_cellid);
  trkHit.setPosition(_pos);
  _trkhitVec->push_back( trkHit );
}

void MuonDigiAlg::Renew_strip(std::array<unsigned long long, 2> _key1, std::array<unsigned long long, 2> _key2)
{
  map_cell_strip[_key1] = -1;
  map_cell_strip[_key2] = -1;
}

void MuonDigiAlg::Find_anotherlayer(int _i, edm4hep::Vector3d _pos1, edm4hep::Vector3d _pos2, double _ddposi, int _pdgid)
{
  Hit_min = _pos1[_i]*0.1-12;//10cm+2cm
  Hit_max = _pos1[_i]*0.1+12;
  if(_ddposi<Hit_min || _ddposi>Hit_max) return;
  anotherlayer_cell_num++;
  map_muonhit[anotherlayer_cell_num] = _ddposi; 
  map_pdgid[anotherlayer_cell_num] = -1;      
  if(std::abs(_pos1[0]-_pos2[0])>40 && std::abs(_pos1[1]-_pos2[1])>40 && std::abs(_pos1[2]-_pos2[2])>40) return;
  map_pdgid[anotherlayer_cell_num] = _pdgid;
}

int MuonDigiAlg::Get_true_pdgid(int _i, int _pdgid)
{
  if(std::abs(_pdgid) == 13 && std::abs(map_pdgid[_i]) == 13) { return 1; } //pdgid->true muon hit
  if(map_pdgid[_i]==-1) { return 2; } //ghost point
}

void MuonDigiAlg::Save_onelayer_signal(edm4hep::TrackerHitCollection* _trkhitVec)
{
  for (const auto& _item : map_cell_strip) 
  {
    if(_item.second == -1) continue;
    key = _item.first;
    cellid = key[0];
    Edep = map_cell_edep[key];
    if ( Edep < m_EdepMin ) continue;  
    ddpos = m_cellIDConverter->position(cellid);
    pos = edm4hep::Vector3d(ddpos.x()*10,ddpos.y()*10,ddpos.z()*10);//mm
    Save_trkhit(_trkhitVec, key, 3, cellid1, pos);  //just one layer signal in a superlayer
  }
}

void MuonDigiAlg::Save_pos(int _i)
{
  if(_i == 1)
  {
    cellid1 = key1[0];
    ddpos1 = m_cellIDConverter->position(cellid1);
    pos1 = map_cell_pos[key1];    
  }
  if(_i == 2)
  {
    cellid2 = key2[0];
    ddpos2 = m_cellIDConverter->position(cellid2);
    pos2 = map_cell_pos[key2];    
  }  
}
