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

#include "TVector2.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <map>
#include <random>
#include <vector>


using namespace std ;

DECLARE_COMPONENT( MuonDigiAlg )

MuonDigiAlg::MuonDigiAlg(const std::string& name, ISvcLocator* svcLoc)
: Algorithm(name, svcLoc)
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
  return Algorithm::initialize();
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

  int hit_data[6];
  double hit_Edep, hit_xydist, hit_sipm_length, hit_mcpz;
  double hit_time;
  edm4hep::Vector3d hit_pos;
  dd4hep::Position hit_ddpos;
  unsigned long long cellid;
  std::array<unsigned long long, 2> cell_key;

  if(STHCol->size()>0)
  {    
    for(auto simhit : *STHCol)
    {
      GetSimHit(simhit, m_decoder_barrel, 2, hit_data, hit_Edep, cellid, hit_xydist, hit_pos, hit_ddpos, cell_key, hit_mcpz, hit_time);
      debug()<<"Barrel Simulation Hit::::::: "<<hit_data[5]<<" "<<hit_data[2]<<" "<<hit_data[1]<<" "<<hit_data[0]<<" "<<hit_Edep<<endmsg;
      SaveData_mapcell(cell_key, hit_Edep, hit_data, hit_pos, hit_time);
    }
    for (const auto& item : map_cell_edep) 
    {
      cell_key = item.first;
      hit_data[0] = map_cell_pdgid[cell_key];
      hit_data[1] = map_cell_layer[cell_key];
      hit_data[2] = map_cell_slayer[cell_key];
      hit_data[3] = map_cell_strip[cell_key];
      hit_data[4] = map_cell_env[cell_key];
      hit_data[5] = map_cell_fe[cell_key];
      hit_sipm_length = Gethit_sipm_length_Barrel(m_cellIDConverter->position(cell_key[0]), map_cell_pos[cell_key], hit_data);
      map_cell_adc[cell_key] = EdeptoADC(item.second, hit_sipm_length);
      debug() <<"ADC::: "<<map_cell_adc[cell_key]<<endmsg;
    }
    // 2 for barrel
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
    GetSimHit(simhit, m_decoder_endcap, 0,hit_data, hit_Edep, cellid, hit_xydist, hit_pos, hit_ddpos, cell_key, hit_mcpz, hit_time);
    debug() <<"Endcap Simulation Hit::::::: "<<hit_data[4]<<" "<<hit_data[2]<<" "<<hit_data[1]<<" "<<hit_data[0]<<endmsg;
    SaveData_mapcell(cell_key, hit_Edep, hit_data, hit_pos, hit_time);
  }
  for (const auto& item : map_cell_edep) 
  {
    cell_key = item.first;
    hit_data[0] = map_cell_pdgid[cell_key];
    hit_data[1] = map_cell_layer[cell_key];
    hit_data[2] = map_cell_slayer[cell_key];
    hit_data[3] = map_cell_strip[cell_key];
    hit_data[4] = map_cell_env[cell_key];
    hit_data[5] = map_cell_fe[cell_key];
    hit_sipm_length = Gethit_sipm_length_Endcap(m_cellIDConverter->position(cell_key[0]), map_cell_pos[cell_key], hit_data);
    map_cell_adc[cell_key] = EdeptoADC(item.second, hit_sipm_length);
    debug()<<"ADC::: "<<map_cell_adc[cell_key]<<endmsg;
  }
  // 0 for endcap
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
    delete m_tree;
    delete m_wfile;
  } 
  info() << "Processed " << m_nEvt << " events " << endmsg;
  return Algorithm::finalize();
}


void MuonDigiAlg::MuonHandle(int barrel_or_endcap)
{
  //barrel_or_endcap: 2 is barrel, 0 is endcap

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
      if(barrel_or_endcap == 2) // barrel
      {
        if(map_cell_slayer[key1]!=map_cell_slayer[key2] || 
           map_cell_env[key1]!=map_cell_env[key2] || 
           map_cell_fe[key1]!=map_cell_fe[key2] || 
           map_cell_layer[key1]==map_cell_layer[key2] || 
           map_cell_layer[key1]==2) continue;
      }
      if(barrel_or_endcap == 0) // endcap
      {
        if(map_cell_slayer[key1]!=map_cell_slayer[key2] || 
           map_cell_fe[key1]!=map_cell_fe[key2] || 
           map_cell_layer[key1]==map_cell_layer[key2] || 
           map_cell_layer[key1]==2) continue;
        if(std::abs(map_cell_env[key1] - map_cell_env[key2]) == 2) continue;        
      }
      ddpos2 = m_cellIDConverter->position(key2[0]);
      double ddposi = (barrel_or_endcap == 2) ? ddpos2.z() : ddpos2.x();
      Find_anotherlayer(barrel_or_endcap, key1, key2, ddposi, anotherlayer_cell_num);
    }
    if(anotherlayer_cell_num == 0) continue;
    for(int i=1; i<=anotherlayer_cell_num; i++)
    {
      true_pdgid = (std::abs(map_cell_pdgid[key1]) == 13 && std::abs(map_pdgid[i]) == 13) ? 1 : (map_pdgid[i] == -1) ? 2 : 0;
      edm4hep::Vector3d pos = (barrel_or_endcap == 2) ? edm4hep::Vector3d(ddpos1.x() * 10, ddpos1.y() * 10, map_muonhit[i] * 10) : edm4hep::Vector3d(map_muonhit[i] * 10, ddpos1.y() * 10, ddpos1.z() * 10);
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
  map_cell_pdgid.clear();
  map_cell_time.clear();
  map_muonhit.clear();
  map_pdgid.clear();
}

void MuonDigiAlg::GetSimHit(edm4hep::SimTrackerHit hit_simhit, 
                            dd4hep::DDSegmentation::BitFieldCoder* hit_m_decoder, 
                            int hit_i, 
                            int hit_data[6], 
                            double & hit_Edep, 
                            unsigned long long & hit_cellid, 
                            double & hit_xydist, 
                            edm4hep::Vector3d & hit_pos, 
                            dd4hep::Position & hit_ddpos, 
                            std::array<unsigned long long, 2> & cell_key, 
                            double & hit_mcpz,
                            double & hit_time)
{
  hit_cellid = hit_simhit.getCellID();
  auto mcp = hit_simhit.getMCParticle();
  auto mcppos = mcp.getEndpoint();
  auto pdgid = mcp.getPDG();    
  hit_data[0] = std::abs(mcp.getPDG()); //abspdgid    
  hit_data[1] = hit_m_decoder->get(hit_cellid, "Layer");
  hit_data[2] = hit_m_decoder->get(hit_cellid, "Superlayer");
  hit_data[3] = hit_m_decoder->get(hit_cellid, "Stripe");
  hit_data[4] = hit_m_decoder->get(hit_cellid, "Env");
  // hit_i = 2 means barrel, hit_i !=2 means endcap
  hit_data[5] = (hit_i == 2) ? hit_m_decoder->get(hit_cellid, "Fe") : hit_m_decoder->get(hit_cellid, "Endcap");
  hit_Edep   = (double)hit_simhit.getEDep();//GeV
  hit_xydist = std::sqrt(mcppos[0]*mcppos[0]+mcppos[1]*mcppos[1]);
  hit_pos = hit_simhit.getPosition();
  hit_ddpos = m_cellIDConverter->position(hit_cellid);  
  cell_key = std::array<unsigned long long, 2>{hit_cellid, (unsigned long long)hit_data[0]};
  hit_mcpz = std::abs(mcppos[2]);
  hit_time = (double)hit_simhit.getTime(); // ns
}

double MuonDigiAlg::Gethit_sipm_length_Barrel(dd4hep::Position hit_ddpos, edm4hep::Vector3d hit_pos, int hit_data[6])
{
  // layer 1 is for strips parallel to the beam z-axis
  if(hit_data[1] == 1)
  {
    // We count the number of strips
    // parpendicular to the beam axis, 
    // this number times the strip width 4 cm to get 
    // the length of the strips parallel to the beam axis
    // hit_data[4] is Envolop, left or right
    // hit_data[2] is superlayer
    int tempnum = hit_data[4] + hit_data[2];
    double strips_count = (tempnum % 2 == 0) ? 115.5 : 106.5; // 115 for long-half-barrel, 106 for short-half-barrel
    // strip length
    double length = (strips_count * 4.0) ; // cm
    // z axis positive half
    if (hit_ddpos.z()>=0) 
    {

      // sipm is at the z ~ 0 side
      // (z<0) =================sipm|z=0|sipm=========c======x=====  (z>0)
      //  ----->z direction                   center  hit

      return length/2.0 + (hit_pos[2] - hit_ddpos.z());
    }
    // z axis negative half
    else 
    {
      // sipm is at the z ~ 0 side
      // (z<0) =========c====x====sipm|z=0|sipm====================  (z>0)
      //             center hit      ----->z direction    

      return length/2.0 - (hit_pos[2] - hit_ddpos.z());
    }
  } 
  // layer 2 is for strips perpendicular to the beam axis
  else if(hit_data[1] == 2)
  {
    // We count the number of strips 
    // parallel to beam direction in each slayer,
    // this number times the strip width 4cm 
    // to give the length of strips in each slayer 
    // parpendicular to the beam axis
    // strip length
    double length = strip_length[hit_data[2]-1] * 4.0;
    
    // define two vectors
    TVector2 hit_pos_2vec(hit_pos[0]*0.1, hit_pos[1]*0.1);
    TVector2 strip_pos_2vec(hit_ddpos.x(), hit_ddpos.y());
    // define distance is hit - strip center
    TVector2 distance_2vec = hit_pos_2vec - strip_pos_2vec; 
    // assume sipm is on the clockwise side
    //
    //    ______sipm
    //   /      \
    //  /        \sipm
    // |          |
    // |     .    |       z>0 point to the out side of the screen
    // |          |sipm
    //  \        /
    //   \______/sipm
    
    // define delta_phi as hit - strip_center
    double dphi = TVector2::Phi_mpi_pi(hit_pos_2vec.Phi()-strip_pos_2vec.Phi());
    //check delta_phi if it is positive, 
    // if yes, the hit is on the half of the strip near the sipm
    // if no, the hit is on the half of the strip farther away from the sipm
    if (dphi>=0.0) 
    {
      return length/2.0 - distance_2vec.Mod();
    }
    else 
    {
      return length/2.0 + distance_2vec.Mod();
    }
  }  
  // error
  else 
  {
    error() << "Error:: hit_data[1] can only be 1 for parallel to z axis or 2 for perpendicular to z-axis" << endmsg;
    throw GaudiException("Error:: hit_data[1] can only be 1 for parallel to z axis or 2 for perpendicular to z-axis", "MuonDigiAlg", StatusCode::FAILURE);
  }
}

double MuonDigiAlg::Gethit_sipm_length_Endcap(dd4hep::Position hit_ddpos, edm4hep::Vector3d hit_pos, int hit_data[6])
{
  // strip length
  double length = endcap_strip_length[hit_data[3]-1] * 100; // cm
   
  // define two vectors
  TVector2 hit_pos_2vec(hit_pos[0]*0.1, hit_pos[1]*0.1);
  TVector2 strip_pos_2vec(hit_ddpos.x(), hit_ddpos.y());
  // define distance is hit - strip center
  TVector2 distance_2vec = hit_pos_2vec - strip_pos_2vec;
  // assume sipm is on the clockwise side
  // define delta_phi as hit - strip_center
  double dphi = TVector2::Phi_mpi_pi(hit_pos_2vec.Phi()-strip_pos_2vec.Phi());
  //check delta_phi if it is positive, 
  // if yes, the hit is on the half of the strip near the sipm
  // if no, the hit is on the half of the strip farther away from the sipm
  if (dphi>=0.0)
  {
    return length/2.0 - distance_2vec.Mod();
  }
  else
  {
    return length/2.0 + distance_2vec.Mod();
  }
}

double MuonDigiAlg::EdeptoADC(double hit_Edep, double hit_sipm_length)
{
  // digitize to ADC 
  double ADCmean = 34*hit_Edep*47.09/(23*0.00141);//mV
  if(hit_sipm_length>10)
  {
    ADCmean = (16.0813*std::exp(-1*hit_sipm_length/50.8147)+19.5474)*hit_Edep*47.09/(23*0.00141);//mV
  }
  return std::max(m_randSvc->generator(Rndm::Landau(ADCmean,7.922))->shoot(), 0.0);
}

void MuonDigiAlg::SaveData_mapcell(std::array<unsigned long long, 2> cell_key, double hit_Edep, int hit_data[6], edm4hep::Vector3d hit_pos, double hit_time)
{
  // check if the key exists, if not initialize be zero
  if ( map_cell_edep.find(cell_key)==map_cell_edep.end() )
  {
    map_cell_edep[cell_key] = 0.0;
  }
  // fill the cell loop
  map_cell_edep[cell_key] += hit_Edep;
  map_cell_layer[cell_key] = hit_data[1];
  map_cell_slayer[cell_key] = hit_data[2];
  map_cell_strip[cell_key] = hit_data[3];
  map_cell_fe[cell_key] = hit_data[5];
  map_cell_env[cell_key] = hit_data[4];
  map_cell_pos[cell_key] = hit_pos;
  map_cell_pdgid[cell_key] = hit_data[0];
  // smear the time:
  map_cell_time[cell_key] = std::max(m_randSvc->generator(Rndm::Gauss(hit_time, m_time_resolution))->shoot(), 0.0);
}

void MuonDigiAlg::Cut3()
{
  //some criteria
  for(const auto& item : map_cell_edep)
  {
    if (map_cell_edep[item.first] <= m_EdepMin || 
        m_randSvc->generator(Rndm::Flat(0, 1))->shoot() > m_hitEff || 
        map_cell_adc[item.first] <= m_hit_Edep_min || 
        (map_cell_adc[item.first] > m_hit_Edep_max && m_hit_Edep_max>0) )
    {
      map_cell_edep[item.first] = 0;
    }
  }
}

void MuonDigiAlg::Find_anotherlayer(int barrel_or_endcap, std::array<unsigned long long, 2> key1, std::array<unsigned long long, 2> key2, double ddposi, int & anotherlayer_cell_num)
{
  map_cell_strip[key1] = -1;
  map_cell_strip[key2] = -1;
  edm4hep::Vector3d pos1 = map_cell_pos[key1];
  edm4hep::Vector3d pos2 = map_cell_pos[key2];
  // barrel_or_endcap: 2 is barrel, 0 is endcap
  // barrel is [2] (z), endcap is [0] (x)
  double Hit_min = pos1[barrel_or_endcap]*0.1-12;//10cm+2cm
  double Hit_max = pos1[barrel_or_endcap]*0.1+12;
  if(ddposi<Hit_min || ddposi>Hit_max) return;
  map_muonhit[++anotherlayer_cell_num] = ddposi; 
  map_pdgid[anotherlayer_cell_num] = -1;      
  if(std::abs(pos1[0]-pos2[0])>40 && std::abs(pos1[1]-pos2[1])>40 && std::abs(pos1[2]-pos2[2])>40) return;
  map_pdgid[anotherlayer_cell_num] = map_cell_pdgid[key2];
}

void MuonDigiAlg::Save_trkhit(edm4hep::TrackerHitCollection* trkhitVec, std::array<unsigned long long, 2> key, int pdgid, edm4hep::Vector3d pos)
{
  edm4hep::MutableTrackerHit trkHit;
  trkHit.setEDep((float)map_cell_adc[key]);
  trkHit.setQuality(pdgid);
  trkHit.setCellID(key[0]);
  trkHit.setPosition(pos);
  trkHit.setTime((float)map_cell_time[key]);
  trkhitVec->push_back( trkHit );
}

void MuonDigiAlg::Save_onelayer_signal(edm4hep::TrackerHitCollection* trkhitVec)
{
  for (const auto& item : map_cell_strip) 
  {
    if(item.second == -1) continue;
    std::array<unsigned long long, 2> key = item.first;
    if ( map_cell_edep[key] <= m_EdepMin ) continue;  
    dd4hep::Position ddpos = m_cellIDConverter->position(key[0]);
    edm4hep::Vector3d pos = edm4hep::Vector3d(ddpos.x()*10,ddpos.y()*10,ddpos.z()*10);//mm
    Save_trkhit(trkhitVec, key, 3, pos);  //just one layer signal in a superlayer
  }
}
