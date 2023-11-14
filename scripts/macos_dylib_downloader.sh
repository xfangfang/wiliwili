set -e

# ./macos_dylib_downloader.sh <arch>
# arch: arm64, x86_64, universal

mkdir -p deps

BASEURL="https://github.com/xfangfang/wiliwili/releases/download/v0.1.0"
PACKAGE="macos_dylib_ffmpeg61_mpv36_$1.tar.gz"
DOWNLOAD_FLAG="${PACKAGE}.done"

if [ ! -f "${DOWNLOAD_FLAG}" ];then
    echo "Download ${PACKAGE}"
    rm -rf "${PACKAGE}"
    wget "${BASEURL}/${PACKAGE}"
    tar -xzvf "${PACKAGE}" -C deps
    touch "${PACKAGE}.done"
else
    echo "Found ${PACKAGE}"
fi
