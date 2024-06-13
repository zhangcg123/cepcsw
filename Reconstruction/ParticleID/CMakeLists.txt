# gaudi_add_header_only_library(ParticleIDLib)

# Modules
gaudi_add_module(ParticleID
                 SOURCES src/TPCDndxAlg.cpp
                         src/DCHDndxAlg.cpp
			 src/TofRecAlg.cpp
                 LINK SimplePIDSvc
                      Gaudi::GaudiAlgLib
                      Gaudi::GaudiKernel
                      DataHelperLib
                      DetSegmentation
                      DetInterface
                      ${GSL_LIBRARIES}
                      ${GEAR_LIBRARIES}
                      ${LCIO_LIBRARIES}
                      EDM4HEP::edm4hep EDM4HEP::edm4hepDict
                      k4FWCore::k4FWCore
)

target_include_directories(ParticleID PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS ParticleID
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
