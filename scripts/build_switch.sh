set -e

BUILD_DIR=cmake-build-switch

# cd to wiliwili
cd "$(dirname $0)/.."

BASE_URL="https://github.com/xfangfang/wiliwili/releases/download/v0.1.0"
LIBASS="switch-libass-0.17.1-1-any.pkg.tar.zst"
FFMPEG="switch-ffmpeg-4.4.4-1-any.pkg.tar.zst"
MPV="switch-libmpv-0.35.1-1-any.pkg.tar.zst"
NSPMINI="switch-nspmini-48d4fc2-1-any.pkg.tar.xz"

if [ ! -f "${LIBASS}" ];then
    wget ${BASE_URL}/${LIBASS}
fi
if [ ! -f "${FFMPEG}" ];then
    wget ${BASE_URL}/${FFMPEG}
fi
if [ ! -f "${MPV}" ];then
    wget ${BASE_URL}/${MPV}
fi
if [ ! -f "${NSPMINI}" ];then
    wget ${BASE_URL}/${NSPMINI}
fi

dkp-pacman -U --noconfirm ${LIBASS} ${FFMPEG} ${MPV} ${NSPMINI}

cmake -B ${BUILD_DIR} -DCMAKE_BUILD_TYPE=Release -DBUILTIN_NSP=ON
make -C ${BUILD_DIR} wiliwili.nro -j$(nproc)