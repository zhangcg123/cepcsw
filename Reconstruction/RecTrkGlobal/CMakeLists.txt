gaudi_add_header_only_library(TrackingLib)

# Modules
gaudi_add_module(Tracking
                 SOURCES src/Clupatra/ClupatraAlg.cpp
                         src/Clupatra/clupatra_new.cpp
                         src/FullLDCTracking/FullLDCTrackingAlg.cpp
                         src/TruthTracker/TruthTrackerAlg.cpp
                         src/FitterTool/KalTestTool.cpp
                 LINK EventSeeder
                      TrackSystemSvcLib
                      DetSegmentation
                      Gaudi::GaudiKernel
                      ${GSL_LIBRARIES}
                      ${LCIO_LIBRARIES}
                      EDM4HEP::edm4hep EDM4HEP::edm4hepDict
                      k4FWCore::k4FWCore
)

target_include_directories(Tracking PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if (CEPCSW_USE_GEAR)
  target_compile_definitions(Tracking PRIVATE CEPCSW_USE_GEAR)
  target_link_libraries(Tracking PRIVATE GearSvc ${GEAR_LIBRARIES})
endif()

install(TARGETS Tracking
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
