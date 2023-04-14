dkp-pacman -Syu --noconfirm
set -e

BUILD_DIR=cmake-build-switch

# cd to wiliwili
cd "$(dirname $0)/.."

BASE_URL="https://github.com/xfangfang/wiliwili/releases/download/v0.1.0"
FFMPEG="switch-ffmpeg-4.4.3-1-any.pkg.tar.xz"
MPV="switch-libmpv-0.34.1-1-any.pkg.tar.xz"
NSPMINI="switch-nspmini-48d4fc2-1-any.pkg.tar.xz"

if [ ! -f "${FFMPEG}" ];then
    wget ${BASE_URL}/${FFMPEG}
fi
if [ ! -f "${MPV}" ];then
    wget ${BASE_URL}/${MPV}
fi
if [ ! -f "${NSPMINI}" ];then
    wget ${BASE_URL}/${NSPMINI}
fi

dkp-pacman -U --noconfirm ${FFMPEG} ${MPV} ${NSPMINI}

cmake -B ${BUILD_DIR} -DCMAKE_BUILD_TYPE=Release -DBUILTIN_NSP=ON
make -C ${BUILD_DIR} wiliwili.nro -j$(nproc)