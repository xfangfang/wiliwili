//
// Created by fang on 2023/1/6.
//

#include <borealis/views/dialog.hpp>
#include "utils/dialog_helper.hpp"
#include "utils/config_helper.hpp"

using namespace brls::literals;

void DialogHelper::showDialog(const std::string& msg) {
    auto dialog = new brls::Dialog(msg);
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->open();
}

void DialogHelper::showCancelableDialog(const std::string& msg, std::function<void(void)> cb) {
    auto dialog = new brls::Dialog(msg);
    dialog->addButton("hints/cancel"_i18n, []() {});
    dialog->addButton("hints/ok"_i18n, [cb]() { cb(); });
    dialog->open();
}

/// 检查本地是否存在登录信息，非联网检查
bool DialogHelper::checkLogin() {
    if (!ProgramConfig::instance().hasLoginInfo()) {
        DialogHelper::showDialog("wiliwili/home/common/no_login"_i18n);
        return false;
    }
    return true;
}

/// 退出应用提示
void DialogHelper::quitApp(bool restart) {
    auto dialog = new brls::Dialog("wiliwili/setting/quit_hint"_i18n);
    dialog->addButton("hints/ok"_i18n, [restart]() {
#ifndef IOS
        brls::Box* container = new brls::Box();
        container->setJustifyContent(brls::JustifyContent::CENTER);
        container->setAlignItems(brls::AlignItems::CENTER);
        brls::Label* hint = new brls::Label();
        hint->setFocusable(true);
        hint->setHideHighlight(true);
        hint->setFontSize(32);
        hint->setText("wiliwili/dialog/quit_hint"_i18n);
        container->addView(hint);
        container->setBackgroundColor(brls::Application::getTheme().getColor("brls/background"));
        brls::Application::pushActivity(new brls::Activity(container), brls::TransitionAnimation::NONE);
        brls::Application::getPlatform()->exitToHomeMode(!restart);
        brls::Application::quit();
#endif /* IOS */
    });
    dialog->setCancelable(false);
    dialog->open();
}