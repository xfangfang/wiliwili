# Build ffmpeg and mpv for switch

If building using macOS, additional tools need to be installed.

```shell
brew install gnu-sed coreutils

# you need add a "gnubin" directory to your PATH with: 
PATH="`brew --prefix gsed`/libexec/gnubin:$PATH"
PATH="`brew --prefix coreutils`/libexec/gnubin:$PATH"
```

### Build and install

```shell
sudo dkp-pacman -S switch-pkg-config dkp-toolchain-vars switch-zlib \
    switch-bzip2 switch-libass switch-libfribidi switch-freetype \
    switch-harfbuzz switch-mesa switch-mbedtls

libs=(libuam deko3d dav1d libass ffmpeg mpv)
for lib in ${libs[@]}; do
    pushd switch/$lib
    dkp-makepkg -i
    popd 
done
```

# Precompiled

```
base_url="https://github.com/xfangfang/wiliwili/releases/download/v0.1.0"
sudo dkp-pacman -U \
    $base_url/switch-ffmpeg-7.1-1-any.pkg.tar.zst \
    $base_url/switch-libmpv-0.36.0-3-any.pkg.tar.zst
```

# Acknowledgement

Huge thanks to averne Cpasjuste and proconsule

- https://github.com/Cpasjuste/pplay
- https://github.com/proconsule/nxmp