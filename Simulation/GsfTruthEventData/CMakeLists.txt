set(GSF_TRUTH_EDM_GENERATED_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/generated")

PODIO_GENERATE_DATAMODEL(
  GsfTruthEventData
  "${CMAKE_CURRENT_SOURCE_DIR}/gsftruth.yaml"
  gsf_truth_edm_headers
  gsf_truth_edm_sources
  UPSTREAM_EDM "edm4hep:${EDM4HEP_DATA_DIR}/edm4hep.yaml"
  IO_BACKEND_HANDLERS ROOT
  OUTPUT_FOLDER "${GSF_TRUTH_EDM_GENERATED_DIR}"
)

PODIO_ADD_DATAMODEL_CORE_LIB(
  GsfTruthEventData
  "${gsf_truth_edm_headers}"
  "${gsf_truth_edm_sources}"
  OUTPUT_FOLDER "${GSF_TRUTH_EDM_GENERATED_DIR}"
)
target_link_libraries(GsfTruthEventData PUBLIC EDM4HEP::edm4hep)
target_include_directories(GsfTruthEventData PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
install(DIRECTORY include/GsfTruthEventData DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

PODIO_ADD_ROOT_IO_DICT(
  GsfTruthEventDataDict
  GsfTruthEventData
  "${gsf_truth_edm_headers}"
  "${GSF_TRUTH_EDM_GENERATED_DIR}/src/selection.xml"
  OUTPUT_FOLDER "${GSF_TRUTH_EDM_GENERATED_DIR}"
)

set(GSF_TRUTH_ROOTMAP
    "${CMAKE_CURRENT_BINARY_DIR}/GsfTruthEventDataDictDict.rootmap")
set(GSF_TRUTH_PCM
    "${CMAKE_CURRENT_BINARY_DIR}/libGsfTruthEventDataDict_rdict.pcm")

# ROOT discovers a generated data model only when its rootmap and PCM live next
# to the dictionary library. CEPCSW places libraries in the top-level build
# lib directory, while rootcling leaves these two files in this subdirectory.
add_custom_command(TARGET GsfTruthEventDataDict POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
          "${GSF_TRUTH_ROOTMAP}"
          "$<TARGET_FILE_DIR:GsfTruthEventDataDict>"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
          "${GSF_TRUTH_PCM}"
          "$<TARGET_FILE_DIR:GsfTruthEventDataDict>"
)

install(TARGETS GsfTruthEventData GsfTruthEventDataDict
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT dev
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/GsfTruthEventData" COMPONENT dev
)

install(DIRECTORY "${GSF_TRUTH_EDM_GENERATED_DIR}/GsfTruthEventData"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  COMPONENT dev
  FILES_MATCHING PATTERN "*.h"
)

install(FILES "${GSF_TRUTH_ROOTMAP}" "${GSF_TRUTH_PCM}"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  COMPONENT dev
)

if(BUILD_TESTING)
  add_executable(GsfTruthRecordsTest tests/TruthRecordsTest.cpp)
  target_include_directories(GsfTruthRecordsTest PRIVATE include)
  target_compile_features(GsfTruthRecordsTest PRIVATE cxx_std_17)
  target_compile_options(GsfTruthRecordsTest PRIVATE -UNDEBUG)
  add_test(NAME GsfTruthRecords COMMAND GsfTruthRecordsTest)
endif()
