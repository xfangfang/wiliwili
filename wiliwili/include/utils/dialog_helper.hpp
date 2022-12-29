//
// Created by fang on 2022/12/28.
//

#pragma once

#include <string>
#include <borealis/views/dialog.hpp>

using namespace brls::literals;

static void showDialog(const std::string& msg) {
    auto dialog = new brls::Dialog(msg);
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->open();
}

/// 非联网检查
static bool checkLogin() {
    if (ProgramConfig::instance().getUserID().empty() ||
        ProgramConfig::instance().getCSRF().empty()) {
        showDialog("wiliwili/home/common/no_login"_i18n);
        return false;
    }
    return true;
}