#include "TruthLayerOwnership.h"

#include <regex>

namespace breakpoint {
namespace {
const std::string beamPipeKey = "effective:beam-pipe-to-vxd0";
const std::string tpcInnerWallKey = "effective:tpc-inner-wall-to-row0";
}
int effectiveOwnerRule(const std::string& key) {
  return key == beamPipeKey ? 1 : key == tpcInnerWallKey ? 2 : 0;
}
MaterialOwner materialOwner(const std::string& path) {
  // shortcut: audited TDR_o1_v01 barrel only; extend from geometry evidence before endcap use.
  MaterialOwner result;
  std::smatch match;
  static const std::regex itk("(.*/ITKBarrel_layer([0-9]+)_LadderLog_[0-9]+)(/.*)?$");
  static const std::regex otk("(.*/OTKBarrel_stave_[0-9]+/(?:inner|outer)_OTKBarrel_ladder_[0-9]+/OTKBarrel_module_(?:inner|outer)_[0-9]+)(/.*)?$");
  static const std::regex bent("(.*/VXD_Layer([0-9]+)_Stave[0-9]+_[0-9]+)(/.*)?$");
  static const std::regex planar("(.*/VXD_LadderLogical_04_[0-9]+)/(.+)$");
  static const std::regex beamPipe(".*/BeamPipe_IPInnerTube_[0-9]+$");
  static const std::regex tpcInnerWall(".*/TPCinnerWallLog_[0-9]+(/TPCinnerWallLog_[0-9]+_[0-9]+)?$");
  if (std::regex_match(path, match, itk)) {
    result = {TruthLossOwner::Passive, match[1], 2, std::stoi(match[2]), {}};
  } else if (std::regex_match(path, match, otk)) {
    // OTK readout layer is 0 on BOTH inner/outer modules; retain the full key.
    result = {TruthLossOwner::Passive, match[1], 5, 0, {}};
  } else if (std::regex_match(path, match, bent)) {
    result = {TruthLossOwner::Passive, match[1], 1, std::stoi(match[2]), {}};
  } else if (std::regex_match(path, match, planar)) {
    const std::string part = match[2];
    int layer = -1;
    // Construction places top flex first, then bottom; sensor envelopes follow.
    if (part.find("VXD_FlexEnvelopeLogical_04_0/") == 0 ||
        part.find("VXD_SensorEnvelopeLogical_04_2/") == 0) layer = 5;
    if (part.find("VXD_FlexEnvelopeLogical_04_1/") == 0 ||
        part.find("VXD_SensorEnvelopeLogical_04_3/") == 0) layer = 4;
    if (layer >= 0)
      result = {TruthLossOwner::Passive, match[1].str()+"/sensor-side-"+std::to_string(layer), 1, layer, {}};
    else
      result = {TruthLossOwner::Shared, match[1], 1, -1, {4, 5}};
  } else if (path.find("/VXD_ShellLogical_") != std::string::npos) {
    result = {TruthLossOwner::Shared, path, 1, -1, {}};
  } else if (std::regex_match(path, beamPipe)) {
    result = {TruthLossOwner::Passive, beamPipeKey, 1, 0, {}};
  } else if (std::regex_match(path, tpcInnerWall)) {
    result = {TruthLossOwner::Passive, tpcInnerWallKey, 4, 0, {}};
  }
  return result;
}
}
