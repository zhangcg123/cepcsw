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
//#include "TRandom3.h"
//#include <TRandom.h>
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
    hitloop = new TTree("h1", "muonbarreldigi_hitloop");
    eventloop = new TTree("e1", "muonbarreldigi_eventloop");
    hitloop->Branch("MuonBarrel_cellid", &MuonBarrel_cellid);
    hitloop->Branch("MuonBarrel_posx", &MuonBarrel_posx);
    hitloop->Branch("MuonBarrel_posy", &MuonBarrel_posy);
    hitloop->Branch("MuonBarrel_posz", &MuonBarrel_posz);
    eventloop->Branch("MuonBarrel_stripcellid", &MuonBarrel_stripcellid);
    eventloop->Branch("MuonBarrel_stripedep", &MuonBarrel_stripedep);
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
  

  int strip_length[8] = {17, 29, 41, 53, 65, 77, 89, 101};

  for(auto simhit : *STHCol)
  {
    auto cellid = simhit.getCellID();
    auto Edep   = simhit.getEDep();//GeV
    int  layer  = m_decoder->get(cellid, "Layer");
    int  slayer = m_decoder->get(cellid, "Superlayer");
    int  strip  = m_decoder->get(cellid, "Stripe");
    int  Fe     = m_decoder->get(cellid, "Fe");
    int  Env    = m_decoder->get(cellid, "Env");
    auto& pos = simhit.getPosition();

    dd4hep::Position hitpos = m_cellIDConverter->position(cellid);
    debug() << "Position::   " << hitpos.x() << " " << hitpos.y() << " " << hitpos.z() << endmsg;

    int testtemp = 0;
    if(Edep<0.01&&Edep>0.0001)
    {
      double hit_strip_length = std::sqrt((hitpos.x()-pos[0]*0.1)*(hitpos.x()-pos[0]*0.1)+(hitpos.y()-pos[1]*0.1)*(hitpos.y()-pos[1]*0.1)+(hitpos.z()-pos[2]*0.1)*(hitpos.z()-pos[2]*0.1));
      double hit_sipm_length;
      if(layer == 1)
      {
        int tempnum = Env + slayer;
        if(tempnum%2 == 0)
        {
          hit_sipm_length = 115 * 2 - hit_strip_length;
        }
        else
        {
          hit_sipm_length = 106 * 2 - hit_strip_length;
        }
      } 
      if(layer == 2)
      {
        hit_sipm_length = strip_length[slayer-1] * 2 - hit_strip_length;
      }

      double ADCmean = 34*Edep*47.09/(23*0.00141);//mV
      if(hit_sipm_length>10)
      {
        ADCmean = (16.0813*std::exp(-1*hit_sipm_length/50.8147)+19.5474)*Edep*47.09/(23*0.00141);//mV
      }
      double ADC = rand_muon.Landau(ADCmean,7.922);
      if(ADC<0)
      {
        ADC = 0;
      }
      //std::cout<<hit_sipm_length<<" "<<Edep<<" "<<ADCmean<<" "<<ADC<<std::endl;

      if(cellidtemp.size()>0)
      {
        for(int i=0; i<cellidtemp.size(); i++)
        {
          if(cellid == cellidtemp[i])
          {
            edeptemp[i]+=Edep;
            ADCtemp[i]+=ADC;
            testtemp++;
            break;
          }
        }
      }
      if(testtemp==0)
      {
        cellidtemp.push_back(cellid);
        edeptemp.push_back(Edep);
        ADCtemp.push_back(ADC);
      }
      if(_writeNtuple)
      {
        MuonBarrel_cellid.push_back(cellid);
        MuonBarrel_posx.push_back(pos[0]);
        MuonBarrel_posy.push_back(pos[1]);
        MuonBarrel_posz.push_back(pos[2]);
      }
    }
  }

  for(int i=0; i<cellidtemp.size(); i++)
  {
    if(edeptemp[i]>0.0005&&edeptemp[i]<0.005&&ADCtemp[i]<1000)
    {
      double random_num = rand_muon.Uniform(0, 1);
      if(random_num>0.1)
      {
        if(_writeNtuple)
        {
          MuonBarrel_stripcellid.push_back(cellidtemp[i]);
          MuonBarrel_stripedep.push_back(edeptemp[i]);
        }
        edm4hep::MutableTrackerHit trkHit;
        dd4hep::Position hitpos1 = m_cellIDConverter->position(cellidtemp[i]);
        edm4hep::Vector3d position_strip(hitpos1.x(),hitpos1.y(),hitpos1.z());
        //std::cout<<"Position::   "<<hitpos.x()<<" "<<hitpos.y()<<" "<<hitpos.z()<<std::endl;
        trkHit.setEDep(ADCtemp[i]);
        trkHit.setCellID(cellidtemp[i]);
        trkHit.setPosition(position_strip);
        trkhitVec->push_back( trkHit );
      }
    }
  }

  if(_writeNtuple)
  {
    hitloop->Fill();
    eventloop->Fill();
  }

  m_nEvt++;
  return StatusCode::SUCCESS;
}

StatusCode MuonDigiAlg::finalize()
{

  if(_writeNtuple)
  {
    m_wfile->cd();
    hitloop->Write();
    eventloop->Write();
    m_wfile->Close();
    delete m_wfile, hitloop, eventloop; 
  } 
  info() << "Processed " << m_nEvt << " events " << endmsg;
  return GaudiAlgorithm::finalize();
}

void MuonDigiAlg::Clear()
{
  MuonBarrel_cellid.clear();
  MuonBarrel_posx.clear();
  MuonBarrel_posy.clear();
  MuonBarrel_posz.clear();
  MuonBarrel_stripcellid.clear();
  MuonBarrel_stripedep.clear();
  cellidtemp.clear();
  edeptemp.clear();
  ADCtemp.clear();
}
