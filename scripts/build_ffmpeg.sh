set -e

# You can change to another version
FFMPEG_VERSION=4.4.3

# cd to wiliwili
cd "$(dirname $0)/.."

# check ffmpeg
FFMPEG_PATH="ffmpeg-${FFMPEG_VERSION}"
if [ ! -d "${FFMPEG_PATH}" ];then
    echo "Download ${FFMPEG_PATH}"
    wget https://ffmpeg.org/releases/${FFMPEG_PATH}.tar.gz
    tar -xzvf ${FFMPEG_PATH}.tar.gz
    cd ${FFMPEG_PATH}
    # patch for switch
    patch -Np1 -i  ../scripts/ffmpeg.patch
else
    echo "Found ${FFMPEG_PATH}"
    cd ${FFMPEG_PATH}
fi

# build
source /opt/devkitpro/switchvars.sh

./configure --prefix=$PORTLIBS_PREFIX --disable-shared --enable-static \
	--cross-prefix=aarch64-none-elf- \
	--enable-cross-compile \
	--arch=aarch64 \
	--target-os=horizon \
	--enable-pic \
	--extra-cflags='-D__SWITCH__ -D_GNU_SOURCE -O2 -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec' \
	--extra-cxxflags='-D__SWITCH__ -D_GNU_SOURCE -O2 -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec' \
	--extra-ldflags='-fPIE -L${PORTLIBS_PREFIX}/lib -L${DEVKITPRO}/libnx/lib' \
	--disable-runtime-cpudetect \
	--disable-programs \
	--disable-debug \
	--disable-doc \
	--disable-autodetect \
	--disable-avdevice \
	--disable-hwaccels \
	--disable-encoders \
	--enable-swscale \
	--enable-swresample \
	--enable-network \
	--disable-protocols \
	--enable-protocol='file,http,tcp,udp,rtmp,hls,https,tls'\
	--enable-asm --enable-neon \
	--enable-zlib --enable-bzlib \
	--enable-libass --enable-libfreetype \
	--enable-libfribidi --enable-mbedtls \
	--enable-version3 --enable-demuxer=hls

# Dont't use the one in libnx, because it's too slow
sed 's/#define HAVE_GETNAMEINFO 1/#define HAVE_GETNAMEINFO 0/g' -i config.h

make -j$(nproc)

# install to devkitpro
sudo su <<HERE
source /opt/devkitpro/switchvars.sh
make install
HERE