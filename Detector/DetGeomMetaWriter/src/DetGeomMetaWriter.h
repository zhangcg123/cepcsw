#ifndef GeomMetaWriter_h
#define GeomMetaWriter_h

// GAUDI
#include "DetInterface/IGeomSvc.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/SmartIF.h"

// key4hep
#include <k4FWCore/DataHandle.h>
#include <k4FWCore/MetaDataHandle.h>

#include <string>

/** @class GeomMetaWriter
 *  Lightweight producer for edm data to test cellID
 */
class GeomMetaWriter : public Algorithm {
public:
  explicit GeomMetaWriter(const std::string &, ISvcLocator *);
  virtual ~GeomMetaWriter();
  /**  Initialize.
   *   @return status code
   */
  virtual StatusCode initialize() final;
  /**  Execute.
   *   @return status code
   */
  virtual StatusCode execute() final;
  /**  Finalize.
   *   @return status code
   */
  virtual StatusCode finalize() final;

private:
  void addDetIDNameMapping();
  void addReadoutDecoderMapping();
  void addEcalBarLengthMapping();

  MetaDataHandle<std::vector<int>> m_detIdMetaDataHandle{
      "DetIDVector", Gaudi::DataHandle::Writer};
  MetaDataHandle<std::vector<std::string>>
      m_detNameMetaDataHandle{"DetNameVector", Gaudi::DataHandle::Writer};
  
  MetaDataHandle<std::vector<std::string>>
      m_readoutNameMetaDataHandle{"ReadoutNameVector", Gaudi::DataHandle::Writer};
  MetaDataHandle<std::vector<std::string>>
      m_decoderMetaDataHandle{"CellIDDecoderStringVector", Gaudi::DataHandle::Writer};

  MetaDataHandle<std::vector<int>>
      m_ecalCellIDMetaDataHandle{"EcalCellIDVector", Gaudi::DataHandle::Writer};
  MetaDataHandle<std::vector<double>>
      m_ecalBarLengthMetaDataHandle{"EcalBarLengthVector", Gaudi::DataHandle::Writer};


  std::vector<int> _detIDVec;
  std::vector<std::string> _detNameVec;

  std::vector<std::string> _readoutNameVec;
  std::vector<std::string> _decoderVec;

  std::vector<int> _ecalCellIdVec;
  std::vector<double> _ecalBarLengthVec;

  SmartIF<IGeomSvc> m_geomSvc = service<IGeomSvc>("GeomSvc");
  dd4hep::VolumeManager m_volumeManager;
};
#endif
