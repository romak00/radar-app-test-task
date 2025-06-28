#!/usr/bin/env bash
set -euo pipefail

CONF=${1:-release}
CONF_LOWER=${CONF,,}
CONF_UPPER=${CONF^^:0:1}${CONF_LOWER:1}

BUILD_DIR="build"
echo "==> Linux: config = ${CONF_UPPER}."

mkdir -p "${BUILD_DIR}"
pushd "${BUILD_DIR}" > /dev/null

cmake ../.. \
  -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=${CONF_UPPER} \
  -DCMAKE_CXX_STANDARD=20
make -j$(nproc)

popd > /dev/null

echo "==> Linux (${CONF_UPPER}) build finished."



