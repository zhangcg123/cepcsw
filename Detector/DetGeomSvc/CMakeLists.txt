
##############################################################################
# Package: GeomSvc
#    Desc: implement the IGeomSvc interface.
##############################################################################

gaudi_add_module(DetGeomSvc
                 SOURCES src/DetGeomSvc.cpp
                 LINK
                   DetInterface
                   ${DD4hep_COMPONENT_LIBRARIES} 
                   Gaudi::GaudiKernel
                   ${GEAR_LIBRARIES}
                   ${ROOT_LIBRARIES}
)

target_include_directories(DetGeomSvc
  PUBLIC ${GEAR_INCLUDE_DIRS}
)

install(TARGETS DetGeomSvc
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
