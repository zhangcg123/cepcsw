cmake_minimum_required(VERSION 3.15)

# Numerical helper tests also build standalone, without Gaudi or geometry.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(RecBreakpoint LANGUAGES CXX)
  include(CTest)
endif()

add_library(RecBreakpointHelpers STATIC src/AugmentedTransport.cpp)
target_compile_features(RecBreakpointHelpers PUBLIC cxx_std_17)
target_include_directories(RecBreakpointHelpers PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
set_target_properties(RecBreakpointHelpers PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  gaudi_add_module(RecBreakpoint
    SOURCES src/RecBreakpoint.cpp src/TrackState.cpp src/KalmanAdapter.cpp
            src/BreakpointFitter.cpp
            # Reuse the exact maintained relation-driven reader, without
            # linking the GSF algorithm plugin or changing its implementation.
            ../RecGsfTracking/src/TruthBHLossEventData.cpp
    LINK RecBreakpointHelpers TrackSystemSvcLib Gaudi::GaudiKernel
         k4FWCore::k4FWCore EDM4HEP::edm4hep EDM4HEP::edm4hepDict
         GsfTruthEventData GsfTruthEventDataDict
         ${ROOT_LIBRARIES} ${LCIO_LIBRARIES})
  target_include_directories(RecBreakpoint PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../RecGsfTracking/src
    ${PROJECT_SOURCE_DIR}/Service/TrackSystemSvc/src)
  install(TARGETS RecBreakpoint
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib)
  # Keep this plugin discoverable after a package-only build/install, without
  # requiring a rebuild of every module to merge the global Gaudi databases.
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/RecBreakpoint.components
    ${CMAKE_CURRENT_BINARY_DIR}/genConfDir/RecBreakpoint/RecBreakpoint.confdb
    DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

if(BUILD_TESTING)
  add_executable(RecBreakpointTransportTest tests/TransportTest.cpp)
  target_link_libraries(RecBreakpointTransportTest PRIVATE RecBreakpointHelpers)
  add_test(NAME RecBreakpointTransport COMMAND RecBreakpointTransportTest)
endif()
