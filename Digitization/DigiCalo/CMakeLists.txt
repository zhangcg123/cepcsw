# Modules
gaudi_add_module(CaloDigi
                 SOURCES src/EcalDigiAlg.cpp
                 SOURCES src/HcalDigiAlg.cpp
                 LINK k4FWCore::k4FWCore
                      GearSvc
                      DetInterface
                      Gaudi::GaudiKernel
                      Gaudi::GaudiAlgLib 
                      ${CLHEP_LIBRARIES}
                      ${GEAR_LIBRARIES} 
                      ${GSL_LIBRARIES} 
                      ${LCIO_LIBRARIES}
                      ${ROOT_LIBRARIES}
                      EDM4HEP::edm4hep EDM4HEP::edm4hepDict
)
install(TARGETS CaloDigi
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)

