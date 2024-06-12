gaudi_add_module(TofRecAlg
                 SOURCES src/TofRecAlg.cpp
                 LINK DataHelperLib
                      Gaudi::GaudiKernel
                      EDM4HEP::edm4hep
                      ${ROOT_LIBRARIES}
                      ${CLHEP_LIBRARIES}
		                  ${LCIO_LIBRARIES}
		                  ${DD4hep_COMPONENT_LIBRARIES}
                      DetInterface
                      k4FWCore::k4FWCore
)

install(TARGETS TofRecAlg
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)