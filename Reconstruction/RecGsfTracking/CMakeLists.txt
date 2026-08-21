gaudi_add_module(RecGsfTracking
                 SOURCES
                        src/GsfComponent.cpp
                        src/GsfMixture.cpp
                        src/BetheHeitlerSplitter.cpp
                        src/TruthBHLossEventData.cpp
                        src/TruthBHLossTupleReader.cpp
                        src/GsfAlgorithm.cpp
                 LINK ${DDKalTest_LIBRARIES}
                      ${KalTest_LIBRARIES}
                      TrackSystemSvcLib
                      TrackingLib
                      Gaudi::GaudiKernel
                      k4FWCore::k4FWCore
                      EDM4HEP::edm4hep EDM4HEP::edm4hepDict
                      GsfTruthEventData GsfTruthEventDataDict
                      ${LCIO_LIBRARIES}
                      DataHelperLib
                      ${ROOT_LIBRARIES}
)

target_include_directories(RecGsfTracking PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/src
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS RecGsfTracking
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)

gaudi_add_module(RecGsfFlatTuple
                 SOURCES
                        src/GsfFlatTuple.cpp
                 LINK Gaudi::GaudiKernel
                      k4FWCore::k4FWCore
                      EDM4HEP::edm4hep EDM4HEP::edm4hepDict
                      ${ROOT_LIBRARIES}
)

target_include_directories(RecGsfFlatTuple PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/src
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS RecGsfFlatTuple
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)


gaudi_add_module(RecGsfSimHitTuple
                 SOURCES
                        src/GsfSimHitTuple.cpp
                 LINK Gaudi::GaudiKernel
                      k4FWCore::k4FWCore
                      EDM4HEP::edm4hep EDM4HEP::edm4hepDict
                      ${ROOT_LIBRARIES}
)

target_include_directories(RecGsfSimHitTuple PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/src
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS RecGsfSimHitTuple
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
