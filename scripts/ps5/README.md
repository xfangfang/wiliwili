# PS5

wiliwili runs on jailbroken PS5s as a homebrew payload, loaded by
[websrv](https://github.com/ps5-payload-dev/websrv) (which also provides `ftpsrv` on port 2121
and the `hbldr` endpoint on port 8080) or by
[elfldr](https://github.com/ps5-payload-dev/elfldr).

The cross compilation environment comes from
[ps5-payload-dev/pacbrew-repo](https://github.com/ps5-payload-dev/pacbrew-repo), it contains the
prospero toolchain wrappers, the SDK sysroot, and prebuilt SDL2, FFmpeg, Mesa (OpenGL via zink),
libass, OpenSSL, curl and friends.

### Requirements

- LLVM 18 (the toolchain wrappers call the mode-matched host `clang`/`lld`)
  - macOS: `brew install llvm@18 && export LLVM_CONFIG=$(brew --prefix llvm@18)/bin/llvm-config`
  - Ubuntu: `apt install clang-18 lld-18 llvm-18 && export LLVM_CONFIG=/usr/lib/llvm-18/bin/llvm-config`
- meson and ninja, only needed to build libmpv

### Build and install

```shell
# download the ps5-payload-dev environment (about 220MB) to ~/.local/opt/ps5-payload-sdk
scripts/ps5/install_sdk.sh

# build libmpv 0.36.0 and wiliwili, the result is located in cmake-build-ps5/ps5
scripts/build_ps5.sh
```

Both scripts read `PS5_PAYLOAD_SDK` from the environment, `scripts/build_ps5.sh` only builds
libmpv when it is not installed in the sysroot yet.

An existing ps5-payload-sdk can also be used, in that case the installation step can be skipped:

```shell
export PS5_PAYLOAD_SDK=/path/to/ps5-payload-sdk
scripts/ps5/build_libmpv.sh  # installs libmpv into $PS5_PAYLOAD_SDK/target/user/homebrew
cmake -B cmake-build-ps5 -DCMAKE_BUILD_TYPE=Release -DPLATFORM_PS5=ON
cmake --build cmake-build-ps5
```

### Deploy

With `websrv` running on the PS5 (replace `ps5` with its IP address if mDNS does not resolve):

```shell
scripts/ps5/deploy.sh -h ps5           # upload ELF + resources and start it
scripts/ps5/deploy.sh -h ps5 -u        # only upload the ELF
```

It uploads the build output to `/data/homebrew/wiliwili/`, the launcher entry point
(`scripts/ps5/homebrew.js`) expects the executable and the resources directory to be there.
An icon (`sce_sys/icon0.png`) is picked up by the PS5 home screen.

The console also needs the mesa libraries of the pacbrew packages (`libGL.so.1`,
`libgallium-26.2.0.so` and `libvulkan.so.1`) in `/user/homebrew/lib` or
`/data/homebrew/lib`, they are the only shared objects the executable depends on.
`scripts/ps5/deploy.sh` and `scripts/ps5/homebrew.js` both put those directories on
`LD_LIBRARY_PATH` when starting the application.

### Notes

- Everything but OpenGL is statically linked into the payload (mpv, FFmpeg, SDL2, curl,
  OpenSSL, libass and the rest), only `libGL.so.1` and the libraries it pulls in have to
  be present on the console, see the deploy section above.
- Video decoding is software only, hardware decoding is not available to payloads.
- libmpv is built with `-Dsdl2=enabled` for audio output and without libplacebo, the video
  frames are rendered by wiliwili into its own framebuffer (`MPV_USE_FB`).
- The PS5 shares the PS4 config directory, settings are stored in `/data/wiliwili/`.
- TLS verification is disabled by default, like on the other consoles. When it is enabled,
  libcurl looks for the CA bundle at `/user/homebrew/etc/ca-bundle.crt`, the path the
  pacbrew curl is built with.

### Status

The port builds and links against [mpv 0.36.0](https://github.com/mpv-player/mpv) (the last
release where libplacebo is optional, needed for the OpenGL render API used here) and the
ps5-payload-dev sysroot; `scripts/ps5/mpv/mpv.patch` contains the three small hunks that make
it compile against FFmpeg 7.

It has not been tested on a real console yet, so treat it as work in progress:

- the OpenGL implementation comes from the pacbrew mesa (zink on top of the PS5 Vulkan
  driver, which is built from Mesa/RADV and submits through the `libSceAgc` interface),
  it is the least proven part of the stack
- SDL2 input, audio output and the video render path all run for the first time on a PS5
- decoding is done by the CPU, 1080P60 playback may need the low quality decoder setting
