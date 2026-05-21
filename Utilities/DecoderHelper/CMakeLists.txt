
gaudi_add_library(DecoderHelperLib
                  SOURCES src/DD4hep2Lcio.cc
)

install(TARGETS DecoderHelperLib
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
