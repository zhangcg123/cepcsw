#ifndef SmearDigiTool_h
#define SmearDigiTool_h

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IRndmGenSvc.h"

#include "DigiTool/IDigiTool.h"
#include "DetInterface/IGeomSvc.h"

#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"

#include "DDRec/DetectorData.h"
#include "DDRec/SurfaceManager.h"

#include <vector>

class SmearDigiTool : public extends<AlgTool, IDigiTool> {
 public:
  using extends::extends;
  //SmearDigiTool(void* p) { m_pAlgUsing=p; };                                                                                                                         

  virtual StatusCode Call(const edm4hep::SimTrackerHitCollection* simCol, edm4hep::TrackerHitCollection* hitCol,
			  edm4hep::MCRecoTrackerAssociationCollection* assCol) override;
  virtual StatusCode Call(edm4hep::SimTrackerHit simhit, edm4hep::TrackerHitCollection* hitCol,
			  edm4hep::MCRecoTrackerAssociationCollection* assCol) override;

  StatusCode initialize() override;
  StatusCode finalize() override;

 private:
  Gaudi::Property<std::string> m_detName{this, "DetName", "VXD"};
  Gaudi::Property<std::string> m_readoutName{this, "Readout", "VXDCollection"};

  Gaudi::Property<float> m_eThreshold{this, "EnergyThreshold", 0};
  Gaudi::Property<float> m_efficiency{this, "Efficiency", 1};

  // resolution in direction of u - either one per layer or one for all layers
  Gaudi::Property<std::vector<float> > m_resU{this, "ResolutionU", {0.004}};
  // resolution in direction of v - either one per layer or one for all layers
  Gaudi::Property<std::vector<float> > m_resV{this, "ResolutionV", {0.004}};
  // resolution of time - either one per layer or one for all layers, ps as unit
  Gaudi::Property<std::vector<float> > m_resT{this, "ResolutionT", {0.0}};
  // whether hits are 1D strip hits
  Gaudi::Property<bool>  m_isStrip{this, "IsStrip", false};
  // whether use Planar tag for type and cov, if true, CEPCConf::TrkHitTypeBit::PLANAR bit is set as true
  // cov[0]=thetaU, cov[1]=phiU, cov[2]=resU, cov[0]=thetaV, cov[1]=phiV, cov[2]=resV
  Gaudi::Property<bool>  m_usePlanarTag{this, "UsePlanarTag", true};
  Gaudi::Property<float> m_maxPull{this, "PullCutToRetry", 1000.};
  Gaudi::Property<bool>  m_parameterize{this, "ParameterizeResolution", false};
  Gaudi::Property<std::vector<float> > m_parU{this, "ParametersU", {0}};
  Gaudi::Property<std::vector<float> > m_parV{this, "ParametersV", {0}};

  SmartIF<IRndmGenSvc> m_randSvc;
  SmartIF<IGeomSvc> m_geosvc;
  dd4hep::DDSegmentation::BitFieldCoder* m_decoder;
  const dd4hep::rec::SurfaceMap* m_surfaces;
  //void* m_pAlgUsing = nullptr;
};
#endif
