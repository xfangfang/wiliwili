# PS5

wiliwili runs on jailbroken PS5s as a homebrew application of the
[ps5-payload-dev](https://github.com/ps5-payload-dev) ecosystem: the build output is a directory
under `/data/homebrew/` holding the payload (`eboot.elf`), an icon, a launcher entry point and
the libraries it links against. It is started by
[websrv](https://github.com/ps5-payload-dev/websrv) (port 8080, which also provides `ftpsrv` on
port 2121) or by the homebrew launcher of the console.

The cross compilation environment comes from
[ps5-payload-dev/pacbrew-repo](https://github.com/ps5-payload-dev/pacbrew-repo), it contains the
prospero toolchain wrappers, the SDK sysroot, and prebuilt SDL2, FFmpeg, Mesa (OpenGL via zink),
libass, OpenSSL, curl and friends.

### Native title

`scripts/ps5/title/` builds the same application as a **native title** instead of a payload: a
folder below `/data/homebrew/<TITLE_ID>/` that holds a signed `eboot.bin` and is shown on the home
screen by a folder-title loader such as ShadowMountPlus. Everything is linked into that one image,
the OpenGL implementation comes from
[ps5-opengl](https://github.com/blackbearreloaded/ps5-opengl), and no payload has to run on the
console. See [title/README.md](title/README.md).

### Requirements

- LLVM 18 (the toolchain wrappers call the mode-matched host `clang`/`lld`)
  - macOS: `brew install llvm@18 && export LLVM_CONFIG=$(brew --prefix llvm@18)/bin/llvm-config`
  - Ubuntu: `apt install clang-18 lld-18 llvm-18 && export LLVM_CONFIG=/usr/lib/llvm-18/bin/llvm-config`
- meson and ninja, only needed to build libmpv

### Build and install

```shell
# download the ps5-payload-dev environment (about 220MB) to ~/.local/opt/ps5-payload-sdk
scripts/ps5/install_sdk.sh

# build libmpv 0.36.0, wiliwili and the home screen installer, the result is located in
# cmake-build-ps5/ps5
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
scripts/ps5/deploy.sh -h ps5           # upload ELF + libraries + resources and start it
scripts/ps5/deploy.sh -h ps5 -u        # only upload the ELF and the libraries
scripts/ps5/deploy.sh -h ps5 -L        # upload the libraries even when the console has them
scripts/ps5/deploy.sh -h ps5 -- -d     # pass arguments to the application
```

It uploads the build output to `/data/homebrew/wiliwili/`, the layout the launcher of the
ecosystem expects:

```text
eboot.elf        the payload, the name websrv and the launcher start it by
homebrew.js      the launcher entry point: name, icon and launch environment
index.html       the web app page the home screen entry opens (see below)
sce_sys/icon0.png
lib/             libGL.so.1, libgallium-26.2.0.so, libvulkan.so.1
resources/
```

The launcher entry (`scripts/ps5/homebrew.js`) starts `eboot.elf` with
`LD_LIBRARY_PATH=<application directory>/lib:/data/homebrew/lib:/user/homebrew/lib`, the
convention every homebrew of the ecosystem uses for its libraries.

The executable links against `libGL.so.1` of the pacbrew mesa, which loads
`libgallium-26.2.0.so` and `libvulkan.so.1` in turn. Those three are the only shared objects it
needs, and the console firmware does not provide them: the build copies them out of
`$PS5_PAYLOAD_SDK/target/user/homebrew/lib` into `cmake-build-ps5/ps5/lib/` (the SDK ships
`libGL.so.1` and `libvulkan.so.1` as symlinks, the console needs the files themselves), and
`scripts/ps5/deploy.sh` uploads them to `/data/homebrew/wiliwili/lib/`. A console which already
has the libraries in `/user/homebrew/lib` or `/data/homebrew/lib` works as well, the launcher
entry looks there too. Without any of them the loader stops at `libGL.so.1` and the application
never gets to its first line, which from the home screen looks like a black flash and an
immediate return.

### Launching

- in a browser, `http://<ps5>:8080/` lists the applications in `/data/homebrew/` (also the
  setup pages: `/elfldr` to start a single payload, `/fs/` to browse the filesystem). wiliwili
  appears there with its icon, pointing a phone or the Mac at that page starts the application
  on the console
- `scripts/ps5/deploy.sh -h ps5` uploads and starts it in one step, the development loop
- the home screen entry below, which starts it from the console without a browser tab

### Home screen entry

The first time websrv runs it installs an entry named "Homebrew Launcher" (title `FAKE00000`) on
the home screen: a deeplink to `http://127.0.0.1:8080/`, the page above. wiliwili can have its
own entry next to it:

```shell
scripts/ps5/install_tile.sh -h ps5                 # adds it, run once
scripts/ps5/install_tile.sh -h ps5 -- --uninstall  # removes it again
```

The entry (title `BREW99099`, built by `scripts/ps5/tile/install_tile.c`) is a deeplink to the
`index.html` of the application directory, which websrv serves from the console's filesystem
(`/fs/data/homebrew/wiliwili/index.html`). That page starts `eboot.elf` with the launcher
environment and shows what the application writes to stdout, useful when something goes wrong
before the first frame. The installer writes the title parameters to `/user/app/<title id>/`
and calls `sceAppInstUtilAppInstallTitleDir`, the same call the websrv and launchpad payloads
use for their entries.

Using the entry needs a running websrv (it serves the page and the payload), it does not need
package support in kstuff. The URL defaults to `http://127.0.0.1:8080`, pass another one when
websrv listens elsewhere: `scripts/ps5/install_tile.sh -h ps5 -- http://127.0.0.1:8081`. The
installer expects the application in `/data/homebrew/wiliwili/`, deploy it first.

### Packing into a compressed PFS image (.ffpfsc)

`scripts/ps5/package_ffpfs.sh` packages the native title folder into a compressed `.ffpfsc` container (PFS v2 image with zlib level 9 compression) using [ps5-dump-forge](https://github.com/quer3q/ps5-dump-forge):

```shell
scripts/ps5/package_ffpfs.sh               # packages dist/PPSA99099 into dist/wiliwili-PS5-<version>.ffpfsc and dist/PPSA99099.ffpfsc
scripts/ps5/package_ffpfs.sh -i <folder>   # packages a custom title folder
```

The script automatically downloads `ps5-dump-forge` if it is not present in `$PATH` or `$PS5_DUMP_FORGE`. The resulting `.ffpfsc` file can be copied directly to `/data/homebrew/` or a USB drive for [ShadowMountPlus](https://github.com/drakmor/ShadowMountPlus) to mount automatically, saving over 55% storage space (~62MB) and eliminating the need to transfer hundreds of individual files.

### Packing into a package (experimental)

`scripts/ps5/package_fpkg.sh` stages the build output as an application folder and packs it into
a debug fPKG, the package format the installer of the console accepts:

```shell
scripts/ps5/package_fpkg.sh     # writes cmake-build-ps5/fpkg/<content id>.pkg
scripts/ps5/package_fpkg.sh -s  # stage the application folder only
```

The packer is [LibProsperoPkg](https://github.com/drakmor/LibProsperoPkg) (GPL-3.0), used through
the command line tool of [ps5-ultrapack](https://github.com/knutwurst/ps5-ultrapack), the only
free tool known to build packages that install and launch (it generates the `icon0.dds` the
console needs to start an application and fake-signs the eboot). Put it in `$PATH` as
`ffpfsc-pkg-tool` or point `PS5_FPKG_TOOL` at it, the script prints the download and build
commands otherwise.

The staged folder is a normal PS5 application folder (`eboot.bin`, `sce_sys/` and the libraries of
the build output), the script only fills in `param.json` and the icon from
`scripts/ps5/fpkg/param.json` and `scripts/ps5/sce_sys/icon0.png`.

The route is not needed for the launcher and home screen entries above and it stays
experimental, it has more open ends than they have:

- a package is started by the system, not by the payload loader, so the process gets neither the
  environment (`LD_LIBRARY_PATH`) nor the patched loader of `scripts/ps5/homebrew.js`. The
  executable still links against `libGL.so.1`, and the pacbrew shared libraries carry a bare
  soname, so the loader has to find them in a directory it searches; the package carries the
  three files next to `eboot.bin` as a first attempt, and the SDK's `hello_so` sample gives its
  libraries absolute sonames (`-Wl,-soname=/data/libtest.so`) as the other way out
- the resources stay in `/data/homebrew/wiliwili/resources/`, the compiled in path does not
  change with the way the application is started
- installing and launching a package needs kstuff with fself and fPKG support. The fPKG support
  came into kstuff-lite in September 2026 (PR #76, "1.12 FPKG SUPPORT", followed by the package
  mount hooks and the SELF lazy loading changes), the published releases predate it, the newest
  build with it was an artifact of the project's CI. On the test console (firmware 11.60, kstuff
  loaded manually, both the release and a CI build with the support) the launch failed with
  `CE-100008-9`, the error a console reports for every title when kstuff is not (or not anymore)
  active, and for packages kstuff cannot fake-sign. It appears before any of the application's
  own code runs

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

It runs on a real console for the first time here, so treat it as work in progress:

- the OpenGL implementation comes from the pacbrew mesa (zink on top of the PS5 Vulkan
  driver, which is built from Mesa/RADV and submits through the `libSceAgc` interface),
  it is the least proven part of the stack
- SDL2 input, audio output and the video render path all run for the first time on a PS5
- decoding is done by the CPU, 1080P60 playback may need the low quality decoder setting
