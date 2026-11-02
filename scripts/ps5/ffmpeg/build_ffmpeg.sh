#!/usr/bin/env bash
# Cross compile FFmpeg for the console, with the console's own hardware video
# decoder, and install it into the ps5-payload-sdk sysroot so that mpv and
# wiliwili link against it.
#
# The decoder is added by a patch: the chip is reached through a system module
# the application loads at startup, and its structures and the way it wants
# its memory come from the VLC for PS5 port.
#
# Requirements:
#   - PS5_PAYLOAD_SDK points to the ps5-payload-sdk directory
#   - PS5_BOILERPLATE points to the ps5-native-app-boilerplate directory
#     (its compiler wrapper is used, the SDK's own wrapper needs a patched
#     LLVM that is not installed)
#   - PS5_CLANG points to a clang 18, LLVM_CONFIG to its llvm-config
#
# Usage:
#   PS5_PAYLOAD_SDK=/path/to/ps5-payload-sdk \
#   PS5_BOILERPLATE=/path/to/ps5-native-app-boilerplate \
#   scripts/ps5/ffmpeg/build_ffmpeg.sh

set -e

FFMPEG_VERSION=7.1.1

if [[ -z "${PS5_PAYLOAD_SDK}" ]]; then
    echo "Please set PS5_PAYLOAD_SDK to the ps5-payload-sdk directory" >&2
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
WORK_DIR="${WORK_DIR:-${HOME}/.cache/wiliwili-ps5}"
PREFIX="${PREFIX:-${WORK_DIR}/ffmpeg-install}"

mkdir -p "${WORK_DIR}"
cd "${WORK_DIR}"

if [[ ! -d "ffmpeg-${FFMPEG_VERSION}" ]]; then
    echo "Downloading FFmpeg ${FFMPEG_VERSION}" >&2
    curl --location --fail --retry 3 --retry-delay 2 --output "ffmpeg-${FFMPEG_VERSION}.tar.xz" \
        "https://ffmpeg.org/releases/ffmpeg-${FFMPEG_VERSION}.tar.xz"
    tar -xf "ffmpeg-${FFMPEG_VERSION}.tar.xz"
fi

cd "ffmpeg-${FFMPEG_VERSION}"

# The patch adds the decoder; it is applied again when the tree already has it.
if ! grep -q "h264_ps5" libavcodec/allcodecs.c; then
    patch --strip=1 --directory="." < "${SCRIPT_DIR}/ps5-decoder.patch"
fi

# The compiler: the boilerplate's wrapper, with the SDK's tools in the way.
export PS5_PAYLOAD_SDK
export PS5_CLANG="${PS5_CLANG:-$(command -v clang-18 || true)}"
export LLVM_CONFIG="${LLVM_CONFIG:-$(command -v llvm-config-18 || command -v llvm-config || true)}"
export PATH="${PS5_PAYLOAD_SDK}/bin:${PATH}"
export PKG_CONFIG_PATH="${PS5_PAYLOAD_SDK}/target/user/homebrew/lib/pkgconfig"

CC="${PS5_BOILERPLATE}/tooling/prospero-clang18"
if [[ ! -x "${CC}" ]]; then
    echo "Please set PS5_BOILERPLATE to the ps5-native-app-boilerplate directory" >&2
    exit 1
fi

mkdir -p "${WORK_DIR}/ffmpeg-build"
cd "${WORK_DIR}/ffmpeg-build"

"${WORK_DIR}/ffmpeg-${FFMPEG_VERSION}/configure" \
    --prefix="${PREFIX}" \
    --enable-cross-compile \
    --arch=x86_64 \
    --target-os=freebsd \
    --disable-shared \
    --enable-static \
    --disable-debug \
    --disable-doc \
    --disable-programs \
    --enable-gpl \
    --enable-version3 \
    --enable-openssl \
    --enable-libfreetype \
    --enable-libfribidi \
    --enable-libharfbuzz \
    --enable-libass \
    --enable-libx264 \
    --enable-lzma \
    --enable-zlib \
    --enable-iconv \
    --enable-decoder=h264_ps5 \
    --cc="${CC}" \
    --cxx="${CC}++" \
    --nm="${PS5_PAYLOAD_SDK}/bin/prospero-nm" \
    --ar="${PS5_PAYLOAD_SDK}/bin/prospero-ar" \
    --ranlib="${PS5_PAYLOAD_SDK}/bin/prospero-ranlib" \
    --strip="${PS5_PAYLOAD_SDK}/bin/prospero-strip" \
    --pkg-config="${PS5_PAYLOAD_SDK}/bin/prospero-pkg-config" \
    --extra-cflags="-I${PS5_PAYLOAD_SDK}/target/user/homebrew/include" \
    --extra-ldflags="-L${PS5_PAYLOAD_SDK}/target/user/homebrew/lib -L${PS5_PAYLOAD_SDK}/target/lib ${PS5_PAYLOAD_SDK}/target/lib/crt1.o -lkernel_web -lSceLibcInternal -lSceNet -lc"

make -j"$(sysctl -n hw.ncpu 2>/dev/null || nproc)"
make install

echo "FFmpeg ${FFMPEG_VERSION} with the console's video decoder is in ${PREFIX}" >&2
