###############################################################################
# Package: DetEcalMatrix 
# Based on package: lcgeo
################################################################################

gaudi_add_module(DetEcalMatrix
                 SOURCES src/calorimeter/EcalMatrix.cpp
		 LINK ${DD4hep_COMPONENT_LIBRARIES} 
                      # ROOT Geant4
)

set(LIBRARY_OUTPUT_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
message(STATUS "LIBRARY_OUTPUT_PATH -> ${LIBRARY_OUTPUT_PATH}")
dd4hep_generate_rootmap(DetEcalMatrix)

install(TARGETS DetEcalMatrix
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
