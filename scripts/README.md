# Build ffmpeg and mpv for switch

Build System: `Ubuntu 22.04`

### Build and install

```shell
libs=(ffmpeg mpv)
for lib in ${libs[@]}; do
    pushd switch/$lib
    dkp-makepkg -i
    popd 
done
```

### Another way (Not recommended)

```shell
sudo dkp-pacman -S switch-pkg-config dkp-toolchain-vars switch-zlib \
    switch-bzip2 switch-libass switch-libfribidi switch-freetype \
    switch-sdl2 switch-mesa switch-mbedtls
bash ./build_ffmpeg.sh
bash ./build_mpv.sh
```

# Precompiled

```
sudo dkp-pacman -U https://github.com/xfangfang/wiliwili/releases/download/v0.1.0/switch-ffmpeg-4.4.3-1-any.pkg.tar.xz

sudo dkp-pacman -U https://github.com/xfangfang/wiliwili/releases/download/v0.1.0/switch-libmpv-0.34.1-1-any.pkg.tar.xz
```


# Acknowledgement

Thanks to Cpasjuste and proconsule

- https://github.com/Cpasjuste/pplay
- https://github.com/proconsule/nxmp