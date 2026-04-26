#include "DetGeomMetaWriter.h"
#include "DDRec/CellIDPositionConverter.h"
#include "DDSegmentation/BitFieldCoder.h"
#include "DetInterface/IGeomSvc.h"
#include "GaudiKernel/MsgStream.h"
#include <string>
#include <vector>

DECLARE_COMPONENT(GeomMetaWriter)

GeomMetaWriter::GeomMetaWriter(const std::string &aName, ISvcLocator *aSvcLoc)
    : Algorithm(aName, aSvcLoc) {}

GeomMetaWriter::~GeomMetaWriter() {}

void GeomMetaWriter::addDetIDNameMapping() {
  auto dets = m_geomSvc->getDD4HepGeo().children();
  for (auto det : dets) {
    int detId = det.second.id();
    if (detId != -1) {
      _detIDVec.push_back(detId);
      _detNameVec.push_back(det.first);
    }
  }
  m_detIdMetaDataHandle.put(_detIDVec);
  m_detNameMetaDataHandle.put(_detNameVec);
}

void GeomMetaWriter::addReadoutDecoderMapping() {

  auto readouts = m_geomSvc->lcdd()->readouts();
  for (auto readout : readouts) {
    auto readoutName = readout.first;
    auto readoutIdSpec = m_geomSvc->lcdd()->readout(readoutName).idSpec();
    auto decoderStr = readoutIdSpec.decoder()->fieldDescription();

    _readoutNameVec.push_back(readoutName);
    _decoderVec.push_back(decoderStr);
  }

  m_readoutNameMetaDataHandle.put(_readoutNameVec);
  m_decoderMetaDataHandle.put(_decoderVec);
}

void GeomMetaWriter::addEcalBarLengthMapping() {
  // cellid -> bar length
  // FIXME: HardCoded
  // ECAL Barrel: system:5,module:5,stave:4,dlayer:5,slayer:6,bar:15
  // system = 20
  // module = 0/1 (odd=1/even=0)
  // stave = 0
  // dlayer = 1-9
  // slayer = 0/1
  // bar = 0
  dd4hep::rec::CellID _cellId;
  auto _barrel_decoder = m_geomSvc->getDecoder("EcalBarrelCollection");
  for (int module = 0; module <= 1; module++) {
    for (int dlayer = 1; dlayer <= 9; dlayer++) {
      for (int slayer = 0; slayer <= 1; slayer++) {
        _barrel_decoder->set(_cellId, "system", 20);
        _barrel_decoder->set(_cellId, "stave", 0);
        _barrel_decoder->set(_cellId, "module", module);
        _barrel_decoder->set(_cellId, "dlayer", dlayer);
        _barrel_decoder->set(_cellId, "slayer", slayer);
        _barrel_decoder->set(_cellId, "bar", 0);
        debug() << "_cellId: " << _cellId << endmsg;
        auto _length = m_geomSvc->getEcalBarLength(_cellId);
        auto low32 = dd4hep::DDSegmentation::BitFieldCoder::lowWord(_cellId);
        _ecalCellIdVec.push_back(static_cast<int>(low32));
        _ecalBarLengthVec.push_back(_length);
      }
    }
  }

  // ECAL Endcap:
  // system:5,module:1,part:7,stave:7,type:4,dlayer:4,slayer:1,bar:7
  // system = 29
  // module = 0/1
  // part = 0-10
  // stave = 10
  // type = 0-16
  // dlayer = 0-8
  // slayer = 0-1
  // bar = 0

  auto _endcap_decoder = m_geomSvc->getDecoder("EcalEndcapsCollection");
  for (int module = 0; module <= 1; module++) {
    for (int part = 0; part <= 10; part++) {
      for (int stave = 0; stave <= 10; stave++) {
        for (int type = 0; type <= 20; type++) { // FIXME: find accurate upper limit
          for (int dlayer = 0; dlayer <= 9; dlayer++) {
            for (int slayer = 0; slayer <= 1; slayer++) {
              _endcap_decoder->set(_cellId, "system", 29);
              _endcap_decoder->set(_cellId, "module", module);
              _endcap_decoder->set(_cellId, "part", part);
              _endcap_decoder->set(_cellId, "stave", stave);
              _endcap_decoder->set(_cellId, "type", type);
              _endcap_decoder->set(_cellId, "dlayer", dlayer);
              _endcap_decoder->set(_cellId, "slayer", slayer);
              _endcap_decoder->set(_cellId, "bar", 0);
              try {
                auto _length = m_geomSvc->getEcalBarLength(_cellId);
                debug() << "_cellId: " << _cellId << endmsg;
                debug() << "OK! " << "module: " << module << "part: " << part
                        << " stave: " << stave << " dlayer: " << dlayer
                        << " slayer: " << slayer << endmsg;
                auto low32 =
                    dd4hep::DDSegmentation::BitFieldCoder::lowWord(_cellId);
                _ecalCellIdVec.push_back(static_cast<int>(low32));
                _ecalBarLengthVec.push_back(_length);
              } catch (const std::exception &e) {
                continue;
              }
            }
          }
        }
      }
    }
  }

  m_ecalCellIDMetaDataHandle.put(_ecalCellIdVec);
  m_ecalBarLengthMetaDataHandle.put(_ecalBarLengthVec);
}

StatusCode GeomMetaWriter::initialize() {
  if (Algorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  addDetIDNameMapping();
  addReadoutDecoderMapping();
  addEcalBarLengthMapping();
  return StatusCode::SUCCESS;
}

StatusCode GeomMetaWriter::execute() { return StatusCode::SUCCESS; }

StatusCode GeomMetaWriter::finalize() { return Algorithm::finalize(); }
