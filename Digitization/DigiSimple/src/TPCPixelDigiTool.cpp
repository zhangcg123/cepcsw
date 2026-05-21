#include "TPCPixelDigiTool.h"
#include "k4FWCore/DataHandle.h"
#include "DataHelper/TrackerHitHelper.h"
#include "DetIdentifier/CEPCConf.h"
#include "DetInterface/IGeomSvc.h"
#include "DataHelper/Circle.h"

#include "edm4hep/Vector3f.h"

#include "DD4hep/Detector.h"
#include <DD4hep/Objects.h>
#include "DD4hep/DD4hepUnits.h"
#include "DDRec/ISurface.h"

#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IRndmGen.h"
#include "GaudiKernel/RndmGenerators.h"
#include <gear/GEAR.h>
#include <gear/TPCParameters.h>

#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include <math.h>

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <array>

// #include "DataHelper/Circle.h"
#include "DataHelper/SimpleHelix.h"
#include "DataHelper/LCCylinder.h"
#include "EventSeeder/IEventSeeder.h"
#include <stdexcept>
#include "GearSvc/IGearSvc.h"
#include <UTIL/BitField64.h>
#include <edm4hep/MCParticle.h>
#include "edm4hep/SimTrackerHit.h"

#include "CLHEP/Vector/TwoVector.h"
#include "UTIL/ILDConf.h"

#define TRKHITNCOVMATRIX 6
// using namespace lcio ;

DECLARE_COMPONENT(TPCPixelDigiTool)

StatusCode TPCPixelDigiTool::initialize()
{

  m_geosvc = service<IGeomSvc>("GeomSvc");
  if (!m_geosvc)
  {
    error() << "Failed to get the GeomSvc" << endmsg;
    return StatusCode::FAILURE;
  }


  debug() << "Readout name: " << m_readoutName << endmsg;
  m_decoder = m_geosvc->getDecoder(m_readoutName);
  if (!m_decoder)
  {
    error() << "Failed to get the decoder. " << endmsg;
    return StatusCode::FAILURE;
  }

  _gear = service<IGearSvc>("GearSvc");
  if (!_gear)
  {
    error() << "Failed to find GearSvc ..." << endmsg;
    return StatusCode::FAILURE;
  }
  _GEAR = _gear->getGearMgr();
  _f1 = new TF1("fp", "[0]*pow(1+[1],1+[1])*pow(x/[2],[1])*exp(-(1+[1])*x/[2])/TMath::Gamma(1+[1])", 0, 10000);
  // _f1->SetParameters(3802, 0.487, 2092);
  _f1->SetParameters(_polyParas[0], _polyParas[1], _polyParas[2]);
  x_diffusion_p0=0.;
  y_diffusion_p0=0.;
  x_diffusion_p1=0.;
  y_diffusion_p1=0.;
  t_diffusion_p0 = 0.;
  t_diffusion_p1 = 0.;

  switch (_magnetic) {
    case 2:
        x_diffusion_p0 = 0.001992;
        x_diffusion_p1 = 0.004387;
        y_diffusion_p0 = 0.001877;
        y_diffusion_p1 = 0.004398;
		t_diffusion_p0 = 0.0002519;
		t_diffusion_p1 = 0.00332;
        tpcZRes = tpcTRes * 80; // drift speed 80um/us
        break;

    case 3:
        x_diffusion_p0 = 0.001403;
        x_diffusion_p1 = 0.003011;
        y_diffusion_p0 = 0.001604;
        y_diffusion_p1 = 0.003042;
		t_diffusion_p0 = 5.41279e-05;
		t_diffusion_p1 = 3.28842e-03;
        tpcZRes = tpcTRes * 80; // drift speed 80um/us
        break;

    default:
        x_diffusion_p0 = 0.0;
        x_diffusion_p1 = 0.0;
        y_diffusion_p0 = 0.0;
        y_diffusion_p1 = 0.0;
		t_diffusion_p0 = 5.41279e-05;
		t_diffusion_p1 = 3.28842e-03;
        tpcZRes = tpcTRes * 80; // drift speed 80um/us
        break;
}

 

  info() << "initialized" << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode TPCPixelDigiTool::Call(const edm4hep::SimTrackerHitCollection *simCol, edm4hep::TrackerHitCollection *hitCol,
                                  edm4hep::MCRecoTrackerAssociationCollection *assCol)
{

  edm4hep::SimTrackerHit simhit;
  StatusCode sc = Call(simhit, hitCol, assCol);
  if (sc.isFailure())
    return sc;
  return StatusCode::SUCCESS;
}

StatusCode TPCPixelDigiTool::Call(edm4hep::SimTrackerHit simhit, edm4hep::TrackerHitCollection *hitCol, edm4hep::MCRecoTrackerAssociationCollection *assCol)
{
  DataHandle<edm4hep::SimPrimaryIonizationClusterCollection> m_inputIonClusterCol{"SimPrimaryIonizationClusterCollection", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::SimTrackerHitCollection> m_inputSimCol{m_readoutName, Gaudi::DataHandle::Reader, this};
  UTIL::BitField64 *_cellid_encoder = new UTIL::BitField64(CEPCConf::DetEncoderString::getStringRepresentation());
  float weight = 1.0;

  // double bReso = _diffRPhi * _diffRPhi;
  const gear::TPCParameters &gearTPC = _GEAR->getTPCParameters();
  const gear::PadRowLayout2D &padLayout = gearTPC.getPadLayout();
  // gear::Vector2D padCoord;
  const edm4hep::SimTrackerHitCollection *simCol = nullptr;
  try
  {
    simCol = m_inputSimCol.get();
  }
  catch (GaudiException &e)
  {
    debug() << "Collection " << m_inputSimCol.fullKey() << " is unavailable in event " << endmsg;
    return StatusCode::SUCCESS;
  }

  const edm4hep::SimPrimaryIonizationClusterCollection *clusterCol = nullptr;
  try
  {
    clusterCol = m_inputIonClusterCol.get();
  }
  catch (GaudiException &e)
  {
    debug() << "Collection " << m_inputIonClusterCol.fullKey() << " is unavailable in event " << endmsg;
    return StatusCode::SUCCESS;
  }
  std::map<int, edm4hep::SimTrackerHit> simhitMap;                      // A map to save the sim tracker hit, key is TPC layer, value is 3D position of simhit
  std::map<int, std::tuple<double, double, double>> digihitMap; // A map to save the XYZ resolutions and  Z position(key is pad index, values are z and rphi resolutions and Z position)
  std::map<int, int> tpcPixelQ;                                         // A map to save number of electrons in same pixel (key is pixel index, value is number of eles)
  std::map<int, int> tpcPixelFlag;                                      // A map to mark if a pixel already has a hit (key is pixel index), only used in merging case
  std::map<int, std::vector<int>> tpcPixelQMap;
  std::map<int, std::vector<std::tuple<float, float>>> TimeZMap;        // A map to save time and z direction infos
  
  for (auto sim : *simCol)
  { // loop all sim tracker hits, save them in a map, key is TPC layer, value is 3D position of simhit
    CLHEP::Hep3Vector SimPoint(sim.getPosition()[0], sim.getPosition()[1], sim.getPosition()[2]);
    int simPadIndex = padLayout.getNearestPad(SimPoint.perp(), SimPoint.phi());
    int iRowSimHit = padLayout.getRowNumber(simPadIndex);
    simhitMap[iRowSimHit] = sim;
  }

  int nclu = 0;


  int padIndex;
  int iRowHit;
  int poylaRandom;
  double x_amp_diffusion;
  double y_amp_diffusion;
  CLHEP::Hep3Vector point;
  CLHEP::Hep3Vector thisPoint;
  gear::Vector2D padCoord;
  edm4hep::Vector3d posAfterSmear;
  edm4hep::Vector3d pos;

  int nIonEle;
  debug() << " Total clusters: " << clusterCol->size() << endmsg;
  // for (int i=0; i < _nPads;++i){
  // padCoord=padLayout.getPadCenter(int(i));
  // std::cout<<" check padxy "<<padCoord[0] * cos(padCoord[1])<<" "<<padCoord[0] * sin(padCoord[1])<<std::endl;
  // std::cout<<" check padrphi "<<padCoord[0] <<" "<<padCoord[1] <<std::endl;
  // }
  

  for (edm4hep::SimPrimaryIonizationCluster cluster : *clusterCol)
  { // Loop all clusters
    // break;
    debug() << " start cluster: " << nclu << " pos:" << cluster.getPosition() << " size: " << cluster.electronPosition_size() << endmsg;
    nclu += 1;
    nIonEle = 0;

    for (std::size_t i = 0; i < cluster.electronPosition_size(); ++i)
    { // loop all electrons in each clusters

      pos = *(cluster.electronPosition_begin() + i);
      float eleTime = *(cluster.electronTime_begin() + i) / 1000.;//time in us

      if (_MaxIonEles > 0 && nIonEle > _MaxIonEles)
      {
        debug() << "Maximum eles achived, break the loop" << endmsg;
        break;
      }
	  driftLength = gearTPC.getMaxDriftLength() - (fabs(pos.z));
      tpcTRes = (t_diffusion_p0 +t_diffusion_p1 * sqrt(driftLength / 10.)); 

      tpcXRes = x_diffusion_p0 + x_diffusion_p1 * sqrt(driftLength);
      tpcYRes = y_diffusion_p0 + y_diffusion_p1 * sqrt(driftLength);
      double randx = gRandom->Gaus(pos.x, tpcXRes);
      double randy = gRandom->Gaus(pos.y, tpcYRes);
      // double randz = gRandom->Gaus(pos.z, tpcZRes);
      double randt = eleTime + gRandom->Gaus(driftLength / 80, tpcTRes); // unit in us
      double randz = randt*80;
      //   
      point.set(randx, randy, randz);

      if (fabs(point.z()) > gearTPC.getMaxDriftLength())
        point.setZ((fabs(point.z()) / point.z()) * gearTPC.getMaxDriftLength());
      debug() << "resolution, x: " << tpcXRes << " y: " << tpcYRes << " z: " << tpcZRes << endmsg;
      debug() << "Max TPC length: " << gearTPC.getMaxDriftLength() << " posz: " << fabs(pos.z) << " driftLength: " << driftLength << endmsg;
      debug() << "smear:" << pos.x << "," << pos.y << "," << pos.z << " -> " << point << endmsg;
      poylaRandom = _f1->GetRandom();
      // poylaRandom = 1;
      debug() << "polyRandom: " << poylaRandom << endmsg;
      for (int j = 0; j < poylaRandom; ++j) // Do amplication
      {

        x_amp_diffusion = gRandom->Gaus(point.x(), _ampDiffusion);
        y_amp_diffusion = gRandom->Gaus(point.y(), _ampDiffusion);
        thisPoint.set(x_amp_diffusion, y_amp_diffusion, point.z());

        // following lines are the method to find the pixel center
        padIndex = padLayout.getNearestPad(thisPoint.perp(), thisPoint.phi());
        padCoord = padLayout.getPadCenter(int(padIndex));
        posAfterSmear.x = padCoord[0] * cos(padCoord[1]);
        posAfterSmear.y = padCoord[0] * sin(padCoord[1]);
        posAfterSmear.z = point.z();

        digihitMap[padIndex] = std::make_tuple(tpcXRes, tpcYRes, tpcZRes); 
        if (tpcPixelQ.find(padIndex) != tpcPixelQ.end())
        {
          tpcPixelQ[padIndex] += 1;
        }
        else
        {
          tpcPixelQ[padIndex] = 1;
        }
        if (TimeZMap[padIndex].empty())
        {
          TimeZMap[padIndex].push_back(std::make_tuple(randt,point.z()));
        }
        else
        {
          if (!_saveAllTime)
          {

            if (randt > std::get<0>(TimeZMap[padIndex].back()) + _deadTime)
            {
              info() << "now the size of pad " << padIndex << " is " << TimeZMap[padIndex].size() << " Q is  " << tpcPixelQ[padIndex] << endmsg;
              info() << "now time is: " << randt << " previous time is: " << std::get<0>(TimeZMap[padIndex].back()) << endmsg;

              TimeZMap[padIndex].push_back(std::make_tuple(randt,point.z()));
              tpcPixelQMap[padIndex].push_back(tpcPixelQ[padIndex]);
              tpcPixelQ[padIndex]=0;
            }
          }
          else
          {
            auto it = TimeZMap[padIndex].rbegin();
            bool found = false;
            for (; it != TimeZMap[padIndex].rend(); ++it)
            {
              float diff = std::abs(std::get<0>(*it) - randt);
              if (diff < _minimumTime)
              {
                found = true;
                break;
              }
            }

            if (!found)
            {
              TimeZMap[padIndex].emplace_back(randt,point.z());
              tpcPixelQMap[padIndex].push_back(tpcPixelQ[padIndex]);
              tpcPixelQ[padIndex]=0;
            }
          }
        }

      }
      nIonEle += 1;
    }
  }
  debug() << "create " << digihitMap.size() << " hits in different pixels before merging" << endmsg;
  for (auto digiHit = digihitMap.begin(); digiHit != digihitMap.end(); ++digiHit)
    {
      padIndex = digiHit->first;
      std::vector<std::tuple<float, float>> TimeZVec = TimeZMap[padIndex];
      tpcPixelQMap[padIndex].push_back(tpcPixelQ[padIndex]);
      std::vector<int> QVec = tpcPixelQMap[padIndex];
      
      tpcXRes = std::get<0>(digiHit->second);
      tpcYRes = std::get<1>(digiHit->second);
      tpcZRes = std::get<2>(digiHit->second);
      padCoord = padLayout.getPadCenter(int(digiHit->first));

      double r = padCoord[0];
      double phi = padCoord[1];
      padIndex = digiHit->first;
      iRowHit = padLayout.getRowNumber(padIndex);
      debug() << "pad center: " << r * cos(phi) << " " << r * sin(phi) << endmsg;
      debug() << "pad r: " << r << " phi: " << phi << " Q: " << tpcPixelQ[padIndex] << endmsg;
      posAfterSmear.x = (r * cos(phi));
      posAfterSmear.y = (r * sin(phi));
      
      for (size_t i = 0; i < TimeZVec.size(); ++i) {
        posAfterSmear.z = std::get<1>(TimeZVec[i]);
        auto it = simhitMap.find(iRowHit);
        if (it != simhitMap.end() && !_skipAss)
      {
        
        simhit = it->second;
        auto outhit = hitCol->create();
        auto ass = assCol->create();
        outhit.setPosition(posAfterSmear);
        (*_cellid_encoder)[lcio::ILDCellID0::subdet] = lcio::ILDDetID::TPC;
        (*_cellid_encoder)[lcio::ILDCellID0::layer] = iRowHit;
        (*_cellid_encoder)[lcio::ILDCellID0::module] = 0;

        (*_cellid_encoder)[lcio::ILDCellID0::side] = lcio::ILDDetID::barrel;
        outhit.setCellID(_cellid_encoder->lowWord());
        std::array<float, TRKHITNCOVMATRIX> covMat = {float(tpcXRes * tpcXRes),
                                                      float(tpcYRes * tpcXRes),
                                                      float(tpcYRes * tpcYRes),
                                                      float(tpcZRes * tpcXRes),
                                                      float(tpcZRes * tpcYRes),
                                                      float(tpcZRes * tpcZRes)};

        outhit.setCovMatrix(covMat);
        outhit.setEDep(tpcPixelQMap[padIndex][i]);
        outhit.setTime(std::get<0>(TimeZVec[i]));
        info() << "out hit, x: " << outhit.getPosition()[0] << " y: " << outhit.getPosition()[1] << " z: " << outhit.getPosition()[2] << " Q: " << outhit.getEDep() << " Time: "<< outhit.getTime() << endmsg;
        ass.setSim(simhit);
        ass.setRec(outhit);
        ass.setWeight(weight);
      }
      else if (_skipAss)// do not match the TPCCollection and TPCTrackerHits, all the TPCTrackerHitsAss has been set to 0,0,0
      {
        debug() << " skip ass" << endmsg;
        debug() << " SimTrackerHit " << simhit.getPosition()[0] << endmsg;

        auto outhit = hitCol->create();
        auto ass = assCol->create();
        outhit.setPosition(posAfterSmear);
        (*_cellid_encoder)[lcio::ILDCellID0::subdet] = lcio::ILDDetID::TPC;
        (*_cellid_encoder)[lcio::ILDCellID0::layer] = iRowHit;
        (*_cellid_encoder)[lcio::ILDCellID0::module] = 0;

        (*_cellid_encoder)[lcio::ILDCellID0::side] = lcio::ILDDetID::barrel;
        outhit.setCellID(_cellid_encoder->lowWord());
        std::array<float, TRKHITNCOVMATRIX> covMat = {float(tpcXRes * tpcXRes),
                                                      float(tpcYRes * tpcXRes),
                                                      float(tpcYRes * tpcYRes),
                                                      float(tpcZRes * tpcXRes),
                                                      float(tpcZRes * tpcYRes),
                                                      float(tpcZRes * tpcZRes)};

        outhit.setCovMatrix(covMat);
        outhit.setEDep(tpcPixelQMap[padIndex][i]);
        outhit.setTime(std::get<0>(TimeZVec[i]));

        ass.setSim(simhit);
        ass.setRec(outhit);
        ass.setWeight(weight);
        info() << "out hit, x: " << outhit.getPosition()[0] << " y: " << outhit.getPosition()[1] << " z: " << outhit.getPosition()[2] << " Q: " << outhit.getEDep() << " Time: "<< outhit.getTime() << endmsg;
      }
      else
      {
        debug() << " no matches" << endmsg;
      }
      }
    }
  return StatusCode::SUCCESS;
}

StatusCode TPCPixelDigiTool::finalize()
{
  StatusCode sc;
  return sc;
}
