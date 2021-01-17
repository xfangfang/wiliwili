# wiliwili

a third party bilibili client for Nintendo Switch.

# Usage

for Nentendo Switch: copy the wiliwili.nro to sdmc://switch folder.

for Linux: unzip the wiliwili.zip and run wiliwili

# Development (under ubuntu20.0.4)

recommand using vscode with cmake tools

## linux

### 1. install dependency

```
# for internet connection
sudo aot install libssl-dev
# for borealis
sudo apt install libglfw3-dev libglm-dev
#for mpv
sudo apt install libmpv-dev
# for building
sudo apt install cmake zip git
```

### 2. build wiliwili

```
mkdir -p linux_build && cd linux_build
rm -rf ./* && cmake -DPLATFORM_LINUX:BOOL=TRUE ..
make wiliwili.data
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

### 2. building switch ffmpeg library

```
sudo dkp-pacman -S devkitpro-pkgbuild-helpers
sudo apt install iat dist simh coreutils vit at bacula-console-qt atfs vim-addon-manager liballegro4-dev
```

###### choice1: using pacman-packages patch for ffmpeg(recommand)

```
cd wiliwili
wget https://ffmpeg.org/releases/ffmpeg-4.2.2.tar.xz
tar -xvJf ffmpeg-4.2.2.tar.xz
cd ffmpeg-4.2.2
patch -Np1 -i "../ffmpeg.patch"
source /opt/devkitpro/switchvars.sh
./configure --prefix=/opt/devkitpro/portlibs/switch \
    --extra-cflags='-DTCP_MAXSEG=2' \
    --enable-cross-compile --cross-prefix=aarch64-none-elf- --arch=aarch64 \
    --pkg-config=/opt/devkitpro/portlibs/switch/bin/aarch64-none-elf-pkg-config \
    --disable-shared --enable-static --target-os=horizon --enable-pic \
    --enable-asm --enable-neon --disable-runtime-cpudetect \https://github.com/Cpasjuste/pplay
    --disable-encoders --disable-muxers \
    --disable-programs --disable-debug --disable-doc
```

###### choice2: using ffmpeg master branch

```
git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg
cd ffmpeg
source /opt/devkitpro/switchvars.sh
./configure --prefix=/opt/devkitpro/portlibs/switch \
    --extra-cflags='-DTCP_MAXSEG=2' \
    --enable-cross-compile --cross-prefix=aarch64-none-elf- --arch=aarch64 \
    --pkg-config=/opt/devkitpro/portlibs/switch/bin/aarch64-none-elf-pkg-config \
    --disable-shared --enable-static --target-os=horizon --enable-pic \
    --enable-asm --enable-neon --disable-runtime-cpudetect \
    --enable-libass --enable-libfreetype --enable-libfribidi --enable-libtheora \
    --disable-protocols --enable-protocol='file,http' \
    --disable-filters --enable-filter='rotate,transpose' \
    --disable-encoders --disable-muxers \
    --disable-programs --disable-debug --disable-doc
sed 's/#define HAVE_INET_ATON 0/#define HAVE_INET_ATON 1/g' -i config.h
```

###### building ffmpeg

```
make -j4
sudo su
source /opt/devkitpro/switchvars.sh
make install
rm -r ${PORTLIBS_PREFIX}/share/
```

### 3. build wiliwili.nro

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