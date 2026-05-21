find_package(Acts COMPONENTS
    Core PluginFpeMonitoring PluginGeant4 PluginJson
    PluginTGeo PluginDD4hep PluginEDM4hep Fatras) 

if(NOT Acts_FOUND)
    message("Acts package not found. RecActsTracking module requires Acts.")
    return()
endif()

gaudi_add_module(RecActsTracking
                 SOURCES
                        src/RecActsTracking.cpp
                 LINK DetInterface
                      k4FWCore::k4FWCore
                    Gaudi::GaudiKernel
                      ${LCIO_LIBRARIES} 
                      ${DD4hep_COMPONENT_LIBRARIES}
                      EDM4HEP::edm4hep EDM4HEP::edm4hepDict
                      ${podio_LIBRARIES} podio::podioRootIO
                      GearSvc ${GEAR_LIBRARIES}
                      ActsCore ActsPluginFpeMonitoring ActsPluginGeant4
                      ActsPluginJson ActsPluginTGeo  ActsPluginDD4hep
                      ActsPluginEDM4hep ActsFatras
)

target_include_directories(RecActsTracking PUBLIC $ENV{ACTS}/include)

target_include_directories(RecActsTracking PUBLIC
  ${LCIO_INCLUDE_DIRS}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS RecActsTracking
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
