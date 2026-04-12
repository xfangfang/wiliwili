# AGENTS.md — wiliwili Codebase Guide

wiliwili is a cross-platform Bilibili client (Nintendo Switch, PSVita, PS4, PC) built in C++17. The UI is rendered via **nanovg**, windowing via **GLFW** or **SDL2**, and video playback via **mpv + FFmpeg**.

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
| Framebuffer (default) | `MPV_USE_FB` (auto) | GL 3.2+ / GLES 2.0+; best performance                  |
| No framebuffer | `-DMPV_NO_FB=ON` | MPV draws fullscreen, UI is overlaid; PS4, PSV-GL, GL2 |
| Software render | `-DMPV_SW_RENDER=ON` | CPU only; for UWP/D3D12 porting                        |
| deko3d | `BOREALIS_USE_DEKO3D` | Switch native; 4K@60 with hardware decode              |
| GXM | `BOREALIS_USE_GXM` | PSVita native                                          |
| D3D11 | `BOREALIS_USE_D3D11` | Windows Native/UWP                                     |

Default hardware decode: Switch/GXM → `"auto"`, PSV-GL → `"vita-copy"`, PS4 → `"no"`, Desktop → `"auto-safe"`.

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

