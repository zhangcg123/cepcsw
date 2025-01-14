#ifndef DetGeomSvc_h
#define DetGeomSvc_h

// Interface
#include "DetInterface/IGeomSvc.h"

// Gaudi
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/ServiceHandle.h"

// DD4Hep
#include "DD4hep/Detector.h"

#include <gear/GEAR.h>
#include <gearimpl/ZPlanarParametersImpl.h>
#include <gearimpl/GearParametersImpl.h>

class TGeoNode;

class DetGeomSvc: public extends<Service, IGeomSvc> {
 public:
  DetGeomSvc(const std::string& name, ISvcLocator* svc);
  ~DetGeomSvc();
  
  // Service
  StatusCode initialize() override;
  StatusCode finalize() override;
  
  // IGeomSvc
  dd4hep::DetElement getDD4HepGeo() override;
  dd4hep::Detector* lcdd() override;

 private:
  Decoder* getDecoder(const std::string& readout_name) override;
  const dd4hep::rec::SurfaceMap* getSurfaceMap(const std::string& det_name) override;
  std::string getDetName(const int det_id) override;
    
private:
  // DD4hep XML compact file path
  Gaudi::Property<std::string> m_dd4hep_xmls{this, "compact"};
  
  // 
  dd4hep::Detector* m_dd4hep_geo;
  dd4hep::rec::SurfaceManager* m_surface_manager = nullptr;

  std::map<int, std::string> m_detIdToNames;
};

#endif // GeomSvc_h
