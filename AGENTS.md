# AGENTS.md — wiliwili Codebase Guide

wiliwili is a cross-platform Bilibili client (Nintendo Switch, PSVita, PS4, PS5, PC) built in C++17. The UI is rendered via **nanovg**, windowing via **GLFW** or **SDL2**, and video playback via **mpv + FFmpeg**.

---

## Architecture Layers (Android-inspired)

| Layer | Directory | Role |
|---|---|---|
| Activity | `wiliwili/include/activity/`, `wiliwili/source/activity/` | Top-level full-screen pages |
| Fragment | `wiliwili/include/fragment/`, `wiliwili/source/fragment/` | Sub-pages composed within Activities |
| Presenter | `wiliwili/include/presenter/` | Async data-fetching logic; Fragments inherit Presenters |
| View | `wiliwili/include/view/` | Custom reusable UI components (MPVCore, RecyclingGrid, VideoView…) |
| API | `wiliwili/include/api/bilibili/` | Bilibili REST/gRPC wrappers via cpr + nlohmann/json |
| Utils | `wiliwili/include/utils/` | Intent (navigation), ProgramConfig (settings), ImageHelper, EventHelper |

The UI framework is a custom fork of **borealis** (`library/borealis/`). All views inherit `brls::Box` or its subclasses.

---

## Key Patterns

### Presenter / async safety
Every Fragment that fetches data inherits a Presenter class. Use the macros in `wiliwili/include/presenter/presenter.h` to guard callbacks against use-after-free:
```cpp
void requestSomething() {
    ASYNC_RETAIN  // captures token, tokenCounter
    bilibili::HTTP::getResultAsync<MyType>(url, params, [ASYNC_TOKEN](auto result) {
        ASYNC_RELEASE  // returns early if the view was destroyed
        brls::Threading::sync([this, result]() {
            // safe to update UI here (main thread)
        });
    });
}
```
Use `CHECK_AND_SET_REQUEST` / `UNSET_REQUEST` to prevent duplicate in-flight requests.

### XML-driven UI
- Layout files live in `resources/xml/` (`activity/`, `fragment/`, `views/`).
- Inflate in constructor: `this->inflateFromXMLRes("xml/fragment/home_recommends.xml");`
- Bind a child by XML id: `BRLS_BIND(RecyclingGrid, recyclingGrid, "home/recommends/recyclingGrid");`
- Custom views must be registered before use: `brls::Application::registerXMLView("RecyclingGrid", RecyclingGrid::create);` (done in `Register::initCustomView()` called from `main.cpp`).
- Valid custom XML element names (all registered in `Register::initCustomView()`):
  - **UI widgets:** `AutoTabFrame`, `RecyclingGrid`, `VideoView`, `VideoProfile`, `QRImage`, `SVGImage`, `TextBox`, `VideoProgressSlider`, `GalleryView`, `CustomButton`, `HintLabel`
  - **App views:** `UserInfoView`, `UpUserSmall`, `VideoComment`, `ButtonClose`, `CheckBox`, `SelectorCell`, `AnimationImage`, `ShareBox`, `DynamicVideoCardView`, `DynamicArticleView`
  - **Fragments:** `HomeTab`, `DynamicTab`, `MineTab`, `HomeRecommends`, `HomeHotsAll`, `HomeHotsHistory`, `HomeHotsWeekly`, `HomeHotsRank`, `HomeHots`, `HomeLive`, `HomeBangumi`, `HomeCinema`, `MineHistory`, `MineLater`, `MineCollection`, `MineBangumi`, `SearchTab`, `SearchOrder`, `SearchVideo`, `SearchCinema`, `SearchBangumi`, `SearchHots`, `SearchHistory`
- i18n key syntax in XML: `@i18n/wiliwili/some/key`; translation files are in `resources/i18n/{en-US,zh-Hans,zh-Hant,ja,ko,it,ja-RYU}/wiliwili.json`.

### Navigation — use `Intent`, never push directly
`wiliwili/include/utils/activity_helper.hpp` defines all navigation entry points:
```cpp
Intent::openBV("BV1Da411Y7U4");         // video by BV id
Intent::openSeasonByEpId(323434);       // bangumi episode
Intent::openLive(1942240);              // live room
Intent::openSearch("keyword");
Intent::openSetting();
Intent::openCollection("2511565362");   // favorites folder
Intent::openPgcFilter("/page/home/pgc/more?type=2&..."); // PGC filter
```

### HTTP API calls
All calls are async via `bilibili::HTTP` (`wiliwili/include/api/bilibili/util/http.hpp`):
```cpp
// Standard GET → parses {"code":0,"data":{...}}
HTTP::getResultAsync<MyResult>(Api::SomeEndpoint, params, callback, error);

// For WBI-signed endpoints (most web-interface APIs since 2023):
HTTP::getResultWithWbiAsync<MyResult>(Api::SomeEndpoint, params, callback, error);

// For app-signed endpoints (pass needSign=true):
HTTP::getResultAsync<MyResult>(url, params, callback, error, /*needSign=*/true);

// POST with typed response:
HTTP::postResultAsync<MyResult>(url, params, payload, callback, error);
```
- API URL constants: `wiliwili/include/api/bilibili/api.h`; JSON result structs: `wiliwili/include/api/bilibili/result/`
- `parseJson` looks for `data` (object/array) first, then `result` (object) as fallback when `code==0`
- **WBI signing** (`getResultWithWbiAsync`): fetches `img_key`+`sub_key` from `Api::Nav`, computes `mixin_key`, appends `wts` + `w_rid`. Keys cached for 1 hour. Required for most `/x/web-interface/` endpoints since 2023.
- HTTP defaults: `User-Agent: wiliwili`, `Referer: https://www.bilibili.com/client`, timeout 10000ms. Proxy and TLS verify are configurable via `SettingItem::HTTP_PROXY*` / `SettingItem::TLS_VERIFY`.

### RecyclingGrid (custom recycler view)
Implement `RecyclingGridDataSource` and call `recyclingGrid->setDataSource(...)`. Example from `player_activity.cpp`:
```cpp
class DataSourceFoo : public RecyclingGridDataSource {
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        auto* item = (MyCell*)recycler->dequeueReusableCell("Cell");
        item->setData(list[index]);
        return item;
    }
    size_t getItemCount() override { return list.size(); }
    void onItemSelected(RecyclingGrid* recycler, size_t index) override { /* handle tap */ }
    void clearData() override { list.clear(); }
};
```
`recyclingGrid->onNextPage([]{...})` triggers when the user scrolls to the end (infinite scroll).

### Image loading
Always append a platform-aware suffix:
```cpp
ImageHelper::with(imageView)->load(url + ImageHelper::h_ext);    // horizontal thumbnail (@672w_378h on PC)
ImageHelper::with(imageView)->load(url + ImageHelper::v_ext);    // vertical thumbnail (@312w_420h on PC)
ImageHelper::with(imageView)->load(url + ImageHelper::face_ext); // avatar (@96w_96h on PC)
```
`IMAGE_EXT` resolves to `.webp` when `USE_WEBP` is defined, else `.jpg`. PSV uses smaller sizes (e.g. `@256w_144h` for horizontal).

### Global event bus
Defined in `wiliwili/include/utils/event_helper.hpp`:
- `MPV_E` — player state (`MPV_LOADED`, `MPV_PAUSE`, `MPV_RESUME`, `UPDATE_PROGRESS`, `END_OF_FILE`, `RESET`, `RESTART`, `CACHE_SPEED_CHANGE`, `VIDEO_SPEED_CHANGE`, …)
- `APP_E` — app-wide custom events (string key + void*)
- `SEARCH_E` — search page events

Subscribe: `MPV_E->subscribe([](MpvEventEnum e){ ... });`  Fire: `MPV_E->fire(MPV_PAUSE);`

### Settings / ProgramConfig
`ProgramConfig::instance()` is a singleton that must be initialized with `init()` **before** `brls::Application::init()`. All settings use the `SettingItem` enum (defined in `wiliwili/include/utils/config_helper.hpp`). Notable items:
- `PLAYER_HWDEC` / `PLAYER_HWDEC_CUSTOM` — hardware decode method
- `APP_RESOURCES` — active custom theme ID (requires restart to apply)
- `KEYMAP` — button icon set: `"xbox"` (PC default), `"ps"`, `"keyboard"`
- `HTTP_PROXY`, `HTTP_PROXY_STATUS`, `TLS_VERIFY` — network settings
- `SHORTCUT_*` — rebindable keyboard shortcuts (modifier-key strings, e.g. `"ctrl-r"`)

### Custom Theme / Layout System
Themes live in `{configDir}/theme/{id}/` and can override any file under `resources/`.
- A theme directory must contain `resources_meta.json`: `{"name":"…","desc":"…","version":"…","author":"…"}`
- Discovered by `ProgramConfig::loadCustomThemes()` at startup; theme ID stored in `SettingItem::APP_RESOURCES`
- Switching themes calls `DialogHelper::quitApp()` — restart is required
- Reference theme repo: https://github.com/xfangfang/wiliwili_theme

### Custom Fonts & Icons
Drop these files into the config directory to override built-in assets:

| Filename | Purpose |
|---|---|
| `font.ttf` | Main UI font |
| `icon.ttf` | Button icon font |
| `emoji.ttf` | Emoji font |
| `danmaku.ttf` | Danmaku overlay font |
| `gamecontrollerdb.txt` | SDL gamepad database (desktop only) |

Built-in keymap fonts in `resources/font/`: `keymap_xbox.ttf`, `keymap_ps.ttf`, `keymap_keyboard.ttf`.

### Anime4K / Shader System
`ShaderHelper` (singleton, `wiliwili/include/utils/shader_helper.hpp`) manages profiles loaded from `{configDir}/shader.json`:
```json
{
  "profiles": [
    {
      "name": "Anime4K Mode A",
      "shaders": ["/path/to/Anime4K_Clamp_Highlights.glsl"],
      "settings": [["set", "scaler", "ewa_lanczossharp"]]
    }
  ],
  "animeList": [{ "anime": 28223043, "profile": "Anime4K Mode A" }]
}
```
- `settings` entries: `["set","key","val"]`, `["change-list","key","op","val"]`, or `["run","cmd"]`.
- Apply/clear with `ShaderHelper::instance().setShader(index)` / `clearShader()`.

### MPV Rendering Modes

| Mode | Trigger | Notes                                                  |
|---|---|--------------------------------------------------------|
| Framebuffer (default) | `MPV_USE_FB` (auto) | GL 3.2+ / GLES 2.0+; best performance; PS5            |
| No framebuffer | `-DMPV_NO_FB=ON` | MPV draws fullscreen, UI is overlaid; PS4, PSV-GL, GL2 |
| Software render | `-DMPV_SW_RENDER=ON` | CPU only; for UWP/D3D12 porting                        |
| deko3d | `BOREALIS_USE_DEKO3D` | Switch native; 4K@60 with hardware decode              |
| GXM | `BOREALIS_USE_GXM` | PSVita native                                          |
| D3D11 | `BOREALIS_USE_D3D11` | Windows Native/UWP                                     |

Default hardware decode: Switch/GXM → `"auto"`, PSV-GL → `"vita-copy"`, PS4 → `"no"`, PS5 → `"no"` (payloads have no hardware decode), Desktop → `"auto-safe"`.

### Video Quality Codes

| Code | Quality | Notes |
|---|---|---|
| 127 | 8K | |
| 120 | 4K | |
| 116 | 1080P60 | Default on non-PSV |
| 80 | 1080P | |
| 64 | 720P | Default PSV GXM |
| 32 | 480P | Default PSV OpenGL |
| 16 | 360P | Default when logged out |

---

## Config Directory Locations

| Platform | Path |
|---|---|
| Nintendo Switch | `/config/wiliwili/` |
| PS4 | `/data/wiliwili/` |
| PS5 | `/data/wiliwili/` |
| PSVita | `ux0:/data/wiliwili/` |
| macOS (release) | `~/Library/Application Support/wiliwili/` |
| Linux (release) | `$XDG_CONFIG_HOME/wiliwili/` or `~/.config/wiliwili/` |
| Windows (release) | `%LOCALAPPDATA%\xfangfang\wiliwili\` |
| Any platform (debug build) | `./config/wiliwili/` (next to binary) |

Main config file: `{configDir}/wiliwili_config.json` (cookie, refreshToken, all SettingItem values, GA client ID, search history).

---

## Build Commands

### Desktop (macOS)
```sh
brew install mpv webp
cmake -B build -DPLATFORM_DESKTOP=ON
make -C build wiliwili -j$(sysctl -n hw.ncpu)
# macOS app bundle:
make -C build wiliwili.app
```

### Desktop (Linux/Ubuntu)
```sh
sudo apt install libssl-dev libmpv-dev libwebp-dev
cmake -B build -DPLATFORM_DESKTOP=ON
make -C build wiliwili -j$(nproc)
# System install with .desktop entry:
cmake -B build -DPLATFORM_DESKTOP=ON -DINSTALL=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
make -C build wiliwili -j$(nproc) && sudo make -C build install
```

### Nintendo Switch (Docker, recommended)
```sh
docker run --rm -v $(pwd):/data devkitpro/devkita64:20251117 bash -c "/data/scripts/build_switch.sh"
# deko3d variant (4K@60 hardware decode):
# see scripts/build_switch_deko3d.sh
```
Switch requires custom ffmpeg/mpv (devkitpro defaults don't support network streams):
```sh
base_url="https://github.com/xfangfang/wiliwili/releases/download/v0.1.0"
sudo dkp-pacman -U $base_url/switch-ffmpeg-7.1-1-any.pkg.tar.zst \
                   $base_url/switch-libmpv-0.36.0-3-any.pkg.tar.zst
cmake -B cmake-build-switch -DPLATFORM_SWITCH=ON
make -C cmake-build-switch wiliwili.nro -j$(nproc)
```

### PSVita / PS4 (Docker)
```sh
# PSVita GXM (recommended):
docker run --rm -v $(pwd):/src/ xfangfang/wiliwili_psv_builder:latest-gxm \
    "cmake -B cmake-build-psv -G Ninja -DPLATFORM_PSV=ON -DUSE_GXM=ON \
     -DUSE_SYSTEM_CURL=ON -DUSE_VITA_SHARK=OFF -DCMAKE_BUILD_TYPE=Release && \
     cmake --build cmake-build-psv"
# PS4:
docker run --rm -v $(pwd):/src/ xfangfang/wiliwili_ps4_builder:latest \
    "cmake -B cmake-build-ps4 -DPLATFORM_PS4=ON -DMPV_NO_FB=ON \
     -DUSE_SYSTEM_CPR=ON && make -C cmake-build-ps4 -j$(nproc)"
```

### PS5 (ps5-payload-dev, no Docker)
```sh
# requires LLVM 18: brew install llvm@18 (macOS) / apt install llvm-18 clang-18 lld-18
export LLVM_CONFIG=$(brew --prefix llvm@18)/bin/llvm-config

scripts/ps5/install_sdk.sh          # downloads the ~220MB ps5-payload-dev sysroot
scripts/build_ps5.sh                # builds libmpv (once) + wiliwili
# native title build (PPSA99099):
# see scripts/ps5/title/README.md for full native title pipeline
scripts/ps5/title/deploy.sh -h <ps5-ip>   # upload PPSA99099 to /data/homebrew/PPSA99099
```
`PS5_PAYLOAD_SDK` points at an existing SDK to skip the download. wiliwili runs as a native
homebrew title under `/data/homebrew/PPSA99099/` on a jailbroken console.

#### PS5 Hardware Decoder & Vulkan Pipeline Notes
- **Direct Memory Permissions**: Hardware decoder frame buffers (`ps5->frames`) MUST be mapped with `PROT_CPU_RW | PROT_GPU_RW` (`0x33`). Using `PROT_CHIP_POOL` (`0x32`, write-only for CPU) causes CPU reads (`memcpy`) to trigger kernel exception `0xa0020328 (SYSTEM_XO_VIOLATION)` on Prospero.
- **Surface Stride & Height**: `sceVideodec2` hardware decoder surfaces align height to 64 pixels (`(height + 63) & ~63`). Semi-planar NV12 Chroma (UV) begins at offset `pitch * surface_height`, NOT display height.
- **MPV Software Rendering (`MPV_SW_RENDER`)**: Under `BOREALIS_USE_VULKAN`, MPV renders to a CPU buffer (`pixels`) aligned to 64 bytes via `posix_memalign(..., 64, size + 64)`. Rendering surface is capped at 1080p to prevent 4K memory bandwidth bottlenecks; GPU shaders scale it to 4K output without CPU cost.
- **Frame Boundaries**: `nvgUpdateImage` must only be invoked within the rendering loop (`draw()`), never out-of-band in `reset()` or `setUrl()`.

#### PS5 Physical HDR10 & Dolby Vision / HLG Pipeline Notes
- **Physical HDR10 Output**:
  - The swapchain decides, not a runtime switch. `MPVCore::HDR_OUTPUT` (config key `ps5_hdr_output`, default off) is copied into `brls::VulkanContext::HDR_SWAPCHAIN` in `main.cpp` before `brls::Application::init()`; the driver then asks for `brls::VulkanContext::HDR_SWAPCHAIN_FORMAT` = `VK_FORMAT_A2R10G10B10_UNORM_PACK32` and falls back to `B8G8R8A8_UNORM` when the surface or the console refuses it.
  - **Channel order**: VideoOut's `Bgr10A2` scans **blue in the low ten bits**. In Vulkan that is `A2R10G10B10_UNORM_PACK32` (Mesa: `PIPE_FORMAT_B10G10R10A2`); `A2B10G10R10_UNORM_PACK32` puts red there and shows the whole screen with red and blue swapped (the theme's cyan focus ring turns gold, video turns purple). The `0x22000000` bits of the pixel format are the same RGB-vs-BGR choice: `0x8000000022000000` is Rgba8Srgb, `0x8000000000000000` is Bgra8Srgb. (Checked against sainsaji/EVO-PLAYER-PS5, `projects/evoplayer/media/src/evo_agc_runtime.c`.)
  - Mesa's `videoout_register_locked()` (`wsi_common_videoout.c`) registers those framebuffers with `PS5_VIDEO_OUT_PIXEL_FORMAT_BGR10A2_BT2100_PQ` (`0x8100070400000000`), 8bit sets with `BGR8A8_SDR` (`0x8000000000000000`). A set is keyed by size **and** format. VideoOut only honors the format given **at registration**.
  - `sceVideoOutSubmitChangeBufferAttribute2` retypes a registered set in place and is applied at the next flip (that is how EVO-PLAYER toggles HDR), but it cannot fix a set whose *pixels* are still 8bit, and `sceVideoOutGetOutputStatus` reports `dynamic_range=2` on a console set to always-HDR whether the retype happened or not — it is not a way to confirm a switch.
  - **Never pack PQ into the 8bit frame buffer.** nanovg paints with premultiplied alpha blending (`ONE, ONE_MINUS_SRC_ALPHA`): any partially transparent draw (text anti-aliasing, rounded corners, danmaku, menu overlays) mixes packed bits with packed bits and the whole screen fills with garbage — that is the yellow UI and purple video this port used to show.
  - In `fill_base.frag` (`hdrOutput == 1`) the shader writes PQ floats straight into the 10bit target (`outColor = vec4(finalPq, alpha)`); UI and text are mapped to 203 Nits (Graphics White) BT.2020 PQ per **ITU-R BT.2408**.
  - Flag plumbing: `brls_nanovg_hdr_output` is a single global (declared in `nanovg.h`, defined in `nanovg.c`) because `nanovg_vk.h` is compiled into more than one translation unit — a `static` in that header gave every one of them its own copy, so the setter and the uniform filler disagreed. `VulkanContext::createSwapchainWithFormat()` sets it with `nvgSetHdrOutput(nullptr, 0|1)`, and `frag->hdrOutput` reads it back through `vknvg_get_hdr_output()`.
- **Video texture types (`nvgImagePattern` → `texType`)**: the value is carried in `NVGpaint.radius` and decoded in `nanovg_vk.h`:
  - `radius = 3.0f` → `texType = 6`: HLG (BT.2100 ARIB-STD-B67) → PQ / SDR.
  - `radius = 2.0f` → `texType = 5`: Dolby Vision Profile 5 (ICtCp reshaping).
  - `radius = 1.0f` → `texType = 4`: generic HDR10 PQ (BT.2020 → BT.709 tone mapping).
  - `NVG_IMAGE_DOVI_PROFILE5` / `NVG_IMAGE_HDR_TONEMAP` image flags select the same paths when the radius is left at 0.
- **Dolby Vision Profile 8.4 vs Profile 5 Distinction**:
  - **Profile 8.4 (iPhone & mobile user uploads)**:
    - Base layer is standard **HLG (ARIB-STD-B67) BT.2020 YUV** (stream `dv_profile=8`, `compatibility id: 4`).
    - Flagged in `mpv_core.cpp` via `isHlg` (`gamma == "arib-std-b67"` / `gamma == "hlg"`), mapping to `videoPaint.radius = 3.0f` and `texType == 6`.
    - Shader applies **ITU-R BT.2100 HLG EOTF** (`hlg_to_nits` -> 1000 Nits nominal peak -> `nits_to_pq_vec3`). Must **never** feed raw HLG directly into PQ (causes severe oversaturation / blowout) or ICtCp reshaping (causes green tint).
  - **Profile 5 (Official streaming & cinema)**:
    - Base layer is proprietary **IPT-PQ / ICtCp** (`dv_profile=5`, Full Range 0~255).
    - Flagged via `isDoviProfile5`, mapping to `videoPaint.radius = 2.0f` and `texType == 5`. Note: `dv_profile` itself is **not** read — the check is heuristic (see below) because the container metadata is not exposed by every Bilibili stream.
    - **swscale color conversion** (the critical detail): mpv has no libplacebo; it software-decodes DV P5 as `yuv420p10le` and lets swscale convert to 8-bit RGBA. swscale sees **bt2020nc limited-range** YUV and applies: (a) limited→full luma expansion `(Y − 16/255) × 255/219`, (b) limited→full chroma expansion `(C − 128/255) × 255/224`, (c) the **BT.2020-nc forward matrix**. The shader must exactly invert this chain.
    - **Shader inverse** (`dolby_vision_profile5_to_nits()` in `fill_base.frag`):
      1. Apply `M_rgb_to_yuv_2020nc` (BT.2020-nc matrix) to recover YCbCr from RGBA.
      2. Re-compress luma: `I_true = yuv.x × (219/255) + (16/255)` — undoes the limited→full expansion.
      3. Re-compress chroma: `Ct = yuv.y × (224/255)`, `Cp = yuv.z × (224/255)`.
      4. ICtCp → LMS′ (PQ-encoded) via the standard 3×3 inverse.
      5. LMS′ → linear LMS via PQ EOTF.
      6. Linear LMS → BT.2020 via libplacebo `dovi_lms2rgb`.
      7. `max(lin2020, vec3(0.0))` to clip out-of-gamut negatives (lifting would make pure black grey).
    - **Do NOT** use `smoothstep` shadow chroma damping — that was compensating for the old wrong BT.709 matrix and has been removed.
    - **Do NOT** apply `vf=@wiliwili-dv:format=colorlevels=full:colormatrix=bt.709` filter — it was tested on the real console and made the picture worse (commit `817b5013` added, `6ad969c4` reverted). The shader's limited→full inversion handles this correctly.
    - After any edit to `fill_base.frag`, regenerate **both** SPIR-V `.inc` files (see glsl_to_inc.py note in Key Files).
    - Reconstructed linear BT.2020 is tone-mapped to PQ (HDR output) or SDR via BT.2390 EETF. Out-of-gamut negatives are clipped. Known limitation: 8-bit RGBA loses bottom ~6% of `I` (< 0.1 nit, invisible) and clamps above 940/1023 (> ~4775 nits, rare in practice).
  - **Heuristic detection (do not replace with `dv_profile` alone)**: `isDoviProfile5` = `colormatrix` contains `ipt`, or `codec` contains `dv`, or `pixelFormat` contains `dovi`, or `currentQuality == 126`; `isHlg` wins over it (Profile 8.4 shares the Profile 5 codec path but is plain HLG). `isHdr` additionally accepts `p010`, any 10-bit pixel format, `gamma == pq/smpte2084` and `primaries == bt.2020`.


### Useful CMake flags
| Flag | Purpose |
|---|---|
| `-DUSE_SDL2=ON` | Use SDL2 instead of GLFW |
| `-DMPV_NO_FB=ON` | No framebuffer (PS4, PSV GL, GL 2.0) |
| `-DMPV_SW_RENDER=ON` | CPU software rendering |
| `-DMPV_BUNDLE_DLL=ON` | Bundle mpv.dll into exe (Windows + USE_LIBROMFS=ON) |
| `-DDISABLE_OPENCC=ON` | Skip Chinese conversion library |
| `-DDISABLE_WEBP=ON` | Skip WebP support |
| `-DINSTALL=ON` | Linux system install with desktop entry |
| `-DDEBUG_SANITIZER=ON` | Enable ASan/UBSan (debug only) |
| `-DUSE_LIBROMFS=ON` | Embed resources into binary |
| `-DBUILTIN_NSP=ON` | Embed NSP forwarder (Switch only) |
| `-DAPP_PLATFORM_CUSTOM_LIBS=ON` | Manual deps: `APP_PLATFORM_INCLUDE` + `APP_PLATFORM_LINK_OPTION` |

---

## Debugging

- Run with `-d` for debug log level, `-v` for borealis visual debug overlay, `-t` for mpv terminal output, `-o <file>` to write log to file.
- To test a specific page without navigating the UI, uncomment the relevant `Intent::open*()` line in `main.cpp` (many test BV IDs are already commented there).
- `ProgramConfig::instance().init()` loads cookies and all settings; it must run before `brls::Application::init()`.
- In-app network diagnostics: Settings → Tools → Network Diagnostics (`fragment/setting_network`). Shows API connectivity, system/server time diff, WiFi state, IP, DNS.
- On Switch: if the app is black-screen on startup, delete `/config/wiliwili/` from SD card and retry.

---

## Platform Macros

| Macro | Platform |
|---|---|
| `__SWITCH__` | Nintendo Switch |
| `__PSV__` | PlayStation Vita |
| `PS4` | PlayStation 4 |
| `PS5` / `__PS5__` | PlayStation 5 (set by the prospero toolchain) |
| `BOREALIS_USE_OPENGL` / `BOREALIS_USE_DEKO3D` / `BOREALIS_USE_D3D11` / `BOREALIS_USE_GXM` | Rendering backend |
| `MPV_USE_FB` / `MPV_NO_FB` | Framebuffer mode (auto-derived from above) |
| `USE_WEBP` | WebP image decoding enabled |

---

## Key Files at a Glance

| File | Description |
|---|---|
| `wiliwili/source/main.cpp` | Entry point; registers views, launches activity |
| `wiliwili/include/utils/activity_helper.hpp` | `Intent` — all navigation |
| `wiliwili/include/utils/config_helper.hpp` | `ProgramConfig` singleton, `SettingItem` enum |
| `wiliwili/include/utils/event_helper.hpp` | Global event buses (`MPV_E`, `APP_E`, `SEARCH_E`) |
| `wiliwili/include/presenter/presenter.h` | `ASYNC_RETAIN`/`ASYNC_RELEASE` macros |
| `wiliwili/include/api/bilibili/util/http.hpp` | `HTTP::getResultAsync`, `getResultWithWbiAsync` |
| `wiliwili/include/api/bilibili/util/wbi.hpp` | WBI signing implementation |
| `wiliwili/include/api/bilibili/api.h` | All Bilibili API URL constants |
| `wiliwili/include/view/recycling_grid.hpp` | Recycler list widget |
| `wiliwili/include/utils/image_helper.hpp` | Async image loading + platform-sized URL suffixes |
| `wiliwili/include/utils/shader_helper.hpp` | Anime4K / mpv shader profile management |
| `wiliwili/include/view/mpv_core.hpp` | MPV singleton, playback control, rendering backend |
| `wiliwili/include/view/danmaku_core.hpp` | Danmaku (bullet comment) overlay rendering |
| `wiliwili/include/api/live/danmaku_live.hpp` | Live room WebSocket danmaku via mongoose |
| `resources/xml/` | All UI layout files |
| `resources/i18n/` | Translations (en-US, zh-Hans, zh-Hant, ja, ko, it, ja-RYU) |
| `scripts/` | Build scripts for each platform; `README.md` covers custom ffmpeg/mpv for Switch |

