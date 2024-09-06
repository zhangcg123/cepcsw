#!/bin/bash
# Description:
#   Run the tests using ctest
#
# Author:
#   Tao Lin <lintao AT ihep.ac.cn>

##############################################################################
# Utilities
##############################################################################
function build-dir() {
    local blddir=build

    if [ -n "${CEPCSW_BLDTOOL}" ]; then
        blddir=${blddir}.${CEPCSW_BLDTOOL}
    fi

    # If detect the extra env var, append it to the build dir
    if [ -n "${CEPCSW_LCG_VERSION}" ]; then
        blddir=${blddir}.${CEPCSW_LCG_VERSION}
    fi
    if [ -n "${CEPCSW_LCG_PLATFORM}" ]; then
        blddir=${blddir}.${CEPCSW_LCG_PLATFORM}
    fi

    echo $blddir
}

function junit-output() {
    local default=cepcsw-ctest-result.xml
    echo ${CEPCSW_JUNIT_OUTPUT:-$default}
}

##############################################################################
# Main
##############################################################################


echo "CEPCSW_LCG_RELEASE: ${CEPCSW_LCG_RELEASE}"
echo "CEPCSW_LCG_PLATFORM: ${CEPCSW_LCG_PLATFORM}"
echo "CEPCSW_LCG_VERSION: ${CEPCSW_LCG_VERSION}"
echo "CEPCSW_BLDTOOL: ${CEPCSW_BLDTOOL}"

source setup.sh

# reconfigure if directory change
pushd $(build-dir)
cmake ..
popd

ctest --output-junit $(junit-output) --test-dir $(build-dir)
