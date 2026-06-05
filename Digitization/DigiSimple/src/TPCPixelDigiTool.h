#ifndef TPCPixelDigiTool_h
#define TPCPixelDigiTool_h
#include "CLHEP/Vector/TwoVector.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "TF1.h"
#include "DigiTool/IDigiTool.h"
#include "DetInterface/IGeomSvc.h"
#include "GearSvc/IGearSvc.h"
#include "edm4hep/EDM4hepVersion.h"
#include "edm4hep/SimTrackerHitCollection.h"

#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
#include "edm4hep/TrackerHit3DCollection.h"
#include "edm4hep/TrackerHitSimTrackerHitLinkCollection.h"
#else
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#include "edm4hep/SimPrimaryIonizationClusterCollection.h"
#include "edm4hep/SimPrimaryIonizationCluster.h"
#endif

#include <gear/GEAR.h>
#include "DDRec/DetectorData.h"
#include "DDRec/SurfaceManager.h"
#include "TRandom3.h"

#include <vector>

class TPCPixelDigiTool : public extends<AlgTool, IDigiTool> {
 public:
  using extends::extends;

  virtual StatusCode Call(const edm4hep::SimTrackerHitCollection* simCol, CEPCSWTrackerHit3DCollection* hitCol,
			  CEPCSWTrackerHitSimTrackerHitLinkCollection* assCol) override;
  virtual StatusCode Call(edm4hep::SimTrackerHit simhit, CEPCSWTrackerHit3DCollection* hitCol,
			  CEPCSWTrackerHitSimTrackerHitLinkCollection* assCol) override;

  StatusCode initialize() override;
  StatusCode finalize() override;
  // double getPadPhi( CLHEP::Hep3Vector* thisPointRPhi, CLHEP::Hep3Vector* firstPointRPhi, CLHEP::Hep3Vector* middlePointRPhi, CLHEP::Hep3Vector* lastPointRPhi);
  // double getPadTheta( CLHEP::Hep3Vector* firstPointRPhi, CLHEP::Hep3Vector* middlePointRPhi, CLHEP::Hep3Vector* lastPointRPhi );

 private:

  Gaudi::Property<std::string> m_detName{this, "DetName", "TPC"};
  Gaudi::Property<std::string> m_readoutName{this, "Readout", "TPCCollection"};
  

  Gaudi::Property<int> _magnetic{this, "magnetic", 3};// Magnet field 3 Tesla
  Gaudi::Property<double> _minimumTime{this, "minimumTime", 0.001};// if the time between two electrons less than this _minimumTime(us), then the two electrons will be treated as one 
  Gaudi::Property<double> _deadTime{this, "deadTime", 300};// 
  Gaudi::Property<int> _saveAllTime{this, "saveAllTime", 1};// 
  Gaudi::Property<double> _ampDiffusion{this, "ampDiffusion", 0.0003}; // Amplification factor
  Gaudi::Property<double> _MaxIonEles{this, "MaxIonEles",-1}; // Max allowed Inoized eles, negetive values mean no limits
  Gaudi::Property<std::vector<double> > _polyParas{this, "polyParas", {3802., 0.487, 2092}};//amplification polynominal paramters
  Gaudi::Property<double> _skipAss{this, "skipAss",1}; // Skip the mactching between digi hits and sim tracker hits




  TF1 *_f1;
  SmartIF<IRndmGenSvc> m_randSvc;
  SmartIF<IGeomSvc> m_geosvc;
  SmartIF<IGearSvc> _gear;
  gear::GearMgr* _GEAR;
  dd4hep::DDSegmentation::BitFieldCoder* m_decoder;
  const dd4hep::rec::SurfaceMap* m_surfaces;
  double x_diffusion_p0;
  double x_diffusion_p1;
  double y_diffusion_p0; 
  double y_diffusion_p1;
  double t_diffusion_p0; 
  double t_diffusion_p1;
  double tpcXRes;
  double tpcYRes;
  double tpcZRes;
  double tpcTRes;
  double driftLength;
  


};
#endif
