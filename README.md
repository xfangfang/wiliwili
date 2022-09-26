‼️：仅在最新 `大气层` + 最新 `固件` + `FAT32` 内存卡测试，其他组合出现的问题不进行处理。

<br>

# wiliwili

一个第三方 Nintendo Switch [B站](https://www.bilibili.com)客户端

<img src="resources/icon/bilibili.png" alt="icon" height="128" width="128" align="left">

wiliwili 拥有非常接近官方PC客户端的B站浏览体验，  
同时支持触屏与手柄按键操控，  
让你的switch瞬间变身机顶盒与掌上平板。
<br>

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/xfangfang/wiliwili)](https://github.com/xfangfang/wiliwili/releases) ![GitHub All Releases](https://img.shields.io/github/downloads/xfangfang/wiliwili/total) ![GitHub stars](https://img.shields.io/github/stars/xfangfang/wiliwili?style=flat) ![GitHub forks](https://img.shields.io/github/forks/xfangfang/wiliwili)

<br>

# 支持特性

主题色：支持根据switch主题自动切换深浅  
播放页：视频 番剧 影视 综艺，同时支持弹幕  
个人页：扫码登录 历史记录 个人收藏  
搜索页：视频 番剧 影视  
动态页：关注的UP主更新的视频  
直播页：不是十分稳定的支持 (也可以说是十分不稳定)  
分类检索：快速找到想看的电影和番剧  
首页推荐：完美复制原版pc端布局  

<br>

# 安装流程

0. 下载安装包：[wiliwili releases](https://github.com/xfangfang/wiliwili/releases)
1. 将wiliwli.nro放置在内存卡路径： switch/wiliwili.nro 
2. 在主页 `按住` R键打开任意游戏进入HBMenu，在列表中选择wiliwili点击打开即可。
3. [可选] 在应用内安装桌面图标，入口：设置/实用工具/使用教程

<br>

# TODO list

- [x] 初步完成底层基础组件、首页各类推荐视频、用户视频播放页
- [x] 微调页面、解决播放器启动速度慢、解决播放页面退出卡顿
- [x] 临时解决异步加载导致的空指针问题（图片异步加载某些情况还会出现问题，待修复）
- [x] 添加番剧/影视播放、添加扫码登录、播放历史、用户收藏夹（收藏夹相关部分工作不稳定）
- [x] 初步添加搜索
- [x] 播放页新增分集与UP主最新投稿
- [ ] 完善视频播放页用户评论内容
- [ ] 重构图片异步加载逻辑
- [ ] 解决收藏夹、搜索页某些情况导致闪退的问题
- [ ] 完善搜索页：番剧、影视 转为竖图
- [x] 完善播放页投稿列表：调整结构、自动加载下一页
- [ ] 播放页展示合集与推荐
- [x] 添加动态页
- [x] 添加视频检索页
- [x] 完善设置页

<br>

# 反馈问题前要做的事

仅在最新 `大气层` + 最新 `固件` + `FAT32` 内存卡测试，其他组合出现的问题不进行处理。

1. 首先确保 `大气层`、`固件`、`内存卡` 三者符合要求
2. 确保switch系统时间正常，如果进入应用弹出 `网络错误` 一般是由这个问题导致的。
3. 重置switch系统网络设置，尤其是DNS配置，某些DNS服务器无法正确解析API域名和视频地址
4. 完整且详细地描述你的问题，最好附加演示视频、截图。
5. 网络相关的问题附加网络诊断截图，入口：设置/实用工具/网络诊断
6. 尝试复现问题，尽力找到BUG出现的规律

<br>

# 贡献

本应用基于 nanovg 绘制界面，nanovg底层可移植切换到任意图形库，视频播放部分则使用MPV+FFMPEG 通过OpenGL绘制。
所以按照我的理解 wiliwili 应该可以移植到任何一个内存大于500MB，支持OpenGL（ES）的设备。

目前我正在处理mpv在switch上播放部分直播视频报错的问题，也欢迎了解 FFMPEG、MPV或命令行GDB调试的朋友与我联系共同研究。

<br>

# 开发

```shell
git clone --recursive https://github.com/xfangfang/wiliwili.git
```

### PC本地运行

##### macOS

```shell
# macOS: install dependencies
brew install glfw3 glm mpv

mkdir -p build && cd build
cmake -DPLATFORM_DESKTOP=ON ..
make wiliwili -j
```

##### Linux

```shell
# Ubuntu: install dependencies (glfw3.3 or later)
sudo apt install libcurl4-openssl-dev libglfw3-dev libglm-dev libmpv-dev

mkdir -p build && cd build
cmake -DPLATFORM_DESKTOP=ON ..
make wiliwili -j
```

##### Windows

```shell
# Windows: install dependencies (MSYS2 MinGW64)
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make \
  git mingw-w64-x86_64-glfw mingw-w64-x86_64-glm mingw-w64-x86_64-mpv

mkdir -p build && cd build
cmake -G "MinGW Makefiles" -DPLATFORM_DESKTOP=ON ..
mingw32-make wiliwili -j
```

### 交叉编译 Switch 可执行文件

稍后会制作一个docker镜像来更便捷的构建

```shell
# 安装devkitpro环境: https://github.com/devkitPro/pacman/releases

# 安装预编译的依赖
sudo dkp-pacman -S switch-glfw switch-mesa switch-glm \
  switch-sdl2 switch-zlib switch-mbedtls switch-libass \
  switch-cmake  switch-bzip2 devkita64-cmake

# 手动构建 ffmpeg与mpv
# 参考：https://github.com/proconsule/nxmp-portlibs
# 参考：https://github.com/xfangfang/wiliwili/issues/6#issuecomment-1236321540

# 可选：安装依赖库 mininsp：https://github.com/StarDustCFW/nspmini
# 1. 在resources 目录下放置：nsp_forwarder.nsp
# 2. cmake 构建参数添加 -DBUILTIN_NSP=ON
# 按上述配置后，从相册打开wiliwili时会增加一个安装NSP Forwarder的按钮

# build
mkdir build_switch && cd build_switch && cmake ..
make wiliwili.nro -j
```

<br>

# 应用截图

<p align="center">
<img src="docs/images/screenshot-2.png" alt="screenshot-2">
<img src="docs/images/screenshot-1.png" alt="screenshot-1">

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
- https://github.com/whoshuu/cpr
- https://github.com/nlohmann/json
- https://github.com/nayuki/QR-Code-generator
- https://github.com/progschj/ThreadPool
