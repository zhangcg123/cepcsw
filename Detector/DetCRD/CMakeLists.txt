###############################################################################
# Package: DetCRD
#   CEPC Reference Detector (CRD)
################################################################################

gaudi_add_module(DetCRD
                 SOURCES src/Calorimeter/CRDEcal_v01.cpp
                         src/Calorimeter/LongCrystalBarBarrelCalorimeter32Polygon_v01.cpp
                         src/Calorimeter/RotatedPolyhedraBarrelCalorimeter_v01_geo.cpp
                         src/Calorimeter/RotatedCrystalCalorimeter_v01_geo.cpp
                         src/Calorimeter/Lumical_v01_geo.cpp
                         src/Other/Lumical_v01_geo_beampipe.cpp
                         src/Other/CRDBeamPipe_v01_geo.cpp
                         src/Muon/Muon_Barrel_v01.cpp
                         src/Muon/Muon_Endcap_v01.cpp
                         src/Tracker/SiTrackerSkewRing_v01_geo.cpp
                         src/Tracker/SiTrackerStaggeredLadder_v01_geo.cpp
                         src/Tracker/TPC_Simple_o1_v01.cpp
                         src/Tracker/TPC_ModularEndcap_o1_v01.cpp

		 LINK ${DD4hep_COMPONENT_LIBRARIES}
)

set(LIBRARY_OUTPUT_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
message(STATUS "LIBRARY_OUTPUT_PATH -> ${LIBRARY_OUTPUT_PATH}")
dd4hep_generate_rootmap(DetCRD)

install(TARGETS DetCRD
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)

################################################################################
# Add tests
################################################################################

add_test(
  NAME Test_TDR_o1_v01_Sim
  COMMAND gaudirun.py Detector/DetCRD/scripts/TDR_o1_v01/sim.py
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_test(
  NAME Test_TDR_o1_v01_Rec
  COMMAND gaudirun.py Detector/DetCRD/scripts/TDR_o1_v01/tracking.py
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
