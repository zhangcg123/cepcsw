# CMake: building software
There is a script `build.sh` to build the CEPCSW. Inside this script, it consists of three major steps:
* configure: in this step, a *build directory* is created and the cmake reads `CMakeLists.txt` in *source code directory* and generates config files which will be used to build the project. 
* build: in this step, the project is built according to the config files under *build directory*. 
* install: in this step, the libraries, header files and python scripts are installed into *install directory*, which is `InstallArea`.

The `cmake` command reads the `CMakeLists.txt` files recursively to produce the config files. As there are some common patterns to build the packages in the project, we use some macros from Gaudi framework.

* `gaudi_add_library`: produce a shared library, which will be linked by other libraries.
  * The source code files need to be added after `SOURCES` option.
  * The link libraries need to be added after `LINK` option.
* `gaudi_add_module`: produce a module library, which will be imported by Gaudi. Note, this library should not be linked by other libraries.
  * Options `SOURCES` and `LINK` are same as `gaudi_add_library`.

We also use some macros from CMake.
* `target_include_directories`: specify the include directories when build a target. 
* `install`: install the targets. 

Examples could be found in CEPCSW. 

For example, the `RecPFACyber` package uses all the macros:
```cmake
gaudi_add_library(CrystalCaloRecLib
                  SOURCES src/Objects/CaloUnit.cc
                          src/Objects/CaloHit.cc
                          ...
                  LINK    k4FWCore::k4FWCore
                          GearSvc
                          CrystalEcalSvcLib
                          ...
)

gaudi_add_module(CrystalCaloRec
                 SOURCES src/CyberPFAlg.cpp
                         src/CyberDataCol.cpp
                         ...
                 LINK    CrystalCaloRecLib
)

target_include_directories(CrystalCaloRec PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS CrystalCaloRecLib CrystalCaloRec
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
```

