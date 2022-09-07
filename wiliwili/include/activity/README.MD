# wiliwili Activity

应用页面，称为Activity，其中可包含任意多的Fragment（参考Android开发中的相关概念，下面称之为子页面）

## 页面功能简述

main_activity:

包含：首页、动态、我的 三个子页面

player_activity:

视频播放界面，以及：评论、相关推荐 两个子页面

settings_activity:

应用的设置界面

search_activity:

视频搜索界面

## 其他页面

其他页面由 子页面 通过 其他继承Activity的子类（如：brls::Dialog） 直接封装而成，此类页面不涉及复杂功能故不单独列出。