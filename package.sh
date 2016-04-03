#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"
TMP="/tmp/qfxcm-build.$$/"

echo "[building qfxcm in ${TMP}]"
mkdir -p ${TMP}
cd ${TMP}

cmake ${DIR}
make
make build_package

cd ->/dev/null

echo "[cleaning up the build from ${TMP}]"
rm -rf ${TMP}