###############################################################################
# Package: DetDriftChamber 
# Based on package: lcgeo
################################################################################

gaudi_add_module(DetDriftChamber
                 SOURCES src/driftchamber/DriftChamber.cpp
		 LINK DetSegmentation
                      ${DD4hep_COMPONENT_LIBRARIES} 
                  # ROOT Geant4
)

set(LIBRARY_OUTPUT_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
message(STATUS "LIBRARY_OUTPUT_PATH -> ${LIBRARY_OUTPUT_PATH}")
dd4hep_generate_rootmap(DetDriftChamber)

install(TARGETS DetDriftChamber
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
