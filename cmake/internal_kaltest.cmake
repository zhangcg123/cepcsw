include(ExternalProject)

OPTION( BUILD_WITH_T0_FIT "Set to ON to build with t0 fit (kSdim=6)" ON )

if (CEPCSW_EXTERNAL_AS_COMPONENT)
  set( KalTest_DIR ${CMAKE_INSTALL_PREFIX} )
else()
  set( KalTest_DIR ${CEPCSW_EXTERNAL_ROOT}/KalTest )
endif()

ExternalProject_Add(
  KalTest
  GIT_REPOSITORY https://code.ihep.ac.cn/cepc/externals/KalTest_iLCSoft.git
  GIT_TAG	 v02-05-cepcsw
  PREFIX         ${CMAKE_BINARY_DIR}/_deps
  SOURCE_DIR     ${CMAKE_BINARY_DIR}/_deps/kaltest-src
  BINARY_DIR     ${CMAKE_BINARY_DIR}/_deps/kaltest-build
  STAMP_DIR      ${CMAKE_BINARY_DIR}/_deps/kaltest-stamp
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND ${CMAKE_COMMAND}
                -DCMAKE_INSTALL_PREFIX=${KalTest_DIR}
                -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${CMAKE_BINARY_DIR}/lib
                -DCMAKE_PREFIX_PATH=${ILCUTIL_DIR}/lib/cmake/ILCUTIL
                -DCONFIGURE_FILE_INSTALL_DIR=lib/cmake/KalTest
                -DBUILD_WITH_T0_FIT=${BUILD_WITH_T0_FIT}
                 <SOURCE_DIR>
  BUILD_COMMAND ${CMAKE_COMMAND} --build . --target install
  DEPENDS        ILCUTIL
)

message(STATUS "will install into ${KalTest_DIR}")

set(KalTest_INCLUDE_DIRS ${KalTest_DIR}/include ${KalTest_DIR}/include/kaltest)
set(KalTest_LIBRARIES ${KalTest_DIR}/lib/libKalTest.so)
