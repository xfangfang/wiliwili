#!/bin/bash
set -e

BUILD_DIR=cmake-build-switch

# cd to wiliwili
cd "$(dirname $0)/.."
git config --global --add safe.directory `pwd`

BASE_URL="https://github.com/xfangfang/wiliwili/releases/download/v0.1.0/"

PKGS=(
    "switch-libass-0.17.1-1-any.pkg.tar.zst"
    "switch-dav1d-1.2.1-1-any.pkg.tar.zst"
    "switch-ffmpeg-6.1-5-any.pkg.tar.zst"
    "switch-libmpv-0.36.0-2-any.pkg.tar.zst"
    "switch-nspmini-48d4fc2-1-any.pkg.tar.xz"
)
for PKG in "${PKGS[@]}"; do
    [ -f "${PKG}" ] || curl -LO ${BASE_URL}${PKG}
    dkp-pacman -U --noconfirm ${PKG}
done

HACBREWPACK="hacbrewpack-v3.06_linux-amd64.tar.gz"
[ -f "${HACBREWPACK}" ] || curl -LO https://github.com/dragonflylee/hacBrewPack/releases/download/v3.06/${HACBREWPACK}
tar zxf ${HACBREWPACK} -C /usr/local/bin

cmake -B ${BUILD_DIR} -DCMAKE_BUILD_TYPE=Release -DBUILTIN_NSP=ON -DPLATFORM_SWITCH=ON
make -C ${BUILD_DIR} wiliwili.nro -j$(nproc)