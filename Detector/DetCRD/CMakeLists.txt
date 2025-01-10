###############################################################################
# Package: DetCRD
#   CEPC Reference Detector (CRD)
################################################################################

gaudi_add_module(DetCRD
                 SOURCES src/Calorimeter/CRDEcal_v01.cpp
                         src/Calorimeter/LongCrystalBarBarrelCalorimeter32Polygon_v01.cpp
                         src/Calorimeter/LongCrystalBarEndcapCalorimeter_v01.cpp
                         src/Calorimeter/LongCrystalBarEndcapCalorimeter_v02.cpp
                         src/Calorimeter/LongCrystalBarEndcapCalorimeter_v03.cpp
                         src/Calorimeter/CRDEcal_Short_v02.cpp
                         src/Calorimeter/CRDEcal_Endcap_Short_v01.cpp
                         src/Calorimeter/RotatedPolyhedraBarrelCalorimeter_v01_geo.cpp
                         src/Calorimeter/RotatedCrystalCalorimeter_v01_geo.cpp
                         src/Calorimeter/Lumical_v01_geo.cpp
                         src/Other/Lumical_v01_geo_beampipe.cpp
                         src/Other/CRDBeamPipe_v01_geo.cpp
                         src/Muon/Muon_Barrel_v01_04.cpp
                         src/Muon/Muon_Endcap_v01_02.cpp
                         src/Tracker/SiTrackerSkewRing_v01_geo.cpp
                         src/Tracker/SiTrackerStitching_v01_geo.cpp
                         src/Tracker/SiTrackerStaggeredLadder_v01_geo.cpp
                         src/Tracker/SiTrackerStaggeredLadder_v02_geo.cpp
                         src/Tracker/SiTrackerComposite_v01_geo.cpp
                         src/Tracker/SiTrackerComposite_v02_geo.cpp
                         src/Tracker/TPC_Simple_o1_v01.cpp
                         src/Tracker/TPC_ModularEndcap_o1_v01.cpp
			 src/Tracker/SiTracker_itkbarrel_v01_geo.cpp
			 src/Tracker/SiTracker_itkbarrel_v02_geo.cpp
                         src/Tracker/SiTracker_otkbarrel_v01_geo.cpp
                         src/Tracker/SiTracker_otkendcap_v01_geo.cpp
                         src/Other/ParaffinEndcap_v01.cpp

		 LINK ${DD4hep_COMPONENT_LIBRARIES}
                      DetIdentifier
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

foreach(detoption TDR_o1_v01 TDR_o1_v02)
  add_test(
    NAME Test_${detoption}_Sim
    COMMAND gaudirun.py Detector/DetCRD/scripts/${detoption}/sim.py
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )

  add_test(
    NAME Test_${detoption}_Rec
    COMMAND gaudirun.py Detector/DetCRD/scripts/${detoption}/tracking.py
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )

  add_test(
    NAME Test_${detoption}_DD4hep2TGeo
    COMMAND geoConverter -compact2tgeo 
                         -input Detector/DetCRD/compact/${detoption}/${detoption}.xml 
                         -output ${detoption}.tgeo.root
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
endforeach()

add_test(
  NAME Test_TDR_o1_v01_CaloDigi
  COMMAND gaudirun.py Detector/DetCRD/scripts/TDR_o1_v01/calodigi.py
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  )

add_test(
  NAME Test_TDR_o1_v01_PFA
  COMMAND gaudirun.py Detector/DetCRD/scripts/TDR_o1_v01/rec.py
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  )

foreach(detoption TDR_o1_v01)
  add_test(
    NAME Test_${detoption}_Geo
    COMMAND checkOverlaps -c Detector/DetCRD/compact/${detoption}/${detoption}.xml -t 0.001
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
endforeach()
