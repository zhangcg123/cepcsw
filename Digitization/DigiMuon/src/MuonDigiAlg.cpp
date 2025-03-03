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

  int all_message[6];
  double Edep, xydist, hit_sipm_length, ADC, mcpz;
  edm4hep::Vector3d pos;
  dd4hep::Position ddpos;
  unsigned long long cellid;
  std::array<unsigned long long, 2> key;

  if(STHCol->size()>0)
  {    
    for(auto simhit : *STHCol)
    {
      GetSimHit(simhit, m_decoder_barrel, 2, all_message, Edep, cellid, xydist, pos, ddpos, key, mcpz);
      debug()<<"Barrel Simulation Hit::::::: "<<all_message[5]<<" "<<all_message[2]<<" "<<all_message[1]<<" "<<all_message[0]<<" "<<Edep<<endmsg;
      //if ( Edep > m_hit_Edep_max || Edep < m_hit_Edep_min ) continue;                 // Cut1: Edep of one hit
      //if ( xydist < 5162 && mcpz < 5475 || all_message[0] != 13 ) continue;  // Cut2: For Muonid Efficiency
      SaveData_mapcell(key, Edep, all_message, pos);
    }
    for (const auto& item : map_cell_edep) 
    {
      key = item.first;
      all_message[0] = map_cell_pdgid[key];
      all_message[1] = map_cell_layer[key];
      all_message[2] = map_cell_slayer[key];
      all_message[3] = map_cell_strip[key];
      all_message[4] = map_cell_env[key];
      all_message[5] = map_cell_fe[key];
      hit_sipm_length = Gethit_sipm_length_Barrel(m_cellIDConverter->position(key[0]), map_cell_pos[key], all_message);
      map_cell_adc[key] = EdeptoADC(item.second, hit_sipm_length);
      debug() <<"ADC::: "<<map_cell_adc[key]<<endmsg;
    }

    MuonHandle(2);
  }

  // Endcaps:
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
    GetSimHit(simhit, m_decoder_endcap, 0, all_message, Edep, cellid, xydist, pos, ddpos, key, mcpz);
    //if ( Edep > m_hit_Edep_max || Edep < m_hit_Edep_min ) continue;                 // Cut1: Edep of one hit
    //if ( xydist < 5162 && mcpz < 5475 || all_message[0] != 13 ) continue;  // Cut2: For Muonid Efficiency
    debug() <<"Endcap Simulation Hit::::::: "<<all_message[4]<<" "<<all_message[2]<<" "<<all_message[1]<<" "<<all_message[0]<<endmsg;
    // hit_sipm_length = Gethit_sipm_length_Endcap(ddpos, pos, all_message);
    // ADC = EdeptoADC(Edep, hit_sipm_length);
    SaveData_mapcell(key, Edep, all_message, pos);
  }
  for (const auto& item : map_cell_edep) 
  {
    key = item.first;
    all_message[0] = map_cell_pdgid[key];
    all_message[1] = map_cell_layer[key];
    all_message[2] = map_cell_slayer[key];
    all_message[3] = map_cell_strip[key];
    all_message[4] = map_cell_env[key];
    all_message[5] = map_cell_fe[key];
    hit_sipm_length = Gethit_sipm_length_Endcap(m_cellIDConverter->position(key[0]), map_cell_pos[key], all_message);
    map_cell_adc[key] = EdeptoADC(item.second, hit_sipm_length);
    debug()<<"ADC::: "<<map_cell_adc[key]<<endmsg;
  }

  MuonHandle(0);
  Clear();
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

void MuonDigiAlg::MuonHandle(int _i)
{
  if(map_cell_edep.size()==0) return;
  Cut3();
  std::array<unsigned long long, 2> key1, key2;
  dd4hep::Position ddpos1, ddpos2;
  int anotherlayer_cell_num, true_pdgid;
  for (const auto& item1 : map_cell_edep) 
  {
    key1 = item1.first;
    if(map_cell_edep[key1] <= m_EdepMin) continue;
    ddpos1 = m_cellIDConverter->position(key1[0]);
    anotherlayer_cell_num = 0;   
    for (const auto& item2 : map_cell_edep) 
    {
      key2 = item2.first;
      if(map_cell_edep[key2] <= m_EdepMin) continue;
      if(_i == 2)
      {
        if(map_cell_slayer[key1]!=map_cell_slayer[key2] || map_cell_env[key1]!=map_cell_env[key2] || map_cell_fe[key1]!=map_cell_fe[key2] || map_cell_layer[key1]==map_cell_layer[key2] || map_cell_layer[key1]==2) continue;
      }
      if(_i == 0)
      {
        if(map_cell_slayer[key1]!=map_cell_slayer[key2] || map_cell_fe[key1]!=map_cell_fe[key2] || map_cell_layer[key1]==map_cell_layer[key2] || map_cell_layer[key1]==2) continue;
        if(std::abs(map_cell_env[key1] - map_cell_env[key2]) == 2) continue;        
      }
      ddpos2 = m_cellIDConverter->position(key2[0]);
      double ddposi = (_i == 2) ? ddpos2.z() : ddpos2.x();
      Find_anotherlayer(_i, key1, key2, ddposi, anotherlayer_cell_num);
    }
    if(anotherlayer_cell_num == 0) continue;
    for(int i=1; i<=anotherlayer_cell_num; i++)
    {
      true_pdgid = (std::abs(map_cell_pdgid[key1]) == 13 && std::abs(map_pdgid[i]) == 13) ? 1 : (map_pdgid[i] == -1) ? 2 : 0;
      edm4hep::Vector3d pos = (_i == 2) ? edm4hep::Vector3d(ddpos1.x() * 10, ddpos1.y() * 10, map_muonhit[i] * 10) : edm4hep::Vector3d(map_muonhit[i] * 10, ddpos1.y() * 10, ddpos1.z() * 10);
      Save_trkhit(trkhitVec, key1, true_pdgid, pos);
    }
  }
  Save_onelayer_signal(trkhitVec);
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

void MuonDigiAlg::GetSimHit(edm4hep::SimTrackerHit _simhit, 
                            dd4hep::DDSegmentation::BitFieldCoder* _m_decoder, 
                            int _i, 
                            int _message[6], 
                            double & _Edep, 
                            unsigned long long & _cellid, 
                            double & _xydist, 
                            edm4hep::Vector3d & _pos, 
                            dd4hep::Position & _ddpos, 
                            std::array<unsigned long long, 2> & _key, 
                            double & _mcpz)
{
  _cellid = _simhit.getCellID();
  auto mcp = _simhit.getMCParticle();
  auto mcppos = mcp.getEndpoint();
  auto pdgid = mcp.getPDG();    
  _message[0] = std::abs(mcp.getPDG()); //abspdgid    
  _message[1] = _m_decoder->get(_cellid, "Layer");
  _message[2] = _m_decoder->get(_cellid, "Superlayer");
  _message[3] = _m_decoder->get(_cellid, "Stripe");
  _message[4] = _m_decoder->get(_cellid, "Env");
  _message[5] = (_i == 2) ? m_decoder_barrel->get(_cellid, "Fe") : m_decoder_endcap->get(_cellid, "Endcap");
  _Edep   = _simhit.getEDep();//GeV
  _xydist = std::sqrt(mcppos[0]*mcppos[0]+mcppos[1]*mcppos[1]);
  _pos = _simhit.getPosition();
  _ddpos = m_cellIDConverter->position(_cellid);  
  _key = std::array<unsigned long long, 2>{_cellid, _message[0]};
  _mcpz = std::abs(mcppos[2]);
}

double MuonDigiAlg::Gethit_sipm_length_Barrel(dd4hep::Position _ddpos, edm4hep::Vector3d _pos, int _message[6])
{
  //calculate hit strip length
  double hit_strip_length = std::sqrt((_ddpos.x()-_pos[0]*0.1)*(_ddpos.x()-_pos[0]*0.1)+
                                      (_ddpos.y()-_pos[1]*0.1)*(_ddpos.y()-_pos[1]*0.1)+
                                      (_ddpos.z()-_pos[2]*0.1)*(_ddpos.z()-_pos[2]*0.1));
  if(_message[1] == 1)
  {
    int tempnum = _message[4] + _message[2];
    double strips_count = (tempnum % 2 == 0) ? 115.5 : 106.5; // 115 for long-half-barrel, 106 for short-half-barrel
    return (strips_count * 4.0) / 2 - hit_strip_length;
  } 
  if(_message[1] == 2)
  {
    // number of strips parallel to beam direction in each slayer, each strip width=4cm, so the number * 4 cm gives the length of strips in each slayer parpendicular to the beam axis
    return (strip_length[_message[2]-1] * 4)/2 - hit_strip_length;
  }  
}

double MuonDigiAlg::Gethit_sipm_length_Endcap(dd4hep::Position _ddpos, edm4hep::Vector3d _pos, int _message[6])
{
  //calculate hit strip length
  double hit_strip_length = std::sqrt((_ddpos.x()-_pos[0]*0.1)*(_ddpos.x()-_pos[0]*0.1)+(_ddpos.y()-_pos[1]*0.1)*(_ddpos.y()-_pos[1]*0.1)+(_ddpos.z()-_pos[2]*0.1)*(_ddpos.z()-_pos[2]*0.1));
  _message[3]--; // strip id begin at 1
  return endcap_strip_length[_message[3]] * 0.5 * 100 - hit_strip_length;//cm  
}

double MuonDigiAlg::EdeptoADC(double _Edep, double _hit_sipm_length)
{
  // digitize to ADC 
  double ADCmean = 34*_Edep*47.09/(23*0.00141);//mV
  if(_hit_sipm_length>10)
  {
    ADCmean = (16.0813*std::exp(-1*_hit_sipm_length/50.8147)+19.5474)*_Edep*47.09/(23*0.00141);//mV
  }
  return std::max(m_randSvc->generator(Rndm::Landau(ADCmean,7.922))->shoot(), 0.0);
}

void MuonDigiAlg::SaveData_mapcell(std::array<unsigned long long, 2> _key, double _Edep, int _message[6], edm4hep::Vector3d _pos)
{
  // fill the cell loop
  map_cell_edep[_key] += _Edep;
  // map_cell_adc[_key] += _ADC;
  map_cell_layer[_key] = _message[1];
  map_cell_slayer[_key] = _message[2];
  map_cell_strip[_key] = _message[3];
  map_cell_fe[_key] = _message[5];
  map_cell_env[_key] = _message[4];
  map_cell_pos[_key] = _pos;
  map_cell_pdgid[_key] = _message[0];
}

void MuonDigiAlg::Cut3()
{
  //some criteria
  for(const auto& item : map_cell_edep)
  {
    if (map_cell_edep[item.first] <= m_EdepMin || m_randSvc->generator(Rndm::Flat(0, 1))->shoot() > m_hitEff || map_cell_adc[item.first] <= m_hit_Edep_min || (map_cell_adc[item.first] > m_hit_Edep_max && m_hit_Edep_max>0) )
    {
      map_cell_edep[item.first] = 0;
    }
  }
}

void MuonDigiAlg::Find_anotherlayer(int _i, std::array<unsigned long long, 2> _key1, std::array<unsigned long long, 2> _key2, double _ddposi, int & _anotherlayer_cell_num)
{
  map_cell_strip[_key1] = -1;
  map_cell_strip[_key2] = -1;
  edm4hep::Vector3d _pos1 = map_cell_pos[_key1];
  edm4hep::Vector3d _pos2 = map_cell_pos[_key2];
  double Hit_min = _pos1[_i]*0.1-12;//10cm+2cm
  double Hit_max = _pos1[_i]*0.1+12;
  if(_ddposi<Hit_min || _ddposi>Hit_max) return;
  map_muonhit[++_anotherlayer_cell_num] = _ddposi; 
  map_pdgid[_anotherlayer_cell_num] = -1;      
  if(std::abs(_pos1[0]-_pos2[0])>40 && std::abs(_pos1[1]-_pos2[1])>40 && std::abs(_pos1[2]-_pos2[2])>40) return;
  map_pdgid[_anotherlayer_cell_num] = map_cell_pdgid[_key2];
}

void MuonDigiAlg::Save_trkhit(edm4hep::TrackerHitCollection* _trkhitVec, std::array<unsigned long long, 2> _key, int _pdgid, edm4hep::Vector3d _pos)
{
  edm4hep::MutableTrackerHit trkHit;
  trkHit.setEDep(map_cell_adc[_key]);
  trkHit.setQuality(_pdgid);
  trkHit.setCellID(_key[0]);
  trkHit.setPosition(_pos);
  _trkhitVec->push_back( trkHit );
}

void MuonDigiAlg::Save_onelayer_signal(edm4hep::TrackerHitCollection* _trkhitVec)
{
  for (const auto& _item : map_cell_strip) 
  {
    if(_item.second == -1) continue;
    std::array<unsigned long long, 2> key = _item.first;
    if ( map_cell_edep[key] <= m_EdepMin ) continue;  
    dd4hep::Position ddpos = m_cellIDConverter->position(key[0]);
    edm4hep::Vector3d pos = edm4hep::Vector3d(ddpos.x()*10,ddpos.y()*10,ddpos.z()*10);//mm
    Save_trkhit(_trkhitVec, key, 3, pos);  //just one layer signal in a superlayer
  }
}
