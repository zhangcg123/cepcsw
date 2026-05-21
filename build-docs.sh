#!/bin/bash
# Description:
#   This script is used to build the CEPCSW docs.

function install-sphinx() {
    if [ ! -d "venv" ]; then
        python -m venv venv
        source venv/bin/activate
        # pip install -r requirements.txt
        pip install sphinx-rtd-theme myst-parser
    else
        source venv/bin/activate
    fi
}

function check-sphinx() {
    which sphinx-build >& /dev/null
}

function run-build-docs() {
    make html
}

pushd docs
install-sphinx

if check-sphinx; then
    run-build-docs
else
    echo "Please setup sphinx before build the docs. " 1>&2
    exit -1
fi
popd
