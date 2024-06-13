
gaudi_add_header_only_library(SimplePIDSvc)

gaudi_add_module(SimplePIDSvcPlugins
                 SOURCES src/SimplePIDSvc.cpp
                 LINK SimplePIDSvc
                      GearSvc
                      Gaudi::GaudiKernel
                      DataHelperLib
                      ${DD4hep_COMPONENT_LIBRARIES}
                      ${ROOT_LIBRARIES}
                      ${GEAR_LIBRARIES}
)

#target_include_directories(SimplePIDSvc PUBLIC
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
#  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS SimplePIDSvc SimplePIDSvcPlugins
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)

