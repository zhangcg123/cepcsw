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
  add_library(RecBreakpointLikelihood STATIC src/TrackLikelihood.cpp)
  target_compile_features(RecBreakpointLikelihood PUBLIC cxx_std_17)
  target_include_directories(RecBreakpointLikelihood PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
  target_link_libraries(RecBreakpointLikelihood PUBLIC Eigen3::Eigen ROOT::Matrix)
  set_target_properties(RecBreakpointLikelihood PROPERTIES POSITION_INDEPENDENT_CODE ON)
  gaudi_add_module(RecBreakpoint
    SOURCES src/RecBreakpoint.cpp src/TrackState.cpp src/KalmanAdapter.cpp
            src/RecBreakpointIdentification.cpp src/BaselineKFDiagnostics.cpp
            src/BreakpointFitter.cpp src/DiffuseLossState.cpp
            src/FreeLossFitter.cpp src/FreeLossTuple.cpp src/FitPairTuple.cpp
            src/CaloStateTuple.cpp src/RecoAssociationTuple.cpp
            src/PhotonCaloProjection.cpp src/PhotonCaloTuple.cpp
            src/NeutralLossCandidate.cpp src/NeutralEnergyMeasurement.cpp
            src/TruthLayerOwnership.cpp src/TruthBreakpointOwnership.cpp src/TruthBreakpointTuple.cpp
            src/LikelihoodTuple.cpp src/BeamSpotMeasurement.cpp
            # Reuse the exact maintained relation-driven reader, without
            # linking the GSF algorithm plugin or changing its implementation.
            ../RecGsfTracking/src/TruthBHLossEventData.cpp
    LINK RecBreakpointHelpers RecBreakpointLikelihood ROOT::Minuit2 TrackSystemSvcLib Gaudi::GaudiKernel
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
  if(TARGET RecBreakpointLikelihood)
    add_executable(RecBreakpointPhotonProjectionTest tests/PhotonProjectionTest.cpp
      src/PhotonCaloProjection.cpp)
    target_compile_features(RecBreakpointPhotonProjectionTest PRIVATE cxx_std_17)
    target_include_directories(RecBreakpointPhotonProjectionTest PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/Service/TrackSystemSvc/include
      ${KalTest_INCLUDE_DIRS})
    target_link_libraries(RecBreakpointPhotonProjectionTest PRIVATE EDM4HEP::edm4hep
      ${KalTest_LIBRARIES} ROOT::Matrix ROOT::Physics)
    add_test(NAME RecBreakpointPhotonProjection COMMAND RecBreakpointPhotonProjectionTest)
    add_executable(RecBreakpointTruthOwnershipTest tests/TruthOwnershipTest.cpp src/TruthLayerOwnership.cpp)
    target_compile_features(RecBreakpointTruthOwnershipTest PRIVATE cxx_std_17)
    target_include_directories(RecBreakpointTruthOwnershipTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
      ${CMAKE_CURRENT_SOURCE_DIR}/../RecGsfTracking/src)
    target_link_libraries(RecBreakpointTruthOwnershipTest PRIVATE GsfTruthEventData EDM4HEP::edm4hep)
    add_test(NAME RecBreakpointTruthOwnership COMMAND RecBreakpointTruthOwnershipTest)
    add_executable(RecBreakpointTruthMatchTest tests/TruthBreakpointMatchTest.cpp
      src/TruthLayerOwnership.cpp src/TruthBreakpointOwnership.cpp
      ../RecGsfTracking/src/TruthBHLossEventData.cpp)
    target_compile_features(RecBreakpointTruthMatchTest PRIVATE cxx_std_17)
    target_include_directories(RecBreakpointTruthMatchTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
      ${CMAKE_CURRENT_SOURCE_DIR}/../RecGsfTracking/src ${LCIO_INCLUDE_DIRS})
    target_link_libraries(RecBreakpointTruthMatchTest PRIVATE GsfTruthEventData EDM4HEP::edm4hep
      ${ROOT_LIBRARIES} ${LCIO_LIBRARIES})
    add_test(NAME RecBreakpointTruthMatch COMMAND RecBreakpointTruthMatchTest)
    add_executable(RecBreakpointNeutralLossCandidateTest tests/NeutralLossCandidateTest.cpp
      src/NeutralLossCandidate.cpp)
    target_compile_features(RecBreakpointNeutralLossCandidateTest PRIVATE cxx_std_17)
    target_include_directories(RecBreakpointNeutralLossCandidateTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(RecBreakpointNeutralLossCandidateTest PRIVATE EDM4HEP::edm4hep)
    add_test(NAME RecBreakpointNeutralLossCandidate COMMAND RecBreakpointNeutralLossCandidateTest)
    add_executable(RecBreakpointLikelihoodTest tests/LikelihoodTest.cpp)
    target_link_libraries(RecBreakpointLikelihoodTest PRIVATE RecBreakpointLikelihood)
    add_test(NAME RecBreakpointLikelihood COMMAND RecBreakpointLikelihoodTest)
    add_executable(RecBreakpointDiffuseTest tests/DiffuseTest.cpp
      src/DiffuseLossState.cpp src/TrackState.cpp)
    target_compile_features(RecBreakpointDiffuseTest PRIVATE cxx_std_17)
    target_include_directories(RecBreakpointDiffuseTest PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src
      ${PROJECT_SOURCE_DIR}/Service/TrackSystemSvc/include)
    target_link_libraries(RecBreakpointDiffuseTest PRIVATE ROOT::Matrix EDM4HEP::edm4hep)
    add_test(NAME RecBreakpointDiffuse COMMAND RecBreakpointDiffuseTest)
    add_executable(RecBreakpointNeutralEnergyTest tests/NeutralEnergyTest.cpp
      src/NeutralEnergyMeasurement.cpp src/TrackState.cpp)
    target_compile_features(RecBreakpointNeutralEnergyTest PRIVATE cxx_std_17)
    target_include_directories(RecBreakpointNeutralEnergyTest PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/Service/TrackSystemSvc/include
      ${KalTest_INCLUDE_DIRS})
    target_link_libraries(RecBreakpointNeutralEnergyTest PRIVATE RecBreakpointLikelihood EDM4HEP::edm4hep
      ${KalTest_LIBRARIES} ROOT::Physics)
    add_test(NAME RecBreakpointNeutralEnergy COMMAND RecBreakpointNeutralEnergyTest)
    add_executable(RecBreakpointBeamSpotTest tests/BeamSpotTest.cpp
      src/BeamSpotMeasurement.cpp src/TrackState.cpp)
    target_compile_features(RecBreakpointBeamSpotTest PRIVATE cxx_std_17)
    target_include_directories(RecBreakpointBeamSpotTest PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/Service/TrackSystemSvc/include
      ${KalTest_INCLUDE_DIRS})
    target_link_libraries(RecBreakpointBeamSpotTest PRIVATE RecBreakpointLikelihood EDM4HEP::edm4hep
      ${KalTest_LIBRARIES} ROOT::Physics)
    add_test(NAME RecBreakpointBeamSpot COMMAND RecBreakpointBeamSpotTest)
  endif()
endif()
