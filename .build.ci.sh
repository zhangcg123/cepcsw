#!/bin/bash
# This is wrapper to run the build.sh on CI

echo "CEPCSW_LCG_RELEASE: ${CEPCSW_LCG_RELEASE}"
echo "CEPCSW_LCG_PLATFORM: ${CEPCSW_LCG_PLATFORM}"
echo "CEPCSW_LCG_VERSION: ${CEPCSW_LCG_VERSION}"
echo "CEPCSW_BLDTOOL: ${CEPCSW_BLDTOOL}"

function build-with-log() {
    buildpid=
    logfile=mylog.txt

    if [ "$CEPCSW_LCG_RELEASE" = "KEY4HEP_STACK" ]; then
        logfile=mylog-k4.sh
        ./build-k4.sh >& ${logfile} &
        buildpid=$!
    else
        source setup.sh
        ./build.sh >& ${logfile} &
        buildpid=$!
    fi

    while ps -p $buildpid 2>/dev/null ; do
        sleep 60
    done &
    echoer=$!

    trap 'kill $echoer' 0

    wait $buildpid
    statuspid=$?

    tail -n100 ${logfile}

    exit $statuspid
}

function build-with-stdout() {
    if [ "$CEPCSW_LCG_RELEASE" = "KEY4HEP_STACK" ]; then
        ./build-k4.sh
    else
        source setup.sh
        ./build.sh
    fi
}

if [ -n "${GITHUB_ACTION}" ]; then
    build-with-log
else
    build-with-stdout
fi
