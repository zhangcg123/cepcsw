# gaudi_add_header_only_library(AnalysisPIDLib)

# Modules
gaudi_add_module(AnalysisPID
    SOURCES src/AnalysisPIDAlg.cpp
                 LINK Gaudi::GaudiKernel
                      DataHelperLib
                      DetSegmentation
                      DetInterface
                      ${GSL_LIBRARIES}
                      ${GEAR_LIBRARIES}
                      ${LCIO_LIBRARIES}
		      EDM4CEPC::edm4cepc EDM4CEPC::edm4cepcDict
                      EDM4HEP::edm4hep EDM4HEP::edm4hepDict
                      k4FWCore::k4FWCore
)

target_include_directories(AnalysisPID PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS AnalysisPID
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
