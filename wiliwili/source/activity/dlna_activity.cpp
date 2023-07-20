//
// Created by fang on 2023/7/18.
//

#include "view/video_view.hpp"
#include "activity/dlna_activity.hpp"
#include "bilibili/util/uuid.hpp"
#include "utils/config_helper.hpp"
#include "utils/number_helper.hpp"

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

    std::string name =
        GET_SETTING(SettingItem::DLNA_NAME, std::string{"wiliwili"});

    dlna = std::make_shared<pdr::DLNA>(ip, port, uuid);
    dlna->setDeviceInfo("friendlyName", name);
    dlna->setDeviceInfo("manufacturer", "xfangfang");
    dlna->setDeviceInfo("manufacturerURL", "https://github.com/xfangfang");
    dlna->setDeviceInfo("modelDescription", "wiliwili DMR");
    dlna->setDeviceInfo("modelName", "wiliwili");
    dlna->setDeviceInfo("modelNumber", APPVersion::instance().getVersionStr());
    dlna->setDeviceInfo("modelURL", "https://github.com/xfangfang/wiliwili");
    dlna->start();

    dlnaEventSubscribeID = DLNA_EVENT.subscribe([this](const std::string& event,
                                                       void* data) {
        if (event == "CurrentURI") {
            std::string url = std::string{(char*)data};
            brls::Logger::info("CurrentURI: {}", url);
            brls::sync([this, url]() {
                video->setTitle("wiliwili/setting/tools/others/dlna"_i18n);
                video->showOSD(true);
                MPVCore::instance().setUrl(url);
            });
        } else if (event == "CurrentURIMetaData") {
            std::string name = std::string{(char*)data};
            brls::sync([this, name]() { video->setTitle(name); });
        } else if (event == "Stop") {
            brls::sync([this]() {
                MPVCore::instance().stop();
                video->showOSD(false);
                video->setTitle(
                    "wiliwili/setting/tools/others/dlna_waiting"_i18n);
            });
        } else if (event == "Play") {
            brls::sync([]() { MPVCore::instance().resume(); });
            std::string value = "PLAYING";
            PLAYER_EVENT.fire("TransportState", (void*)value.c_str());
        } else if (event == "Pause") {
            brls::sync([]() { MPVCore::instance().pause(); });
            std::string value = "PAUSED_PLAYBACK";
            PLAYER_EVENT.fire("TransportState", (void*)value.c_str());
        } else if (event == "Seek") {
            std::string position = std::string{(char*)data};
            brls::sync([position]() {
                const char* cmd[] = {"seek", position.c_str(), "absolute",
                                     nullptr};
                MPVCore::instance().command(cmd);
            });
        } else if (event == "SetVolume") {
            std::string volume = std::string{(const char*)data};
            brls::sync([volume]() {
                const char* cmd[] = {"set", "volume", volume.c_str(), nullptr};
                MPVCore::instance().command(cmd);
                std::string text = "show-text \"Volume: " + volume + "\" 2000";
                MPVCore::instance().command_str(text.c_str());
            });
        } else if (event == "Error") {
            std::string msg = std::string{(const char*)data};
            brls::sync([this, msg]() {
                video->showOSD(false);
                video->setTitle("[Error] " + msg);
            });
        }
    });

    mpvEventSubscribeID = MPV_E->subscribe([](MpvEventEnum event) {
        switch (event) {
            case MpvEventEnum::MPV_RESUME: {
                std::string value = "PLAYING";
                PLAYER_EVENT.fire("TransportState", (void*)value.c_str());
                break;
            }
            case MpvEventEnum::MPV_PAUSE: {
                std::string value = "PAUSED_PLAYBACK";
                PLAYER_EVENT.fire("TransportState", (void*)value.c_str());
                break;
            }
            case MpvEventEnum::START_FILE: {
                std::string value = "TRANSITIONING";
                PLAYER_EVENT.fire("TransportState", (void*)value.c_str());
                break;
            }
            case MpvEventEnum::END_OF_FILE:
            case MpvEventEnum::MPV_STOP: {
                std::string value = "STOPPED";
                PLAYER_EVENT.fire("TransportState", (void*)value.c_str());
                break;
            }
            case MpvEventEnum::UPDATE_DURATION: {
                std::string value =
                    wiliwili::sec2TimeDLNA(MPVCore::instance().duration);
                PLAYER_EVENT.fire("CurrentTrackDuration", (void*)value.c_str());
                break;
            }
            case MpvEventEnum::UPDATE_PROGRESS: {
                std::string value =
                    wiliwili::sec2TimeDLNA(MPVCore::instance().video_progress);
                PLAYER_EVENT.fire("AbsoluteTimePosition", (void*)value.c_str());
                PLAYER_EVENT.fire("RelativeTimePosition", (void*)value.c_str());
                break;
            }
            case MpvEventEnum::VIDEO_SPEED_CHANGE:
                break;
            case MpvEventEnum::VIDEO_VOLUME_CHANGE: {
                int number = (int)MPVCore::instance().getVolume();
                PLAYER_EVENT.fire("Volume", &number);
                break;
            }
            default:
                break;
        }
    });

    // 初始化状态
    std::string value;
    value = "STOPPED";
    PLAYER_EVENT.fire("TransportState", (void*)value.c_str());
    value = "1";
    PLAYER_EVENT.fire("TransportPlaySpeed", (void*)value.c_str());
    value = "OK";
    PLAYER_EVENT.fire("TransportStatus", (void*)value.c_str());
    value = "0:00:00";
    PLAYER_EVENT.fire("AbsoluteTimePosition", (void*)value.c_str());
    PLAYER_EVENT.fire("RelativeTimePosition", (void*)value.c_str());
    PLAYER_EVENT.fire("CurrentTrackDuration", (void*)value.c_str());
    int number = 2147483647;
    PLAYER_EVENT.fire("AbsoluteCounterPosition", &number);
    PLAYER_EVENT.fire("RelativeCounterPosition", &number);
    number = (int)MPVCore::instance().getVolume();
    PLAYER_EVENT.fire("Volume", &number);
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