//
// Created by fang on 2022/12/28.
//

#pragma once

#include <string>

class DialogHelper {
public:
    /// 展示普通对话框
    static void showDialog(const std::string& msg);

    /// 展示带有取消按钮的对话框
    static void showCancelableDialog(const std::string& msg, std::function<void(void)> cb);

    /// 检查本地是否存在登录信息，非联网检查
    static bool checkLogin();

    /// 退出应用提示
    static void quitApp(bool restart = true);
};