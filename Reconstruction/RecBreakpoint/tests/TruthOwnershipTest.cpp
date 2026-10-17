#include "TruthLayerOwnership.h"
#include <stdexcept>
#include <iostream>

void check(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
int main() {
  using breakpoint::materialOwner;
  const std::string itk = "/world/ITKBarrel_assembly_2/ITKBarrel_layer01_LadderLog_93";
  const auto tube = materialOwner(itk+"/ITKBarrel_layer01_SupportLog_0/ITKBarrel_layer01_SupportSliceLog_3");
  const auto sensor = materialOwner(itk+"/ITKBarrel_layer01_SensitiveLog_1/ITKBarrel_layer01_ModuleLog_8/ITKBarrel_layer01_SensorLog_2");
  check(tube.kind == TruthLossOwner::Passive && tube.layer == 1 && tube.key == sensor.key,
        "ITK cooling and sensor must share the layer-specific ladder");
  const std::string otk = "/world/OTKBarrel_envelope_5/OTKBarrel_stave_62/inner_OTKBarrel_ladder_2/OTKBarrel_module_inner_1";
  check(materialOwner(otk+"/OTKBarrel_modulecooling_tube_wall_inner_3").key ==
        materialOwner(otk+"/OTKBarrel_modulesensor_inner_6").key, "OTK cooling owner");
  check(materialOwner(otk).key != materialOwner("/world/OTKBarrel_envelope_5/OTKBarrel_stave_62/outer_OTKBarrel_ladder_2/OTKBarrel_module_outer_1").key,
        "OTK inner/outer must not collapse to their common layer=0");
  const std::string vxd = "/world/VXD_assembly_1/layer_04_9/VXD_LadderLogical_04_7/";
  const auto top = materialOwner(vxd+"VXD_FlexEnvelopeLogical_04_0/VXD_FlexLayerLogical_04index_06_6");
  const auto bottom = materialOwner(vxd+"VXD_FlexEnvelopeLogical_04_1/VXD_FlexLayerLogical_04index_06_6");
  check(top.layer == 5 && bottom.layer == 4 && top.key != bottom.key, "VXD side-specific flex");
  check(top.key == materialOwner(vxd+"VXD_SensorEnvelopeLogical_04_2/VXD_SensorLogical_05_27").key,
        "VXD top sensor and flex");
  check(bottom.key == materialOwner(vxd+"VXD_SensorEnvelopeLogical_04_3/VXD_SensorLogical_04_27").key,
        "VXD bottom sensor and flex");
  const auto shared = materialOwner(vxd+"VXD_SupLogical_04_4");
  check(shared.kind == TruthLossOwner::Shared && shared.candidates == std::vector<int>({4,5}),
        "VXD support must retain both possible layers");
  check(materialOwner("/world/VXD_assembly_1/VXD_ShellLogical_0").kind == TruthLossOwner::Shared, "VXD shell");
  const auto bent = materialOwner("/world/VXD_assembly_1/VXD_Layer03_Stave01_8/VXD_Layer03_Stave01Flex_1/VXD_Layer03_Stave01Flex_0_0");
  check(bent.kind == TruthLossOwner::Passive && bent.layer == 3, "bent VXD stave");
  for (const auto* path : {"/world", "/world/BeamPipe_IPInnerTube_2",
       "/world/TPC_envelope_4/TPCLog_0/TPCinnerWallLog_0", "/world/OTKBarrel_envelope_5",
       "/world/ITKEndcap_unknown_support", "/world/VXD_UnknownDesign"})
    check(materialOwner(path).kind == TruthLossOwner::Unassigned, "unknown/shared environment must not invent a layer");
  std::cout << "Truth ownership geometry rules passed\n";
}
