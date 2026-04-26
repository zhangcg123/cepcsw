# Modules
gaudi_add_module(MuonDigi
                 SOURCES src/MuonDigiAlg.cpp
                 LINK k4FWCore::k4FWCore
                      GearSvc
                      DetInterface
                      Gaudi::GaudiKernel
                      ${CLHEP_LIBRARIES}
                      ${GEAR_LIBRARIES} 
                      ${GSL_LIBRARIES} 
                      ${LCIO_LIBRARIES}
                      ${ROOT_LIBRARIES}
                      EDM4HEP::edm4hep EDM4HEP::edm4hepDict
)
install(TARGETS MuonDigi
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)

