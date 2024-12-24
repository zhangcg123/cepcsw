include(ExternalProject)

if (CEPCSW_EXTERNAL_AS_COMPONENT)
  set( aidaTT_DIR ${CMAKE_INSTALL_PREFIX} )
else()
  set( aidaTT_DIR ${CEPCSW_EXTERNAL_ROOT}/aidaTT )
endif()

ExternalProject_Add(
  aidaTT
  GIT_REPOSITORY https://code.ihep.ac.cn/cepc/externals/mirroring/aidaTT.git
  GIT_TAG        v00-10-cepcsw
  PREFIX         ${CMAKE_BINARY_DIR}/_deps
  SOURCE_DIR     ${CMAKE_BINARY_DIR}/_deps/aidatt-src
  BINARY_DIR     ${CMAKE_BINARY_DIR}/_deps/aidatt-build
  STAMP_DIR      ${CMAKE_BINARY_DIR}/_deps/aidatt-stamp
  CMAKE_ARGS    -DCMAKE_INSTALL_PREFIX=${aidaTT_DIR}
                -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${CMAKE_BINARY_DIR}/lib
                -DCMAKE_PREFIX_PATH=${ILCUTIL_DIR}/lib/cmake/ILCUTIL
                -DCONFIGURE_FILE_INSTALL_DIR=lib/cmake/aidaTT
                -DUSE_CXX11=FALSE
  UPDATE_COMMAND ""
  DEPENDS        ILCUTIL
)

message(STATUS "will install into ${aidaTT_DIR}")
