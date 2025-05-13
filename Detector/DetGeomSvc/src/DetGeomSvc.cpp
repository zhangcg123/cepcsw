#include "DetGeomSvc.h"
#include "DD4hep/Detector.h"
#include "DD4hep/Plugins.h"
#include "GaudiKernel/MsgStream.h"
#include "podio/FrameCategories.h"
#include <DD4hep/Segmentations.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DDRec/DetectorData.h>
#include <string>

DECLARE_COMPONENT(DetGeomSvc)

DetGeomSvc::DetGeomSvc(const std::string &name, ISvcLocator *svc)
    : base_class(name, svc), m_dd4hep_geo(nullptr) {}

DetGeomSvc::~DetGeomSvc() {}

void DetGeomSvc::initDetIdToNames() {
  if (m_enable_fastinit.value()) {
    // fastinit enabled, no dd4hep geo initlized, read from metadata
    auto _detIdVec = m_metadata->getValue<std::vector<int>>("DetIDVector");
    auto _detNameVec =
        m_metadata->getValue<std::vector<std::string>>("DetNameVector");
    // Convert vectors to map (detID -> detName)
    for (size_t i = 0; i < _detIdVec.size() && i < _detNameVec.size(); ++i) {
      m_detIdToNames[_detIdVec[i]] = _detNameVec[i];
    }
  } else {
    auto world = getDD4HepGeo();
    auto subs = world.children();
    for (auto sub : subs) {
      int detId = sub.second.id();
      if (detId != -1)
        m_detIdToNames[detId] = sub.first;
      info() << sub.second.id() << " " << sub.first << endmsg;
    }
  }
}

void DetGeomSvc::initReadoutNameToDecoder() {
  if (m_enable_fastinit.value()) {
    // fastinit enabled, no dd4hep geo initlized, read from metadata
    auto _readoutNameVec =
        m_metadata->getValue<std::vector<std::string>>("ReadoutNameVector");
    auto _decoderVec = m_metadata->getValue<std::vector<std::string>>(
        "CellIDDecoderStringVector");
    // Convert vectors to map (detID -> detName)
    for (size_t i = 0; i < _readoutNameVec.size() && i < _decoderVec.size();
         ++i) {
      m_readoutNameToDecoder[_readoutNameVec[i]] = _decoderVec[i];
    }
  }
}

StatusCode DetGeomSvc::initialize() {
  StatusCode sc = Service::initialize();

  m_dd4hep_geo = &(dd4hep::Detector::getInstance());
  // if DEBUG, set
  // dd4hep::PrintLevel level = msgLevel(MSG::INFO) ? dd4hep::printLevel() :
  // dd4hep::setPrintLevel(dd4hep::DEBUG);
  // if failed to load the compact, a runtime error will be thrown.

  // load the compact file
  m_dd4hep_geo->fromCompact(m_dd4hep_xmls.value());
  // recover to old level, if not, too many DD4hep print
  // dd4hep::setPrintLevel(level);
  if (m_enable_fastinit.value()) {
    podio::ROOTFrameReader m_reader;
    m_reader.openFile(m_metadata_path.value());
    auto frame = podio::Frame(m_reader.readEntry(podio::Category::Metadata, 0));
    m_metadata =
        std::make_unique<podio::GenericParameters>(frame.getParameters());
  }
  return sc;
}

StatusCode DetGeomSvc::finalize() {
  StatusCode sc;

  // m_surface_manager has added as extension of Detector, so not delete?
  // if (m_surface_manager != nullptr) delete m_surface_manager;
  // if(m_dd4hep_geo) {
  //   dd4hep::Detector::destroyInstance();
  // }
  // m_volumeManager.destroy();

  return sc;
}

dd4hep::DetElement DetGeomSvc::getDD4HepGeo() {
  if (lcdd()) {
    return lcdd()->world();
  }
  return dd4hep::DetElement();
}

dd4hep::Detector *DetGeomSvc::lcdd() { return m_dd4hep_geo; }

IGeomSvc::Decoder *DetGeomSvc::getDecoder(const std::string &readout_name) {
  IGeomSvc::Decoder *decoder = nullptr;

  if (m_enable_fastinit.value()) {
    if (m_readoutNameToDecoder.empty()) {
      initReadoutNameToDecoder();
    }
    auto it = m_readoutNameToDecoder.find(readout_name);
    if (it != m_readoutNameToDecoder.end()) {
      decoder = new IGeomSvc::Decoder(it->second);
    }
  } else {
    if (!lcdd()) {
      error() << "Failed to get lcdd()" << endmsg;
      return decoder;
    }

    auto readouts = m_dd4hep_geo->readouts();
    if (readouts.find(readout_name) == readouts.end()) {
      error() << "Failed to find readout name '" << readout_name << "'"
              << " in DD4hep::readouts. " << endmsg;
      return decoder;
    }

    dd4hep::Readout readout = lcdd()->readout(readout_name);
    auto m_idspec = readout.idSpec();

    decoder = m_idspec.decoder();
  }

  if (!decoder) {
    error() << "Failed to get the decoder with readout '" << readout_name << "'"
            << endmsg;
  }

  return decoder;
}

const dd4hep::rec::SurfaceMap *
DetGeomSvc::getSurfaceMap(const std::string &det_name) {
  if (!m_dd4hep_geo) {
    return nullptr;
  }
  if (m_surface_manager == nullptr) {
    dd4hep::rec::SurfaceManager *surfaceMgr = nullptr;
    // first check whether exist
    try {
      surfaceMgr = m_dd4hep_geo->extension<dd4hep::rec::SurfaceManager>();
    } catch (std::runtime_error &e) {
      info() << e.what() << " " << surfaceMgr << endmsg;
      surfaceMgr = nullptr;
    }

    if (surfaceMgr) {
      m_surface_manager = surfaceMgr;
    } else {
      m_dd4hep_geo->addExtension<dd4hep::rec::SurfaceManager>(
          m_surface_manager = new dd4hep::rec::SurfaceManager(*m_dd4hep_geo));
    }
  }
  return m_surface_manager->map(det_name);
}

std::string DetGeomSvc::getDetName(const int det_id) {
  if (m_detIdToNames.empty()) {
    initDetIdToNames();
  }
  auto it = m_detIdToNames.find(det_id);
  if (it != m_detIdToNames.end()) {
    return it->second;
  } else {
    return lcdd()->world().name();
  }
}

double DetGeomSvc::getEcalBarLength(unsigned long cellId) {
  if (m_enable_fastinit.value()) {
    if (m_ecalCellIdToBarLengthMap.empty()) {
      // Lazy init
      _ecal_barrel_decoder = getDecoder("EcalBarrelCollection");
      _ecal_endcap_decoder = getDecoder("EcalEndcapsCollection");
      auto cellIDs = m_metadata->getValue<std::vector<int>>("EcalCellIDVector");
      auto barLengths =
          m_metadata->getValue<std::vector<double>>("EcalBarLengthVector");
      for (size_t i = 0; i < cellIDs.size(); ++i) {
        m_ecalCellIdToBarLengthMap[cellIDs[i]] = barLengths[i];
      }
    }
    dd4hep::rec::CellID _cellId = 0;
    auto system = _ecal_barrel_decoder->get(cellId, "system");

    // Configure decoder based on system type
    auto decoder = (system == 20) ? _ecal_barrel_decoder : _ecal_endcap_decoder;
    decoder->set(_cellId, "system", system);
    decoder->set(_cellId, "bar", 0);
    decoder->set(_cellId, "dlayer", decoder->get(cellId, "dlayer"));
    decoder->set(_cellId, "slayer", decoder->get(cellId, "slayer"));

    if (system == 20) {
      decoder->set(_cellId, "stave", 0);
      decoder->set(_cellId, "module", decoder->get(cellId, "module") % 2);
    } else if (system == 29) {
      decoder->set(_cellId, "module", decoder->get(cellId, "module"));
      decoder->set(_cellId, "part", decoder->get(cellId, "part"));
      decoder->set(_cellId, "stave", decoder->get(cellId, "stave"));
      decoder->set(_cellId, "type", decoder->get(cellId, "type"));
    }

    // Lookup and return result
    auto it = m_ecalCellIdToBarLengthMap.find(_cellId);
    if (it != m_ecalCellIdToBarLengthMap.end()) {
      return it->second;
    }

    // Debug output for missing cell ID
    fatal() << "GetEcalBarLength: Cell ID not found: " << _cellId
            << " (system: " << system << ")" << endmsg;
    throw GaudiException("GetEcalBarLength: Cell ID not found", name(), StatusCode::FAILURE);
  } else {
    if (!m_volumeManager) {
      m_volumeManager = new dd4hep::VolumeManager(
          dd4hep::VolumeManager::getVolumeManager(*m_dd4hep_geo));
    }
    // use dd4hep volume
    dd4hep::PlacedVolume ipv = m_volumeManager->lookupVolumePlacement(cellId);
    dd4hep::Volume ivol = ipv.volume();
    std::vector<double> iVolParam = ivol.solid().dimensions();
    auto maxElement = std::max_element(iVolParam.begin(), iVolParam.end());
    iVolParam.clear();
    return *maxElement * 20;
  }
}