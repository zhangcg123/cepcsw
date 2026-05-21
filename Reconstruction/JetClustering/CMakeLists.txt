# Set the CMake module path if necessary

set(FASTJET_ROOT "/cvmfs/sft.cern.ch/lcg/views/LCG_103/x86_64-centos7-gcc11-opt/")
set(INCLUDE_DIRS "${FASTJET_ROOT}/include")
set(LIBRARY_DIRS "${FASTJET_ROOT}/lib")
find_library(FASTJET_LIB fastjet PATHS ${LIBRARY_DIRS})

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
                      ${FASTJET_LIB}
                      EDM4HEP::edm4hep EDM4HEP::edm4hepDict
)
install(TARGETS JetClustering
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
