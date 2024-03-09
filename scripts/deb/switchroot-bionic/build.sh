#!/bin/bash

set -e

export CMAKE_PREFIX_PATH=/opt/wiliwili
export PKG_CONFIG_PATH=$CMAKE_PREFIX_PATH/lib/pkgconfig
export LD_LIBRARY_PATH=$CMAKE_PREFIX_PATH/lib:/usr/lib/aarch64-linux-gnu/tegra

git clone https://gitlab.com/switchroot/switch-l4t-multimedia/FFmpeg.git --depth=1 /tmp/ffmpeg
git clone https://gitlab.com/switchroot/switch-l4t-multimedia/mpv.git --depth=1 /tmp/mpv
git clone https://github.com/curl/curl.git --depth=1 --branch=curl-8_5_0 /tmp/curl

cd /opt/library/borealis/library/lib/extern/glfw
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$CMAKE_PREFIX_PATH \
  -DCMAKE_INSTALL_RPATH=$CMAKE_PREFIX_PATH/lib -DBUILD_SHARED_LIBS=ON -DGLFW_BUILD_WAYLAND=OFF \
  -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
cmake --build build -j$(nproc)
cmake --install build

cd /tmp/curl
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$CMAKE_PREFIX_PATH \
  -DCMAKE_INSTALL_RPATH=$CMAKE_PREFIX_PATH/lib -DBUILD_SHARED_LIBS=ON -DCURL_USE_OPENSSL=ON \
  -DHTTP_ONLY=ON -DCURL_DISABLE_PROGRESS_METER=ON -DBUILD_CURL_EXE=OFF -DBUILD_TESTING=OFF \
  -DUSE_LIBIDN2=OFF -DCURL_USE_LIBSSH2=OFF -DCURL_USE_LIBPSL=OFF -DBUILD_LIBCURL_DOCS=OFF
cmake --build build -j$(nproc)
cmake --install build

cd /tmp/ffmpeg
./configure --prefix=$CMAKE_PREFIX_PATH --enable-shared --disable-static \
  --extra-cflags='-march=armv8-a+simd+crypto+crc -mtune=cortex-a57 -I/usr/src/jetson_multimedia_api/include' \
  --extra-ldflags='-L/usr/lib/aarch64-linux-gnu/tegra' \
  --extra-libs='-lpthread -lm -lnvbuf_utils -lv4l2' \
  --ld=g++ --enable-nonfree --enable-openssl --enable-libv4l2 --enable-nvv4l2 \
  --enable-opengl --disable-doc --enable-asm --enable-neon --disable-debug \
  --enable-libass --enable-demuxer=hls --disable-muxers --disable-avdevice \
  --disable-protocols --enable-protocol='file,http,tcp,rtmp,hls,https,tls' \
  --disable-encoders --disable-programs --enable-rpath
make -j$(nproc)
make install

cd /tmp/mpv
./bootstrap.py
LIBDIR=$CMAKE_PREFIX_PATH/lib RPATH=$CMAKE_PREFIX_PATH/lib ./waf configure --prefix=$CMAKE_PREFIX_PATH \
  --disable-libmpv-static --enable-libmpv-shared --disable-debug-build --disable-libavdevice --disable-cplayer
./waf install

cd /opt
mkdir -p /tmp/deb/DEBIAN /tmp/deb/usr /tmp/deb/opt/wiliwili/lib

cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$CMAKE_PREFIX_PATH -DPLATFORM_DESKTOP=ON \
  -DUSE_SYSTEM_CURL=ON -DUSE_SYSTEM_GLFW=ON -DHOMEBREW_MPV=$CMAKE_PREFIX_PATH \
  -DINSTALL=ON -DCUSTOM_RESOURCES_DIR=$CMAKE_PREFIX_PATH -DCMAKE_INSTALL_RPATH=$CMAKE_PREFIX_PATH/lib
cmake --build build -j$(nproc)
DESTDIR="/tmp/deb" cmake --install build

cp -d /opt/wiliwili/lib/*.so.* /tmp/deb/opt/wiliwili/lib
mv /tmp/deb/opt/wiliwili/share /tmp/deb/usr
sed -i 's|Exec=wiliwili|Exec=/opt/wiliwili/bin/wiliwili|' /tmp/deb/usr/share/applications/cn.xfangfang.wiliwili.desktop
cp scripts/deb/switchroot-bionic/control /tmp/deb/DEBIAN
dpkg --build /tmp/deb wiliwili-Linux-aarch64-switchroot-ubuntu.deb
