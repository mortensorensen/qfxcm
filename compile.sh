#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"
TMP="${DIR}/build"

echo "[building qfxcm in ${TMP}]"
mkdir -p ${TMP}
cd ${TMP}

cmake ${DIR} && make

cd ->/dev/null