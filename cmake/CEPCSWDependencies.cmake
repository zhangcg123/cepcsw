#[[

Find all the dependencies here, so in each package user don't need to find the packages again.

- CLHEP
- DD4hep
- EDM4hep
- Garfiel++/Garfieldpp
- Gaudi
- Geant4
- GEAR
- GSL
- HepMC
- k4FWCore
- LCContent
- LCIO
- PandoraSDK
- podio
- ROOT
- CKF
#]]

##############################################################################
# import Gaudi at beginning
##############################################################################
find_package(Gaudi REQUIRED)


find_package(CLHEP REQUIRED;CONFIG)
find_package(DD4hep COMPONENTS DDCore DDG4 DDParsers DDRec REQUIRED)
include(DD4hep) # include necessary macro
find_package(EDM4HEP REQUIRED)
find_package(Eigen3)
find_package(Garfield)
find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})
find_package(GEAR REQUIRED)
find_package(GenFit)
find_package(GSL REQUIRED)
find_package(HepMC)
find_package(k4FWCore REQUIRED)
find_package(LCContent REQUIRED)
find_package(LCIO REQUIRED)
find_package(OnnxRuntime REQUIRED)
find_package(PandoraSDK REQUIRED)
find_package(podio REQUIRED)
find_package(Pythia8 REQUIRED)
find_package(ROOT COMPONENTS EG Geom GenVector Graf Graf3d Gpad MathCore Net RIO Tree TreePlayer REQUIRED)

if (CEPCSW_USE_SYSTEM_CKF_BELLE)
  message("Try to use an existing installation of CKF BELLE")
  find_package(CKF)
else()
  message("Try to use an internal installation of CKF BELLE")
  include("${CMAKE_CURRENT_LIST_DIR}/internal_ckf.cmake")
endif()


if (CEPCSW_USE_SYSTEM_EDM4CEPC)
  message("Try to use an existing installation of EDM4CEPC")
  find_package(EDM4CEPC)
else()
  message("Try to use an internal installation of EDM4CEPC")
  include("${CMAKE_CURRENT_LIST_DIR}/internal_edm4cepc.cmake")
endif()

find_package(FastJet)
