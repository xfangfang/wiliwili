#!/bin/bash
set -e

BUILD_DIR=cmake-build-ps5

# cd to wiliwili
cd "$(dirname $0)/.."

if [[ -z "${PS5_PAYLOAD_SDK}" ]]; then
    # the ps5-payload-sdk ships SDL2, mpv's dependencies and a LLVM based
    # toolchain, see scripts/ps5/install_sdk.sh for details
    PS5_PAYLOAD_SDK="${HOME}/.local/opt/ps5-payload-sdk"
    scripts/ps5/install_sdk.sh "${PS5_PAYLOAD_SDK}"
    export PS5_PAYLOAD_SDK
fi

if ! ${PS5_PAYLOAD_SDK}/bin/prospero-pkg-config --exists mpv; then
    echo "libmpv not found in the sysroot, building it first" >&2
    scripts/ps5/build_libmpv.sh
fi

cmake -B ${BUILD_DIR} \
    -DCMAKE_BUILD_TYPE=Release \
    -DPLATFORM_PS5=ON \
    -DBRLS_UNITY_BUILD=${BRLS_UNITY_BUILD:-ON} -DCMAKE_UNITY_BUILD_BATCH_SIZE=16

cmake --build ${BUILD_DIR} -j$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu)

# the optional payload that adds a wiliwili entry to the home screen
scripts/ps5/install_tile.sh -b ${BUILD_DIR}
