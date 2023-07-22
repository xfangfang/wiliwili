<p align="center">
    <img src="resources/svg/cn.xfangfang.wiliwili.svg" alt="logo" height="128" width="128"/>
</p>
<p align="center">
  一个专为手柄用户设计的第三方 <a href="https://www.bilibili.com">B站</a> 客户端
</p>
<p align="center">
<b><a href="https://github.com/xfangfang/wiliwili#特点">特点</a></b>
|
<b><a href="https://github.com/xfangfang/wiliwili#安装">安装</a></b>
|
<b><a href="https://github.com/xfangfang/wiliwili#文档">文档</a></b>
|
<b><a href="https://github.com/xfangfang/wiliwili#开发">开发</a></b>
</p>

- - -

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/xfangfang/wiliwili)](https://github.com/xfangfang/wiliwili/releases)
![GitHub All Releases](https://img.shields.io/github/downloads/xfangfang/wiliwili/total)
![GitHub stars](https://img.shields.io/github/stars/xfangfang/wiliwili?style=flat)
![GitHub forks](https://img.shields.io/github/forks/xfangfang/wiliwili)
[![Crowdin](https://badges.crowdin.net/wiliwili/localized.svg)](https://crowdin.com/project/wiliwili)
![NS](https://img.shields.io/badge/-Nintendo%20Switch-e4000f?style=flat&logo=Nintendo%20Switch)
![MS](https://img.shields.io/badge/-Windows%207+-357ec7?style=flat&logo=Windows)
![mac](https://img.shields.io/badge/-macOS%2010.11+-black?style=flat&logo=Apple)
![Linux](https://img.shields.io/badge/-Linux-lightgrey?style=flat&logo=Linux&logoColor=white)
[![fedora](https://img.shields.io/badge/fedora-copr-blue?logo=fedora)](https://copr.fedorainfracloud.org/coprs/mochaa/wiliwili/)
[![Scoop Version (extras bucket)](https://img.shields.io/scoop/v/wiliwili?bucket=extras)](https://scoop.sh/#/apps?q=wiliwili)
[![aur](https://img.shields.io/aur/version/wiliwili-git?color=blue&logo=archlinux)](https://aur.archlinux.org/packages/wiliwili-git/)
[![nightly.link](https://img.shields.io/badge/nightly.link-%E6%B5%8B%E8%AF%95%E7%89%88-green)](https://nightly.link/xfangfang/wiliwili/workflows/build.yaml/dev)

<br>

# 特点

wiliwili 拥有非常接近官方PC客户端的B站浏览体验  
同时支持**触屏**、**鼠标**、**键盘** 与 **手柄**操控  
无论是电脑还是游戏掌机都能获得全新的使用体验

多语言：简、繁、日、韩、英 ...   
搜索页：热搜 视频 番剧 影视  
筛选页：快速找到想看的影视内容  
动态页：关注的UP主最近视频动态  
直播页：关注的主播与其他系统推荐  
播放页：视频 番剧 电影 纪录片 综艺，支持弹幕与评论  
个人页：扫码登录 历史记录 个人收藏 我的追番 我的追剧  
主题色：拥有深浅两色主题，跟随系统自动切换

<br>

# 安装

### Nintendo Switch

1. 下载 `wiliwili-NintendoSwitch.zip`：[wiliwili releases](https://github.com/xfangfang/wiliwili/releases)
2. 将 wiliwili.nro 放置在**内存卡** `switch` 目录下。
3. 在主页 `按住` R键打开任意游戏进入 hbmenu，在列表中选择 wiliwili 点击打开即可。
4. [可选] 在应用内安装桌面图标，入口：设置/实用工具/使用教程

🔔: 桌面图标会优先尝试打开 `switch/wiliwili.nro`，如果其不存在，则尝试打开 `switch/wiliwili/wiliwili.nro`，如果这两个路径都不存在，则打开 hbmenu 自行选择路径。

### PC

PC客户端支持切换硬件解码、秒开流畅适合老电脑、支持鼠标操控（左键点击 右键返回）

下载对应系统的安装包运行即可：[wiliwili releases](https://github.com/xfangfang/wiliwili/releases)

<details>

<br>
注意：

1. 显卡驱动需要支持 `OpenGL 3.2` 以运行此程序
2. Linux: 对于 Linux 系统只提供 Flatpak 安装包供 Steam Deck 安装，如有其他打包需求欢迎提交完善的打包脚本
3. macOS: 欢迎 macOS 用户提交 Homebrew 安装方式到官方仓库
4. Windows: 不提供 x86 安装包，如有需求可以自行编译

补充信息：

1. 可以从 [Github-Actions](https://github.com/xfangfang/wiliwili/actions/workflows/build.yaml) 下载自动构建的测试版客户端
2. 如果不知道如何从 Github-Actions 下载软件，这里是 [教程](https://xfangfang.github.io/036)
3. 你也可以从 [nightly.link](https://nightly.link/xfangfang/wiliwili/workflows/build.yaml/dev) 免登录下载测试版 （可能不稳定）
4. Windows用户可以通过下载 [debug版](https://github.com/xfangfang/DIY/actions/workflows/wiliwili_win_debug.yml) 来查看log
5. 自定义字体或按键图标：[#38](https://github.com/xfangfang/wiliwili/discussions/38)
6. 目前支持的键盘映射见：[#47](https://github.com/xfangfang/wiliwili/discussions/47)
7. Steam Deck 安装教程：[#41](https://github.com/xfangfang/wiliwili/discussions/41)
8. 从包管理安装 wiliwili: [#92](https://github.com/xfangfang/wiliwili/discussions/92)

</details>

<br>

# 文档

在各位开发者的帮助下，wiliwili 支持了一系列包管理器，同时 wiliwili 还拥有丰富的自定义选项，包括：使用 Anime4K
提升观感，自定义字体及图标等等  
前往 [项目 WIKI](https://github.com/xfangfang/wiliwili/wiki) 查看更多使用技巧

<br>

# TODO list

如果你有其他改进的想法或创意，欢迎在讨论区交流：[Discussions](https://github.com/xfangfang/wiliwili/discussions/categories/ideas)

<details>

- [x] 初步完成底层基础组件、首页各类推荐视频、用户视频播放页
- [x] 微调页面、解决播放器启动速度慢、解决播放页面退出卡顿
- [x] 临时解决异步加载导致的空指针问题（图片异步加载某些情况还会出现问题，待修复）
- [x] 添加番剧/影视播放、添加扫码登录、播放历史、用户收藏夹（收藏夹相关部分工作不稳定）
- [x] 初步添加搜索
- [x] 播放页新增分集与UP主最新投稿
- [x] 完善视频播放页用户评论内容
- [x] 重构图片异步加载逻辑
- [x] 解决收藏夹、搜索页某些情况导致闪退的问题
- [x] 完善搜索页：番剧、影视 转为竖图
- [x] 完善播放页投稿列表：调整结构、自动加载下一页
- [x] 播放页展示合集与推荐
- [x] 添加动态页
- [x] 添加视频检索页
- [x] 完善设置页
- [x] 弹幕相关设置
- [x] 点赞、投币、收藏
- [x] 拖拽调节进度
- [x] 增加单手模式使用一个手柄来控制播放器
- [x] NSP forwarder自动检查多个位置的nro文件，避免无法打开
- [x] 增加设置使首页无法通过返回退出，避免误触
- [x] 使用教程添加未指明的快捷键说明
- [x] 重压摇杆临时快进
- [x] 支持切换按键图标
- [x] 应用内多语言切换
- [x] 重构搜索页面
- [x] 评论@显示不同颜色
- [x] 完善评论图片
- [x] 评论大表情包所在行增加行高
- [x] 支持webp图片
- [ ] 搜索支持搜索用户
- [ ] 长按一键三连
- [ ] 支持个人主页
- [ ] 评论跳转进度
- [ ] 评论跳转搜索
- [ ] 评论下方的更多信息 (up主点赞等内容)
- [ ] 投票评论
- [ ] 互动视频

</details>

<br>

# 反馈问题前要做的事

1. 网络相关的问题附加 `网络诊断截图`，入口：应用内设置/实用工具/网络诊断
2. [Switch用户] 要确保 `大气层`和`系统固件` 更新到 **最新** ，`内存卡`为 **FAT32**
3. [Switch用户] 如果打开应用黑屏时间过长，可以尝试删除内存卡目录 `config/wiliwili` 重新进入
4. 确保 `系统时间`正确、系统`网络设置`正确（主要是DNS）、如果使用了`网络代理`请在反馈前关闭并重新测试
5. 查找有没有其他人出现过类似的问题：[Issues](https://github.com/xfangfang/wiliwili/issues?q=is%3Aissue)
6. **完整且详细地** 描述你的问题，最好附加演示视频、截图。
7. 尝试复现问题，尽力找到BUG出现的规律

<br>

# 贡献

### 软件移植

本应用基于 nanovg 绘制界面，nanovg 底层可移植切换到任意图形库，视频播放部分则使用 MPV + FFMPEG 通过 OpenGL 绘制。
所以按照我的理解 wiliwili 应该可以移植到任何一个内存大于500MB，支持OpenGL（ES）的设备。如果你有想要移植的设备欢迎发一条
issue 讨论。

### 新功能

如果你有想完成的创意，请在开发前发布一个 issue 讨论，避免和别人的创意撞车浪费了时间

### 多语言支持

如果你想为软件添加多语言的翻译支持，或者发现了某些翻译存在问题需要订正，请查看 [#52](https://github.com/xfangfang/wiliwili/issues/52)
了解如何贡献翻译

### 代码分支

主分支 yoga 为保证编译无误的最新代码  
开发分支 dev 为正在开发中的代码，任何新的 PR 都需要向 dev 分支提交

<br>

# 开发

```shell
# 拉取代码
git clone --recursive https://github.com/xfangfang/wiliwili.git
cd wiliwili
```

### PC本地运行

目前 wiliwili 支持运行在 Linux macOS 和 Windows上

<details>

#### macOS

```shell
# macOS: install dependencies
brew install mpv webp

cmake -B build -DPLATFORM_DESKTOP=ON
make -C build wiliwili -j$(sysctl -n hw.ncpu)
```

#### Linux

不同 Linux 的编译过程或依赖可能不同，这里是一份总结：[#89](https://github.com/xfangfang/wiliwili/discussions/89)

欢迎在上面的链接中写出你所使用系统的编译过程供大家参考。

```shell
# Ubuntu: install dependencies
sudo apt install libcurl4-openssl-dev libmpv-dev libwebp-dev

cmake -B build -DPLATFORM_DESKTOP=ON
make -C build wiliwili -j$(nproc)
```

```shell
# 如果你想安装在系统路径，并生成一个桌面图标，请使用如下内容编译
cmake -B build -DPLATFORM_DESKTOP=ON -DINSTALL=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr
make -C build wiliwili -j$(nproc)
sudo make -C build install

# uninstall (run after install)
sudo xargs -a build/install_manifest.txt rm
```

#### Windows

```shell
# Windows: install dependencies (MSYS2 MinGW64)
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make \
  git mingw-w64-x86_64-mpv mingw-w64-x86_64-libwebp

cmake -B build -G "MinGW Makefiles" -DPLATFORM_DESKTOP=ON
mingw32-make -C build wiliwili -j$(nproc)
```

#### SDL2 （测试支持）

由于 SDL2 支持的平台更多，考虑到未来向其他平台移植，所以 wiliwili 也支持切换到 SDL2 环境构建。

⚠️：PC 平台非测试所需请不要使用此方式构建，没有经过太多的测试，且不支持 PC 系统输入法。

```shell
# macOS
brew install sdl2 mpv

cmake -B build -DPLATFORM_DESKTOP=ON -DUSE_SDL2=ON
make -C build wiliwili -j$(sysctl -n hw.ncpu)
```

</details>

### 交叉编译 Switch 可执行文件 (wiliwili.nro)

推荐使用docker构建，本地构建配置环境略微繁琐不过可用来切换底层的ffmpeg或mpv等其他依赖库更灵活地进行调试。

<details>

#### Docker

```shell
docker run --rm -v $(pwd):/data devkitpro/devkita64:20230622 \
  sh -c "/data/scripts/build_switch.sh"
```

#### 本地编译

```shell
# 1. 安装devkitpro环境: https://github.com/devkitPro/pacman/releases

# 2. 安装依赖
sudo dkp-pacman -S switch-glfw switch-libwebp switch-cmake switch-curl devkitA64

# 3. 安装自定义依赖
# devkitpro提供的部分依赖版本过低, 提供的 ffmpeg 无法播放网络视频
# 手动编译方法请参考：scripts/README.md
base_url="https://github.com/xfangfang/wiliwili/releases/download/v0.1.0"
sudo dkp-pacman -U \
    $base_url/switch-libass-0.17.1-1-any.pkg.tar.zst \
    $base_url/switch-ffmpeg-4.4.4-1-any.pkg.tar.zst \
    $base_url/switch-libmpv-0.35.1-1-any.pkg.tar.zst

# 4. 可选：安装依赖库 nspmini：https://github.com/StarDustCFW/nspmini
# (1). 在resources 目录下放置：nsp_forwarder.nsp (如何生成nsp见: scripts/switch-forwarder)
# (2). cmake 构建参数添加 -DBUILTIN_NSP=ON
# 按上述配置后，从相册打开wiliwili时会增加一个安装NSP Forwarder的按钮

# 5. build
cmake -B cmake-build-switch -DPLATFORM_SWITCH=ON
make -C cmake-build-switch wiliwili.nro -j$(nproc)
```

</details>

<br>

# 应用截图

<p align="center">
<img src="docs/images/screenshot-3.jpg" alt="screenshot">
<img src="docs/images/screenshot-4.jpg" alt="screenshot">
</p>

# Acknowledgement

- devkitpro and switchbrew
    - https://github.com/devkitPro/pacman/releases
    - https://github.com/devkitPro/pacman-packages
    - https://github.com/switchbrew/libnx
- natinusala and XITRIX
    - https://github.com/natinusala/borealis
    - https://github.com/XITRIX/borealis
- Cpasjuste and proconsule
    - https://github.com/Cpasjuste/pplay
    - https://github.com/proconsule/nxmp
- https://github.com/libcpr/cpr
- https://github.com/nlohmann/json
- https://github.com/nayuki/QR-Code-generator
- https://github.com/BYVoid/OpenCC
- https://github.com/imageworks/pystring
- https://github.com/sammycage/lunasvg

# Special thanks

- Thanks to Crowdin for supporting [open-source projects](https://crowdin.com/page/open-source-project-setup-request).
- Thanks to JetBrains for providing [Open Source development licenses](https://jb.gg/OpenSourceSupport).

    <img style="width: 70px;" src="https://resources.jetbrains.com/storage/products/company/brand/logos/jb_beam.svg" alt="JetBrains Logo (Main) logo.">
