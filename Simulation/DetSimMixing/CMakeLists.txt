gaudi_add_module(DetSimMixing
                 SOURCES src/DetSimMixingAlg.cc
                 LINK DetSimInterface
                      EDM4HEP::edm4hep EDM4HEP::edm4hepDict
                      podio::podioRootIO
                      Gaudi::GaudiKernel
)

install(TARGETS DetSimMixing
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
