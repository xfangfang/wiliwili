#!/usr/bin/env bash
# Cross compile libmpv for the PS5 payload environment and install it into the
# ps5-payload-sdk sysroot, so that wiliwili can link against it.
#
# Requirements:
#   - PS5_PAYLOAD_SDK points to the ps5-payload-sdk directory
#   - meson, ninja and a LLVM toolchain (llvm-config in PATH or LLVM_CONFIG set)
#
# Usage:
#   PS5_PAYLOAD_SDK=/path/to/ps5-payload-sdk scripts/ps5/build_libmpv.sh

set -e

MPV_VERSION=0.36.0

if [[ -z "${PS5_PAYLOAD_SDK}" ]]; then
    echo "Please set PS5_PAYLOAD_SDK to the ps5-payload-sdk directory" >&2
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
WORK_DIR="${WORK_DIR:-${HOME}/.cache/wiliwili-ps5}"

mkdir -p "${WORK_DIR}"
cd "${WORK_DIR}"

if [[ ! -d "mpv-${MPV_VERSION}" ]]; then
    echo "Downloading mpv ${MPV_VERSION}" >&2
    curl --location --fail --retry 3 --retry-delay 2 --output "mpv-${MPV_VERSION}.tar.gz" \
        "https://github.com/mpv-player/mpv/archive/refs/tags/v${MPV_VERSION}.tar.gz"
    tar -xzf "mpv-${MPV_VERSION}.tar.gz"
    # mpv 0.36.0 was released before FFmpeg 7, this patch makes it compile with it
    patch --strip=1 --directory="mpv-${MPV_VERSION}" < "${SCRIPT_DIR}/mpv/mpv.patch"
fi

cd "mpv-${MPV_VERSION}"

if [[ -x "${PS5_PAYLOAD_SDK}/bin/prospero-meson" ]]; then
    MESON="${PS5_PAYLOAD_SDK}/bin/prospero-meson setup"
else
    MESON="meson setup --cross-file=${PS5_PAYLOAD_SDK}/toolchain/prospero.ini"
fi

${MESON} build \
    --buildtype=release \
    -Dlibmpv=true -Dcplayer=false \
    -Dsdl2=enabled -Dsdl2-gamepad=disabled \
    -Diconv=disabled -Dlua=disabled -Djpeg=disabled -Dlibavdevice=disabled \
    -Degl=disabled \
    -Dhtml-build=disabled -Dmanpage-build=disabled

ninja -C build
meson install -C build --destdir="${PS5_PAYLOAD_SDK}/target"

echo "libmpv ${MPV_VERSION} has been installed into ${PS5_PAYLOAD_SDK}/target/user/homebrew" >&2
