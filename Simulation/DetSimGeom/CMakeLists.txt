
gaudi_add_module(DetSimGeom 
                 SOURCES src/WorldDetElemTool.cpp
                         src/DDG4DetElemTool.cpp
                 LINK DetSimInterface
                      DetInterface
                      ${DD4hep_COMPONENT_LIBRARIES} 
                      Gaudi::GaudiKernel 
                      ${Geant4_LIBRARIES}
)

install(TARGETS DetSimGeom
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)


