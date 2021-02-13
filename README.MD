# wiliwili

a third party bilibili client for Linux & Nintendo Switch.

# Usage

for Nentendo Switch: copy the wiliwili.nro to sdmc://switch folder.

for Linux: unzip the wiliwili.zip and run wiliwili

# Development (under ubuntu20.04)

recommand using vscode with cmake tools

```
git clone --recursive https://github.com/xfangfang/wiliwili.git
# or
git clone https://github.com/xfangfang/wiliwili.git
cd wiliwili
git submodule update --init --recursive
```

## linux

### 1. install dependency

```
# for internet connection
sudo apt install libcurl4-openssl-dev
# for borealis
sudo apt install libglfw3-dev libglm-dev
# for mpv
sudo apt install libmpv-dev
# for building
sudo apt install cmake zip git
```

### 2. build wiliwili

```
mkdir -p linux_build && cd linux_build
rm -rf ./* && cmake -DPLATFORM_LINUX:BOOL=TRUE ..
make -j4
```


## switch

### 1. install devkitpro

```
wget https://github.com/devkitPro/pacman/releases/download/v1.0.2/devkitpro-pacman.amd64.deb
sudo apt-get install gdebi-core
sudo gdebi devkitpro-pacman.amd64.deb
sudo dkp-pacman -S switch-dev switch-glfw switch-mesa switch-glm switch-portlibs switch-glad
cat >> ~/.bashrc <<-EOF

# devkitpro
export DEVKITPRO=/opt/devkitpro
export PATH=\$PATH:\$DEVKITPRO/tools/bin:\$DEVKITPRO/devkitA64/bin
EOF
source ~/.bashrc
```

### 2. building switch ffmpeg library (optional)

```
sudo dkp-pacman -S devkitpro-pkgbuild-helpers
sudo apt install iat dist simh coreutils vit at bacula-console-qt atfs vim-addon-manager liballegro4-dev
```

###### set ffmpeg config (thanks for pplay)

```
cd wiliwili
wget https://ffmpeg.org/releases/ffmpeg-4.3.1.tar.xz
tar -xvJf ffmpeg-4.3.1.tar.xz
cd ffmpeg-4.3.1
source /opt/devkitpro/switchvars.sh
./configure --prefix=/opt/devkitpro/portlibs/switch \
    --extra-cflags='-D_GNU_SOURCE -O2 -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec' \
    --extra-cxxflags='-D_GNU_SOURCE -O2 -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec' \
    --extra-ldflags='-fPIE -L${PORTLIBS_PREFIX}/lib -L${DEVKITPRO}/libnx/lib' \
    --enable-cross-compile --cross-prefix=aarch64-none-elf- --arch=aarch64 \
    --pkg-config=/opt/devkitpro/portlibs/switch/bin/aarch64-none-elf-pkg-config \
    --disable-shared --enable-static --target-os=linux --enable-pic \
    --enable-asm --enable-neon --disable-runtime-cpudetect --disable-autodetect\
    --enable-libass --enable-libfreetype --enable-libfribidi --enable-libtheora \
    --disable-protocols --enable-protocol='file,http' \
    --disable-filters --enable-filter='rotate,transpose' \
    --disable-encoders --disable-muxers --disable-bsfs\
    --disable-programs --disable-debug --disable-doc
sed 's/#define HAVE_INET_ATON 0/#define HAVE_INET_ATON 1/g' -i config.h
sed 's/#define HAVE_CLOSESOCKET 1/#define HAVE_CLOSESOCKET 0/g' -i config.h
sed 's/#define HAVE_MMAP 1/#define HAVE_MMAP 0/g' -i config.h
```

###### building ffmpeg

```
make -j4
sudo su
source /opt/devkitpro/switchvars.sh
make install
rm -r ${PORTLIBS_PREFIX}/share/
```

### 3. build switch mpv library (optional)

##### set libmpv config

```
cd wiliwili
wget https://github.com/mpv-player/mpv/archive/v0.33.0.tar.gz
tar -xvJf v0.33.0.tar.gz
cd mpv-0.33.0
patch -Np1 -i ../mpv.patch
source /opt/devkitpro/switchvars.sh
./bootstrap.py
export CFLAGS="$CFLAGS -D_POSIX_VERSION=200809L -I`pwd`/osdep/switch"
TARGET=aarch64-none-elf ./waf configure --prefix="${PORTLIBS_PREFIX}" \
    --disable-libmpv-shared --enable-libmpv-static --disable-cplayer \
    --enable-sdl2 --enable-sdl2-audio --enable-sdl2-gamepad --enable-sdl2-video \
    --disable-iconv --disable-jpeg --disable-libavdevice --disable-debug-build

sed -i 's/#define HAVE_POSIX 1/#define HAVE_POSIX 0/' build/config.h
sed -i 's/#define HAVE_POSIX_OR_MINGW 1/#define HAVE_POSIX 0/' build/config.h
```

##### building libmpv

```
./waf build
sudo ./waf install
```

### 4. build wiliwili.nro

```
mkdir -p switch_build && cd switch_build
rm -rf ./* && cmake -DPLATFORM_SWITCH:BOOL=TRUE ..
make -j4 wiliwili.nro
nxlink -s wiliwili.nro
```

# Reference

https://devkitpro.org/wiki/devkitPro_pacman

https://github.com/devkitPro/pacman/releases

https://github.com/devkitPro/pacman-packages

https://github.com/natinusala/borealis

https://github.com/Cpasjuste/pplay

https://github.com/whoshuu/cpr

https://github.com/nlohmann/json

https://github.com/adobe-fonts/source-han-sans

https://github.com/progschj/ThreadPool
