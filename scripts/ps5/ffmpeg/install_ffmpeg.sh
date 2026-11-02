#!/usr/bin/env bash
# Put the FFmpeg that carries the console's hardware video decoder into the
# ps5-payload-sdk sysroot, so that mpv and wiliwili link against it.
#
# The FFmpeg that comes with the sysroot has no decoder for the console's
# video chip. The one that is built replaces it; the old libraries are kept
# aside, so that the sysroot can be put back the way it was.
#
# Usage:
#   PS5_PAYLOAD_SDK=/path/to/ps5-payload-sdk scripts/ps5/ffmpeg/install_ffmpeg.sh

set -e

if [[ -z "${PS5_PAYLOAD_SDK}" ]]; then
    echo "Please set PS5_PAYLOAD_SDK to the ps5-payload-sdk directory" >&2
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
WORK_DIR="${WORK_DIR:-${HOME}/.cache/wiliwili-ps5}"
PREFIX="${PREFIX:-${WORK_DIR}/ffmpeg-install}"
TARGET="${PS5_PAYLOAD_SDK}/target/user/homebrew"
BACKUP="${WORK_DIR}/ffmpeg-sdk-backup"

if [[ ! -d "${PREFIX}/lib" ]]; then
    echo "There is no FFmpeg in ${PREFIX}: build it first" >&2
    exit 1
fi

mkdir -p "${BACKUP}"

# The libraries that come with the sysroot are kept aside.
for lib in libavcodec libavdevice libavfilter libavformat libavutil libpostproc libswscale libswresample; do
    if [[ -f "${TARGET}/lib/${lib}.a" && ! -f "${BACKUP}/${lib}.a" ]]; then
        cp "${TARGET}/lib/${lib}.a" "${BACKUP}/${lib}.a"
    fi
done

for lib in libavcodec libavdevice libavfilter libavformat libavutil libpostproc libswscale libswresample; do
    cp -f "${PREFIX}/lib/${lib}.a" "${TARGET}/lib/${lib}.a"
done

# The headers and the files that say where the libraries are.
rm -rf "${TARGET}/include/libavcodec" "${TARGET}/include/libavdevice" \
    "${TARGET}/include/libavfilter" "${TARGET}/include/libavformat" \
    "${TARGET}/include/libavutil" "${TARGET}/include/libswscale" \
    "${TARGET}/include/libswresample"
cp -R "${PREFIX}/include/libavcodec" "${PREFIX}/include/libavdevice" \
    "${PREFIX}/include/libavfilter" "${PREFIX}/include/libavformat" \
    "${PREFIX}/include/libavutil" "${PREFIX}/include/libswscale" \
    "${PREFIX}/include/libswresample" "${TARGET}/include/"

for pc in libavcodec libavdevice libavfilter libavformat libavutil libpostproc libswscale libswresample; do
    if [[ -f "${PREFIX}/lib/pkgconfig/${pc}.pc" ]]; then
        sed 's|^prefix=.*|prefix=/user/homebrew|' \
            "${PREFIX}/lib/pkgconfig/${pc}.pc" > "${TARGET}/lib/pkgconfig/${pc}.pc"
    fi
done

echo "The FFmpeg with the console's video decoder is in ${TARGET}" >&2
echo "The libraries that were there before are in ${BACKUP}" >&2
