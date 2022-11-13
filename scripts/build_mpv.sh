set -e

# You can change to another version
MPV_VERSION=0.34.1

# cd to wiliwili
cd "$(dirname $0)/.."

# check mpv
MPV_PATH="mpv-${MPV_VERSION}"
if [ ! -d "${MPV_PATH}" ];then
    # download mpv
    echo "Download ${MPV_PATH}"
    wget https://github.com/mpv-player/mpv/archive/refs/tags/v${MPV_VERSION}.tar.gz -O ${MPV_PATH}.tar.gz
    tar -xzvf ${MPV_PATH}.tar.gz
    cd ${MPV_PATH}
    # patch for switch
    patch -Np1 -i ../scripts/switch/libmpv/mpv.patch
else
    echo "Found ${MPV_PATH}"
    cd ${MPV_PATH}
fi

# build
./bootstrap.py
source /opt/devkitpro/switchvars.sh
export CFLAGS="$CFLAGS -D_POSIX_VERSION=200809L -DNDEBUG -I`pwd`/osdep/switch"
LIBDIR=${PORTLIBS_PREFIX}/lib TARGET=aarch64-none-elf ./waf configure --prefix="${PORTLIBS_PREFIX}" \
    --disable-libmpv-shared --enable-libmpv-static --disable-cplayer \
    --enable-sdl2 --enable-sdl2-audio --enable-sdl2-gamepad --enable-sdl2-video \
    --disable-iconv --disable-jpeg --disable-libavdevice --disable-debug-build

sed -i 's/#define HAVE_POSIX 1/#define HAVE_POSIX 0/' build/config.h
sed -i 's/#define HAVE_POSIX_OR_MINGW 1/#define HAVE_POSIX 0/' build/config.h

./waf build

# install to devkitpro
sudo su <<HERE
source /opt/devkitpro/switchvars.sh
./waf install
HERE