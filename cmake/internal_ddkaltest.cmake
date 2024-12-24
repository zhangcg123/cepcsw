include(ExternalProject)

if (CEPCSW_EXTERNAL_AS_COMPONENT)
  set(DDKalTest_DIR ${CMAKE_INSTALL_PREFIX})
else()
  set(DDKalTest_DIR ${CEPCSW_EXTERNAL_ROOT}/DDKalTest)
endif()

ExternalProject_Add(
  DDKalTest
  GIT_REPOSITORY https://code.ihep.ac.cn/cepc/externals/DDKalTest_iLCSoft.git
  GIT_TAG        v01-07-cepcsw
  PREFIX         ${CMAKE_BINARY_DIR}/_deps
  SOURCE_DIR     ${CMAKE_BINARY_DIR}/_deps/ddkaltest-src
  BINARY_DIR     ${CMAKE_BINARY_DIR}/_deps/ddkaltest-build
  STAMP_DIR      ${CMAKE_BINARY_DIR}/_deps/ddkaltest-stamp
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND ${CMAKE_COMMAND}
                -DCMAKE_INSTALL_PREFIX=${DDKalTest_DIR}
                -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${CMAKE_BINARY_DIR}/lib
                -DCONFIGURE_FILE_INSTALL_DIR=lib/cmake/DDKalTest
                -DKalTest_DIR=${KalTest_DIR}/lib/cmake/KalTest
                -DILCUTIL_DIR=${ILCUTIL_DIR}/lib/cmake/ILCUTIL
                -DaidaTT_DIR=${aidaTT_DIR}/lib/cmake/aidaTT
                 <SOURCE_DIR>
  BUILD_COMMAND ${CMAKE_COMMAND} --build . --target install
  DEPENDS ILCUTIL KalTest aidaTT
)

message(STATUS "will install into ${DDKalTest_DIR}")

set(DDKalTest_INCLUDE_DIRS ${DDKalTest_DIR}/include)
set(DDKalTest_LIBRARIES ${DDKalTest_DIR}/lib/libDDKalTest.so)
