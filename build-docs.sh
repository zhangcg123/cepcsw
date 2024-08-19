#!/bin/bash
# Description:
#   This script is used to build the CEPCSW docs.

function check-sphinx() {
    which sphinx-build >& /dev/null
}

function run-build-docs() {
    pushd docs
    make html
    popd
}

if check-sphinx; then
    run-build-docs
else
    echo "Please setup sphinx before build the docs. " 1>&2
    exit -1
fi
