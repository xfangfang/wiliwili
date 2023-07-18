//
// Created by fang on 2023/7/18.
//

#include "view/video_view.hpp"
#include "activity/dlna_activity.hpp"
#include "bilibili/util/uuid.hpp"
#include "utils/config_helper.hpp"

using namespace brls::literals;

#define GET_SETTING ProgramConfig::instance().getSettingItem

DLNAActivity::DLNAActivity() {
    ip = brls::Application::getPlatform()->getIpAddress();
    ip = GET_SETTING(SettingItem::DLNA_IP, ip);
    brls::Logger::info("DLNA IP: {}", ip);

    port = 9958;
    port = GET_SETTING(SettingItem::DLNA_PORT, port);
    brls::Logger::info("DLNA Port: {}", port);

    uuid = "uuid:" + bilibili::genUUID(ProgramConfig::instance().getClientID());
    brls::Logger::info("DLNA UUID: {}", uuid);

    std::string name = GET_SETTING(SettingItem::DLNA_NAME, std::string{"wiliwili"});

    dlna = std::make_shared<pdr::DLNA>(ip, port, uuid);
    dlna->setDeviceInfo("friendlyName", name);
    dlna->start();

    dlnaEventSubscribeID = DLNA_EVENT.subscribe([this](const std::string& event, void* data) {
        if (event == "CurrentURI") {
            std::string url = std::string{(char*)data};
            brls::Logger::info("CurrentURI: {}", url);
            brls::sync([this, url]() {
                video->setTitle("wiliwili/setting/tools/others/dlna"_i18n);
                video->setUrl(url);
                video->showOSD(true);
            });
        } else if (event == "Stop") {
            brls::sync([this]() {
                MPVCore::instance().stop();
                this->video->showOSD(false);
                video->setTitle("wiliwili/setting/tools/others/dlna_waiting"_i18n);
            });
        }
    });

    mpvEventSubscribeID = MPV_E->subscribe([](MpvEventEnum event) {
        // todo: 发布播放器事件到 DLNA
    });
}

void DLNAActivity::onContentAvailable() {
    this->video->registerAction("", brls::BUTTON_B, [](...) {
        brls::Application::popActivity();
        return true;
    });

    this->video->hideActionButtons();
    this->video->setFullscreenIcon(true);
    this->video->setTitle("wiliwili/setting/tools/others/dlna_waiting"_i18n);
    this->video->showOSD(false);
    this->video->setOnlineCount(fmt::format("http://{}:{}", ip, port));
}

DLNAActivity::~DLNAActivity() {
    MPV_E->unsubscribe(mpvEventSubscribeID);
    DLNA_EVENT.unsubscribe(dlnaEventSubscribeID);
    dlna->stop();
    video->stop();
}