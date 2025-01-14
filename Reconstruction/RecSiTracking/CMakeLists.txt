# Modules
gaudi_add_module(SiliconTracking
                 SOURCES src/ForwardTrackingAlg.cpp
                         src/SiliconTrackingAlg.cpp
                         src/SpacePointBuilderAlg.cpp
                         src/TrackSubsetAlg.cpp
                 LINK EventSeeder
                      TrackingLib
                      TrackSystemSvcLib
                      DataHelperLib
                      KiTrackLib
                      Gaudi::GaudiKernel
                      k4FWCore::k4FWCore
                      ${GSL_LIBRARIES}
                      ${LCIO_LIBRARIES}
)

if (CEPCSW_USE_GEAR)
  target_compile_definitions(SiliconTracking PRIVATE CEPCSW_USE_GEAR)
  target_link_libraries(SiliconTracking PRIVATE GearSvc ${GEAR_LIBRARIES})
endif()

install(TARGETS SiliconTracking
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
