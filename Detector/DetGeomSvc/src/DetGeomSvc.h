#ifndef DetGeomSvc_h
#define DetGeomSvc_h

// Interface
#include "DetInterface/IGeomSvc.h"

// Gaudi
#include "Gaudi/Property.h"
#include "GaudiKernel/Service.h"

// DD4Hep
#include "DD4hep/Detector.h"

#include <gear/GEAR.h>
#include <gearimpl/ZPlanarParametersImpl.h>
#include <gearimpl/GearParametersImpl.h>
#include <string>
#include <k4FWCore/MetaDataHandle.h>

// podio
#include <podio/podioVersion.h>

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
  double getEcalBarLength(unsigned long cellId) override;
  void initDetIdToNames();
  void initReadoutNameToDecoder();
    
private:
  // DD4hep XML compact file path
  Gaudi::Property<std::string> m_dd4hep_xmls{this, "compact"};
  // init without loading dd4hep geometry
  Gaudi::Property<bool> m_enable_fastinit{this, "fastinit", false};
  
  dd4hep::Detector* m_dd4hep_geo;
  dd4hep::rec::SurfaceManager* m_surface_manager = nullptr;
  dd4hep::VolumeManager* m_volumeManager = nullptr;

  std::unordered_map<int, std::string> m_detIdToNames;
  std::unordered_map<std::string, std::string> m_readoutNameToDecoder;
  std::unordered_map<int, double> m_ecalCellIdToBarLengthMap;
  
  Gaudi::Property<std::string> m_metadata_path{this, "metadata_path"};
  std::unique_ptr<podio::GenericParameters> m_metadata;

  // helper class to access metadata
  template <typename T>
  T get_metadata_value(const std::string& key) const {
      if (!m_metadata) {
          throw std::runtime_error("DetGeomSvc metadata has not been initialized");
      }   
#if podio_VERSION >= PODIO_VERSION(1, 0, 0)
      auto value = m_metadata->get<T>(key);
      if (!value) {
          throw std::runtime_error("Missing metadata key: " + key);
      }
      return *value;
#else
      return m_metadata->getValue<T>(key);
#endif
  }
  
  IGeomSvc::Decoder* _ecal_barrel_decoder;
  IGeomSvc::Decoder* _ecal_endcap_decoder;
};

#endif // GeomSvc_h
