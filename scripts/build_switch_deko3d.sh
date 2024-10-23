#!/bin/bash
set -e

BUILD_DIR=cmake-build-switch

# cd to wiliwili
cd "$(dirname $0)/.."
git config --global --add safe.directory `pwd`

BASE_URL="https://github.com/xfangfang/wiliwili/releases/download/v0.1.0/"

PKGS=(
    "libuam-f8c9eef01ffe06334d530393d636d69e2b52744b-1-any.pkg.tar.zst"
    "switch-ffmpeg-7.1-1-any.pkg.tar.zst"
    "switch-libmpv_deko3d-0.36.0-2-any.pkg.tar.zst"
    "switch-nspmini-48d4fc2-1-any.pkg.tar.xz"
    "hacBrewPack-3.05-1-any.pkg.tar.zst"
)
for PKG in "${PKGS[@]}"; do
    [ -f "${PKG}" ] || curl -LO ${BASE_URL}${PKG}
    dkp-pacman -U --noconfirm ${PKG}
done

cmake -B ${BUILD_DIR} -DCMAKE_BUILD_TYPE=Release -DBUILTIN_NSP=ON -DPLATFORM_SWITCH=ON -DUSE_DEKO3D=ON -DBRLS_UNITY_BUILD=ON -DCMAKE_UNITY_BUILD_BATCH_SIZE=16
make -C ${BUILD_DIR} wiliwili.nro -j$(nproc)