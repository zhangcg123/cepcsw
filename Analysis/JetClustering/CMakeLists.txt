# Set the CMake module path if necessary

# Modules
gaudi_add_module(JetClustering
                 SOURCES src/JetClustering.cpp
                 LINK GearSvc
                      Gaudi::GaudiKernel
                      k4FWCore::k4FWCore
                      DetInterface
                      DataHelperLib
                      ${GEAR_LIBRARIES} 
                      ${GSL_LIBRARIES} 
                      ${ROOT_LIBRARIES}
                      ${FASTJET_LIBRARY}
                      EDM4HEP::edm4hep EDM4HEP::edm4hepDict
)
install(TARGETS JetClustering
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
