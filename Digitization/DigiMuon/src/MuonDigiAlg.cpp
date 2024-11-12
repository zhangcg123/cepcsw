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
  m_cellIDConverter = new dd4hep::rec::CellIDPositionConverter(*m_dd4hep);
  if(!m_cellIDConverter)
  {
    error() << "Failed to get the m_cellIDConverter." << endmsg;
    return StatusCode::FAILURE;
  }

  std::string readout_name1 = m_inputMuonBarrel.objKey(); 
  debug() << "Readout name: " << readout_name1 << endmsg;
  m_decoder1 = m_geosvc->getDecoder(readout_name1);
  if(!m_decoder1)
  {
    error() << "Failed to get the decoder. " << endmsg;
    return StatusCode::FAILURE;
  }

  std::string readout_name2 = m_inputMuonEndcap.objKey(); 
  debug() << "Readout name: " << readout_name2 << endmsg;
  m_decoder2 = m_geosvc->getDecoder(readout_name2);
  if(!m_decoder2)
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
  for(int muonmode = 0; muonmode<2; muonmode++)
  {
    if(muonmode == 0)//muon barrel run
    {
      Clear();
      if(m_MuonMode == 2) continue;
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
        continue;
      }
      if(STHCol->size()==0) continue;
      debug() << m_inputMuonBarrel.fullKey() << " has SimTrackerHit "<< STHCol->size() << endmsg;
      
      // number of strips parallel to beam direction 
      // in each slayer, each strip width=4cm, so 
      int strip_length[6] = {26, 38, 50, 62, 74, 86};

      // define variables to be repeatedly used later
      std::array<unsigned long long, 2> key;
      int pdgid;
      unsigned long long cellid, abspdgid; 
      double Edep, ADC, ADCmean;
      int layer, slayer, strip, Fe, Env;
      edm4hep::Vector3d pos;
      dd4hep::Position ddpos;
      // loop over all hits
      for(auto simhit : *STHCol)
      {
        auto mcp = simhit.getMCParticle();
        cellid = simhit.getCellID();
        pdgid = mcp.getPDG();
        abspdgid = std::abs(mcp.getPDG());
        key = std::array<unsigned long long, 2>{cellid, abspdgid};
        Edep   = simhit.getEDep();//GeV
        layer  = m_decoder1->get(cellid, "Layer");
        slayer = m_decoder1->get(cellid, "Superlayer");
        strip  = m_decoder1->get(cellid, "Stripe");
        Fe     = m_decoder1->get(cellid, "Fe");
        Env    = m_decoder1->get(cellid, "Env");
        pos = simhit.getPosition();
        ddpos = m_cellIDConverter->position(cellid);
        debug() << "Position::   " << ddpos.x() << " " << ddpos.y() << " " << ddpos.z() << endmsg;

        // if not satisfy energy requirement, skip this hit
        if ( Edep>0.1 || Edep<0.000001 ) continue;
        
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
        map_cell_edep[key] += Edep;
        map_cell_adc[key] += ADC;
        map_cell_layer[key] = layer;
        map_cell_slayer[key] = slayer;
        map_cell_strip[key] = strip;
        map_cell_fe[key] = Fe;
        map_cell_env[key] = Env;
        map_cell_pos[key] = pos;
        map_cell_pdgid[key] = pdgid;

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
      std::array<unsigned long long, 2> key1;
      unsigned long long cellid1; 
      int layer1, slayer1, strip1, Fe1, Env1, pdgid1;
      edm4hep::Vector3d pos1;
      dd4hep::Position ddpos1;

      std::array<unsigned long long, 2> key2;
      unsigned long long cellid2; 
      int layer2, slayer2, strip2, Fe2, Env2, pdgid2;
      edm4hep::Vector3d pos2;
      dd4hep::Position ddpos2;

      if(map_cell_edep.size()==0) continue;
      //some criteria
      for(const auto& item : map_cell_edep)
      {
        key = item.first;
        if (map_cell_edep[key] < m_EdepMin || rand_muon.Uniform(0, 1)>m_hitEff || map_cell_adc[key] == 0)
        {
          map_cell_edep[key] = 0;
        }
      }

      double muonhitx, muonhity, muonhitz;
      double Hit_max, Hit_min;

      // loop over all cells 
      for (const auto& item1 : map_cell_edep) 
      {
        key1 = item1.first;
        cellid1 = key1[0];
        Edep = map_cell_edep[key1];
        if ( Edep < 0.0001 ) continue;  
        int anotherlayer_cell_num = 0;

        ADC = map_cell_adc[key1];
        layer1 = map_cell_layer[key1];
        slayer1 = map_cell_slayer[key1];
        strip1 = map_cell_strip[key1];
        Fe1 = map_cell_fe[key1];
        Env1 = map_cell_env[key1];
        pos1 = map_cell_pos[key1];
        pdgid1 = map_cell_pdgid[key1];
        ddpos1 = m_cellIDConverter->position(cellid1);

        for (const auto& item2 : map_cell_edep) 
        {
          key2 = item2.first;
          cellid2 = key2[0];
          Edep = map_cell_edep[key2];
          if ( Edep < 0.0001 ) continue;  

          layer2 = map_cell_layer[key2];
          slayer2 = map_cell_slayer[key2];
          strip2 = map_cell_strip[key2];
          Fe2 = map_cell_fe[key2];
          Env2 = map_cell_env[key2];
          pos2 = map_cell_pos[key2];
          pdgid2 = map_cell_pdgid[key2];
          ddpos2 = m_cellIDConverter->position(cellid2);    

          if(slayer1!=slayer2 || Fe1!=Fe2 || Env1!=Env2 || layer1==layer2 || layer1==2) continue;//layer1 = 1
          Hit_min = pos1[2]*0.1-12;
          Hit_max = pos1[2]*0.1+12;
          if(ddpos2.z()<Hit_min || ddpos2.z()>Hit_max) continue;
          anotherlayer_cell_num++;
          map_muonhit[anotherlayer_cell_num] = ddpos2.z(); 
          map_pdgid[anotherlayer_cell_num] = -1;      
          if(std::abs(pos1[0]-pos2[0])>40 && std::abs(pos1[1]-pos2[1])>40 && std::abs(pos1[2]-pos2[2])>40) continue;
          map_pdgid[anotherlayer_cell_num] = pdgid2;
        }

        if(anotherlayer_cell_num == 0) continue;
        if(anotherlayer_cell_num > 0)
        {
          for(int i=1; i<=anotherlayer_cell_num; i++)
          {
              muonhitx = ddpos1.x();
              muonhity = ddpos1.y();
              muonhitz = map_muonhit[i];
            // store to raw hit
            edm4hep::MutableTrackerHit trkHit;
            int true_pdgid = 0;
            if(std::abs(pdgid1) == 13 && std::abs(map_pdgid[i]) == 13)
            {
              true_pdgid = 1;//pdgid->true muon hit
            }
            if(map_pdgid[i]==-1)
            {
              true_pdgid = 2;//ghost point
            }
            pos = edm4hep::Vector3d(muonhitx*10,muonhity*10,muonhitz*10);//mm
            trkHit.setEDep(ADC);
            trkHit.setQuality(true_pdgid);
            trkHit.setCellID(cellid1);
            trkHit.setPosition(pos);
            trkhitVec->push_back( trkHit );
          }
        }
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
    }
    if(muonmode == 1)//muon endcap run
    {
      Clear();
      if(m_MuonMode == 1) continue;
      trkhitVec = m_outputMuonEndcap.createAndPut();
      const edm4hep::SimTrackerHitCollection* STHCol = nullptr;
      try 
      {
        STHCol = m_inputMuonEndcap.get();
      }
      catch(GaudiException &e)
      {
        debug() << "Collection " << m_inputMuonEndcap.fullKey() << " is unavailable in event " << m_nEvt << endmsg;
        continue;
      }
      if(STHCol->size()==0) continue;
      debug() << m_inputMuonEndcap.fullKey() << " has SimTrackerHit "<< STHCol->size() << endmsg;

      double endcap_strip_length[193] = {2.12, 2.12, 2.12, 2.13, 2.14, 2.15, 2.16, 2.17, 2.19, 2.22,
                                         2.24, 2.28, 2.32, 2.37, 2.45, 2.65, 2.64, 2.63, 2.62, 2.61,
                                         2.60, 2.59, 2.57, 2.56, 2.54, 2.53, 2.51, 2.50, 2.48, 2.46, 
                                         2.44, 2.42, 2.40, 2.38, 2.36, 2.33, 2.31, 2.28, 2.26, 2.23,
                                         2.20, 2.17, 2.14, 2.11, 2.07, 2.04, 2.00, 1.97, 1.93, 1.89, 
                                         1.84, 1.80, 1.75, 1.70, 1.65, 1.60, 1.54, 1.48, 1.42, 1.35, 
                                         1.28, 1.20, 1.12, 1.02, 0.92, 0.80, 0.65, 0.46, 2.20, 2.20,
                                         2.20, 2.20, 2.20, 2.20, 2.20, 2.21, 2.21, 2.21, 2.21, 2.22,
                                         2.22, 2.22, 2.23, 2.23, 2.23, 2.24, 2.24, 2.25, 2.25, 2.26,
                                         2.26, 2.27, 2.28, 2.28, 2.29, 2.30, 2.31, 2.32, 2.32, 2.33,
                                         2.34, 2.35, 2.36, 2.38, 2.39, 2.40, 2.41, 2.43, 2.44, 2.45,
                                         2.47, 2.49, 2.50, 2.52, 2.54, 2.56, 2.58, 2.60, 2.62, 2.65, 
                                         2.67, 2.70, 2.73, 2.76, 2.79, 2.82, 2.86, 2.90, 2.94, 2.99,
                                         3.04, 3.10, 3.16, 3.23, 3.31, 3.41, 3.53, 3.70, 4.14, 4.12, 
                                         4.09, 4.06, 4.03, 4.00, 3.97, 3.94, 3.91, 3.87, 3.84, 3.81, 
                                         3.77, 3.74, 3.70, 3.67, 3.63, 3.59, 3.55, 3.51, 3.47, 3.43, 
                                         3.38, 3.34, 3.30, 3.25, 3.20, 3.15, 3.10, 3.05, 3.00, 2.95, 
                                         2.89, 2.83, 2.77, 2.71, 2.65, 2.58, 2.52, 2.45, 2.37, 2.30, 
                                         2.22, 2.14, 2.05, 1.96, 1.86, 1.76, 1.65, 1.53, 1.40, 1.25, 
                                         1.09, 0.89, 0.63};


      // define variables to be repeatedly used later
      std::array<unsigned long long, 2> key;
      int pdgid;
      unsigned long long cellid, abspdgid; 
      double Edep, ADC, ADCmean;
      int layer, slayer, strip, Env, Endcap;
      edm4hep::Vector3d pos;
      dd4hep::Position ddpos;
      // loop over all hits
      for(auto simhit : *STHCol)
      {
        auto mcp = simhit.getMCParticle();
        cellid = simhit.getCellID();
        pdgid = mcp.getPDG();
        abspdgid = std::abs(mcp.getPDG());
        key = std::array<unsigned long long, 2>{cellid, abspdgid};
        Edep   = simhit.getEDep();//GeV
        layer  = m_decoder2->get(cellid, "Layer");
        slayer = m_decoder2->get(cellid, "Superlayer");
        strip  = m_decoder2->get(cellid, "Stripe");
        Endcap = m_decoder2->get(cellid, "Endcap");
        Env    = m_decoder2->get(cellid, "Env");
        pos = simhit.getPosition();
        ddpos = m_cellIDConverter->position(cellid);
        debug() << "Position::   " << ddpos.x() << " " << ddpos.y() << " " << ddpos.z() << endmsg;

        // if not satisfy energy requirement, skip this hit
        if ( Edep>0.1 || Edep<0.000001 ) continue;

        //calculate hit strip length
        double hit_strip_length = std::sqrt((ddpos.x()-pos[0]*0.1)*(ddpos.x()-pos[0]*0.1)+(ddpos.y()-pos[1]*0.1)*(ddpos.y()-pos[1]*0.1)+(ddpos.z()-pos[2]*0.1)*(ddpos.z()-pos[2]*0.1));
        strip--; // strip id begin at 1
        double hit_sipm_length = endcap_strip_length[strip] * 0.5 * 100 - hit_strip_length;//cm

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

        map_cell_edep[key] += Edep;
        map_cell_adc[key] += ADC;
        map_cell_layer[key] = layer;
        map_cell_slayer[key] = slayer;
        map_cell_strip[key] = strip;
        map_cell_fe[key] = Endcap;
        map_cell_env[key] = Env;
        map_cell_pos[key] = pos;
        map_cell_pdgid[key] = pdgid;
      }

      std::array<unsigned long long, 2> key1;
      unsigned long long cellid1; 
      int layer1, slayer1, strip1, Fe1, Env1, pdgid1;
      edm4hep::Vector3d pos1;
      dd4hep::Position ddpos1;

      std::array<unsigned long long, 2> key2;
      unsigned long long cellid2; 
      int layer2, slayer2, strip2, Fe2, Env2, pdgid2;
      edm4hep::Vector3d pos2;
      dd4hep::Position ddpos2;

      if(map_cell_edep.size()==0) continue;

      for(const auto& item : map_cell_edep)
      {
        key = item.first;
        if (map_cell_edep[key] < m_EdepMin || rand_muon.Uniform(0, 1)>m_hitEff || map_cell_adc[key] == 0)
        {
          map_cell_edep[key] = 0;
        }
      }

      double muonhitx, muonhity, muonhitz;
      double Hit_max, Hit_min;

      // loop over all cells 
      for (const auto& item1 : map_cell_edep) 
      {
        key1 = item1.first;
        cellid1 = key1[0];
        Edep = map_cell_edep[key1];
        if ( Edep < 0.0001 ) continue;  
        int anotherlayer_cell_num = 0;

        ADC = map_cell_adc[key1];
        layer1 = map_cell_layer[key1];
        slayer1 = map_cell_slayer[key1];
        strip1 = map_cell_strip[key1];
        Fe1 = map_cell_fe[key1];//endcap
        Env1 = map_cell_env[key1];
        pos1 = map_cell_pos[key1];
        pdgid1 = map_cell_pdgid[key1];
        ddpos1 = m_cellIDConverter->position(cellid1);

        for (const auto& item2 : map_cell_edep) 
        {
          key2 = item2.first;
          cellid2 = key2[0];
          Edep = map_cell_edep[key2];
          if ( Edep < 0.0001 ) continue;  

          layer2 = map_cell_layer[key2];
          slayer2 = map_cell_slayer[key2];
          strip2 = map_cell_strip[key2];
          Fe2 = map_cell_fe[key2];//endcap
          Env2 = map_cell_env[key2];
          pos2 = map_cell_pos[key2];
          pdgid2 = map_cell_pdgid[key2];
          ddpos2 = m_cellIDConverter->position(cellid2);    

          if(Fe1!=Fe2 || slayer1!=slayer2 || layer1==layer2 || layer1==2) continue;
          if(std::abs(Env2-Env1) == 2) continue;
          Hit_min = pos1[0]*0.1-12;
          Hit_max = pos1[0]*0.1+12;
          if(ddpos2.x()<Hit_min || ddpos2.x()>Hit_max) continue;
          anotherlayer_cell_num++;
          map_muonhit[anotherlayer_cell_num] = ddpos2.x();      
          map_pdgid[anotherlayer_cell_num] = -1;      
          if(std::abs(pos1[0]-pos2[0])>40 && std::abs(pos1[1]-pos2[1])>40 && std::abs(pos1[2]-pos2[2])>40) continue;
          map_pdgid[anotherlayer_cell_num] = pdgid2;         
        }
        if(anotherlayer_cell_num == 0) continue;
        if(anotherlayer_cell_num > 0)
        {
          for(int i=1; i<=anotherlayer_cell_num; i++)
          {
            muonhitx = map_muonhit[i];
            muonhity = ddpos1.y();
            muonhitz = ddpos1.z();
            // store to raw hit
            edm4hep::MutableTrackerHit trkHit;
            int true_pdgid = 0;
            if(std::abs(pdgid1) == 13 && std::abs(map_pdgid[i]) == 13)
            {
              true_pdgid = 1;//pdgid->true muon hit
            }
            if(map_pdgid[i]==-1)
            {
              true_pdgid = 2;//ghost point
            }
            pos = edm4hep::Vector3d(muonhitx*10,muonhity*10,muonhitz*10);
            trkHit.setEDep(ADC);
            trkHit.setQuality(true_pdgid);
            trkHit.setCellID(cellid1);
            trkHit.setPosition(pos);
            trkhitVec->push_back( trkHit );
          }
        }
      }
    }
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
  map_cell_pos.clear();
  map_muonhit.clear();
  map_cell_pdgid.clear();
  map_pdgid.clear();
}
