# This variable will be used by GaudiToolbox.cmake to generate the cepcswenv.sh

set(RUN_SCRIPT_EXTRA_COMMANDS "${RUN_SCRIPT_EXTRA_COMMANDS}

CEPCSW_PROJECT_SOURCE_DIR=${CMAKE_SOURCE_DIR} # this is hardcode
export CEPCSW_ROOT=\${CEPCSW_ROOT:-\${CEPCSW_PROJECT_SOURCE_DIR}} # allow user to override the default

export DETCEPCV4ROOT=\${CEPCSW_ROOT}/Detector/DetCEPCv4
export DETCRDROOT=\${CEPCSW_ROOT}/Detector/DetCRD
export DETDRIFTCHAMBERROOT=\${CEPCSW_ROOT}/Detector/DetDriftChamber
")
