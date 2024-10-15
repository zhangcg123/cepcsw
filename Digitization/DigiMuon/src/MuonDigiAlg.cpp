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

  // Output collections
  declareProperty("MuonBarrelTrackerHits", m_outputMuonBarrel, "Handle of the output TrackerHit collection");
  //declareProperty("MuonBarrelTrackerHitAssociationCollection", m_assMuonBarrel, "Handle of the Association collection between SimTrackerHit and TrackerHit");
}


StatusCode MuonDigiAlg::initialize()
{
  // set rand seed;
  rand_muon.SetSeed(123456);

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
  std::string readout_name = m_inputMuonBarrel.objKey(); 
  debug() << "Readout name: " << readout_name << endmsg;
  m_cellIDConverter = new dd4hep::rec::CellIDPositionConverter(*m_dd4hep);

  if(!m_cellIDConverter)
  {
    error() << "Failed to get the m_cellIDConverter." << endmsg;
    return StatusCode::FAILURE;
  }
  m_decoder = m_geosvc->getDecoder(readout_name);
  if(!m_decoder)
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


  Clear();
  trkhitVec = m_outputMuonBarrel.createAndPut();
  //auto assVec = m_assMuonBarrel.createAndPut();

  const edm4hep::SimTrackerHitCollection* STHCol = nullptr;
  try 
  {
    STHCol = m_inputMuonBarrel.get();
  }
  catch(GaudiException &e)
  {
    debug() << "Collection " << m_inputMuonBarrel.fullKey() << " is unavailable in event " << m_nEvt << endmsg;
    return StatusCode::SUCCESS;
  }
  if(STHCol->size()==0) return StatusCode::SUCCESS;
  debug() << m_inputMuonBarrel.fullKey() << " has SimTrackerHit "<< STHCol->size() << endmsg;
  

  // number of strips parallel to beam direction 
  // in each slayer, each strip width=4cm, so 
  int strip_length[6] = {26, 38, 50, 62, 74, 86};

  // define variables to be repeatedly used later
  unsigned long long cellid; 
  double Edep, ADC, ADCmean;
  int layer, slayer, strip, Fe, Env;
  edm4hep::Vector3d pos;
  dd4hep::Position ddpos;
  // loop over all hits
  for(auto simhit : *STHCol)
  {
    cellid = simhit.getCellID();
    Edep   = simhit.getEDep();//GeV
    layer  = m_decoder->get(cellid, "Layer");
    slayer = m_decoder->get(cellid, "Superlayer");
    strip  = m_decoder->get(cellid, "Stripe");
    Fe     = m_decoder->get(cellid, "Fe");
    Env    = m_decoder->get(cellid, "Env");
    pos = simhit.getPosition();

    ddpos = m_cellIDConverter->position(cellid);
    debug() << "Position::   " << ddpos.x() << " " << ddpos.y() << " " << ddpos.z() << endmsg;

    // if not satisfy energy requirement, skip this hit
    //if ( Edep>0.01 || Edep<0.0001 ) continue;
    
    //calculate hit strip length
    double hit_strip_length = std::sqrt((ddpos.x()-pos[0]*0.1)*(ddpos.x()-pos[0]*0.1)+(ddpos.y()-pos[1]*0.1)*(ddpos.y()-pos[1]*0.1)+(ddpos.z()-pos[2]*0.1)*(ddpos.z()-pos[2]*0.1));
    double hit_sipm_length;
    
    if(layer == 1)
    {
      int tempnum = Env + slayer;
      if(tempnum%2 == 0)
      {
        // 115 is the number of strips parpendicular 
        // to the beam axis in long-half-barrel
        hit_sipm_length = (115 * 4.0)/2 - hit_strip_length;
      }
      else
      {
        // 106 is the number of strips parpendicular 
        // to the beam axis in short-half-barrel
        hit_sipm_length = (106 * 4.0)/2 - hit_strip_length;
      }
    } 
    if(layer == 2)
    {
      // number of strips parallel to beam direction 
      // in each slayer, each strip width=4cm, so 
      // the number * 4 cm gives the length of strips 
      // in each slayer parpendicular to the beam axis
      hit_sipm_length = (strip_length[slayer-1] * 4)/2 - hit_strip_length;
    }

    // digitize to ADC 
    ADCmean = 34*Edep*47.09/(23*0.00141);//mV
    if(hit_sipm_length>10)
    {
      ADCmean = (16.0813*std::exp(-1*hit_sipm_length/50.8147)+19.5474)*Edep*47.09/(23*0.00141);//mV
    }
    ADC = rand_muon.Landau(ADCmean,7.922);
    if(ADC<0)
    {
      ADC = 0;
    }

    // fill the cell loop
    map_cell_edep[cellid] += Edep;
    map_cell_adc[cellid] += ADC;
    map_cell_layer[cellid] = layer;
    map_cell_slayer[cellid] = slayer;
    map_cell_strip[cellid] = strip;
    map_cell_fe[cellid] = Fe;
    map_cell_env[cellid] = Env;

    // write to tree
    if(_writeNtuple)
    {
      hit_cellid[n_hit] = cellid;
      hit_posx[n_hit] = pos[0];
      hit_posy[n_hit] = pos[1];
      hit_posz[n_hit] = pos[2];
      hit_edep[n_hit] = Edep;
      hit_layer[n_hit] = layer;
      hit_slayer[n_hit] = slayer;
      hit_strip[n_hit] = strip;
      hit_fe[n_hit] = Fe;
      hit_env[n_hit] = Env;
      n_hit++;
    }
    
  }

  // loop over all cells 
  for (const auto& item : map_cell_edep) 
  {
    cellid = item.first;
    Edep = map_cell_edep[cellid];
    ADC = map_cell_adc[cellid];
    layer = map_cell_layer[cellid];
    slayer = map_cell_slayer[cellid];
    strip = map_cell_strip[cellid];
    Fe = map_cell_fe[cellid];
    Env = map_cell_env[cellid];
  
    if ( Edep < 0.0005 ) continue;  // skip cell energy < 50 MeV ?? Why?
    //if ( Edep > 0.005 ) continue; // skip cell energy > 500 MeV ?? Why?
    //if ( ADC > 10000 ) continue;  // skip cell energy > 1000 ADC counts? why?

    // simulate hit efficiency: random > hiteff, skip the hit
    if( rand_muon.Uniform(0, 1) > m_hitEff) continue;

    // store to raw hit
    edm4hep::MutableTrackerHit trkHit;
    ddpos = m_cellIDConverter->position(cellid);
    pos = edm4hep::Vector3d(ddpos.x(),ddpos.y(),ddpos.z());
    trkHit.setEDep(ADC);
    trkHit.setCellID(cellid);
    trkHit.setPosition(pos);
    trkhitVec->push_back( trkHit );


    // write for out ntuple
    if(_writeNtuple)
    {
      cell_cellid[n_cell] = cellid;
      cell_edep[n_cell] = Edep;
      cell_adc[n_cell] = ADC;
      cell_posx[n_cell] = pos[0];
      cell_posy[n_cell] = pos[1];
      cell_posz[n_cell] = pos[2];
      cell_layer[n_cell] = layer;
      cell_slayer[n_cell] = slayer;
      cell_strip[n_cell] = strip;
      cell_fe[n_cell] = Fe;
      cell_env[n_cell] = Env;
      n_cell++;
    }

  }

  if(_writeNtuple)
  {
    m_tree->Fill();
  }

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

}
