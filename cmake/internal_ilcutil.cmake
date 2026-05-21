include(ExternalProject)

if (CEPCSW_EXTERNAL_AS_COMPONENT)
  set( ILCUTIL_DIR ${CMAKE_INSTALL_PREFIX} )
else()
  set( ILCUTIL_DIR ${CEPCSW_EXTERNAL_ROOT}/ILCUTIL )
endif()

ExternalProject_Add(
  ILCUTIL
  GIT_REPOSITORY https://code.ihep.ac.cn/cepc/externals/mirroring/iLCUtil.git
  GIT_TAG        v01-07-03-cepcsw
  PREFIX         ${CMAKE_BINARY_DIR}/_deps
  SOURCE_DIR     ${CMAKE_BINARY_DIR}/_deps/ilcutil-src
  BINARY_DIR     ${CMAKE_BINARY_DIR}/_deps/ilcutil-build
  STAMP_DIR      ${CMAKE_BINARY_DIR}/_deps/ilcutil-stamp
  CMAKE_ARGS    -DCMAKE_INSTALL_PREFIX=${ILCUTIL_DIR}
                -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${CMAKE_BINARY_DIR}/lib
                -DCONFIGURE_FILE_INSTALL_DIR=lib/cmake/ILCUTIL
                -DINSTALL_DOC=OFF
  UPDATE_COMMAND ""
)

message(STATUS "will install into ${ILCUTIL_DIR}")

set(streamlog_INCLUDE_DIRS ${ILCUTIL_DIR}/include)
set(streamlog_LIBRARIES ${ILCUTIL_DIR}/lib/libstreamlog.so)
