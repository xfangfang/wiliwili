//
// Created by fang on 2022/8/4.
//

#include <borealis/core/thread.hpp>
#include <borealis/views/dialog.hpp>

#include "activity/live_player_activity.hpp"
#include "utils/number_helper.hpp"

#include <vector>
#include <chrono>

#include "utils/shader_helper.hpp"
#include "utils/config_helper.hpp"

#include "live/extract_messages.hpp"
#include "live/ws_utils.hpp"

#include "view/video_view.hpp"
#include "view/live_core.hpp"
#include "view/grid_dropdown.hpp"
#include "view/qr_image.hpp"
#include "view/mpv_core.hpp"

using namespace brls::literals;

static void process_danmaku(const std::vector<LiveDanmakuItem>& danmaku_list) {
    //TODO:做其他处理
    //...

    //弹幕加载到视频中去
    LiveDanmakuCore::instance().add(danmaku_list);

    // danmaku_t_free(dan);
}

static void onDanmakuReceived(const std::string& msg) {
    std::vector<uint8_t> payload(msg.begin(), msg.end());
    std::vector<std::string> messages = parse_packet(payload);

    if (messages.empty()) {
        return;
    }

    std::vector<LiveDanmakuItem> danmaku_list;

    for (const auto& live_msg : extract_messages(messages)) {
        if (live_msg.type == danmaku) {
            if (!live_msg.ptr) continue;
            danmaku_list.emplace_back(LiveDanmakuItem((danmaku_t*)live_msg.ptr));
            // free(live_msg.ptr);
        } else if (live_msg.type == watched_change) {
            //TODO: 更新在线人数
            free(live_msg.ptr);
        }
    }
    process_danmaku(danmaku_list);
}

static void showDialog(const std::string& msg, const std::string& pic, bool forceQuit) {
    brls::Dialog* dialog;
    if (pic.empty()) {
        dialog = new brls::Dialog(msg);
    } else {
        auto box   = new brls::Box();
        auto img   = new brls::Image();
        auto label = new brls::Label();
        label->setText(msg);
        label->setHorizontalAlign(brls::HorizontalAlign::CENTER);
        label->setMargins(20, 0, 10, 0);
        img->setMaxHeight(400);
        img->setImageFromRes(pic);
        box->addView(img);
        box->addView(label);
        box->setAxis(brls::Axis::COLUMN);
        box->setAlignItems(brls::AlignItems::CENTER);
        box->setMargins(20, 20, 20, 20);
        dialog = new brls::Dialog(box);
    }

    dialog->setCancelable(false);
    dialog->addButton("hints/ok"_i18n, [forceQuit]() {
        if (forceQuit) brls::sync([]() { brls::Application::popActivity(); });
    });
    dialog->open();
}

LiveActivity::LiveActivity(int roomid, const std::string& name, const std::string& views) {
    brls::Logger::debug("LiveActivity: create: {}", roomid);
    this->liveData.roomid                  = roomid;
    this->liveData.title                   = name;
    this->liveData.watched_show.text_large = views;
    this->setCommonData();
}

void LiveActivity::setCommonData() {
    // 重置播放器
    MPVCore::instance().reset();

    // 清空自定义着色器
    ShaderHelper::instance().clearShader(false);

    event_id    = APP_E->subscribe([this](const std::string& event, void* data) {
        if (event == VideoView::QUALITY_CHANGE) {
            this->setVideoQuality();
        }
    });
    tl_event_id = MPV_E->subscribe([this](MpvEventEnum e) {
        if (e == UPDATE_PROGRESS) {
            if (!liveRoomPlayInfo.live_time) return;
            std::chrono::time_point<std::chrono::system_clock> _zero;
            size_t now =
                std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - _zero).count();
            this->video->setStatusLabelLeft(wiliwili::sec2Time(now - liveRoomPlayInfo.live_time));
        } else if (e == MPV_FILE_ERROR) {
            this->video->showOSD(false);
            this->video->setStatusLabelLeft("播放错误");

            switch (MPVCore::instance().mpv_error_code) {
                case MPV_ERROR_UNKNOWN_FORMAT:
                    this->video->setOnlineCount("暂不支持当前视频格式");
                    break;
                case MPV_ERROR_LOADING_FAILED:
                    this->video->setOnlineCount("加载失败");
                    // 加载失败时，获取直播间信息，查看是否直播间已经关闭
                    // 如果直播间信息获取失败，则认定为断网，每隔N秒重试一次
                    this->retryRequestData();
                    break;
                default:
                    this->video->setOnlineCount({mpvErrorString(MPVCore::instance().mpv_error_code)});
            }
        } else if (e == END_OF_FILE) {
            // flv 直播遇到网络错误不会报错，而是输出 END_OF_FILE
            // 直播间关闭时也可能进入这里
            this->retryRequestData();
        }
    });
}

void LiveActivity::setVideoQuality() {
    if (this->liveUrl.accept_qn.empty()) return;

    brls::sync([this]() {
        auto dropdown = BaseDropdown::text(
            "wiliwili/player/quality"_i18n, this->getQualityDescriptionList(),
            [this](int selected) {
                defaultQuality = liveUrl.accept_qn[selected];
                this->requestData(this->liveData.roomid);
            },
            this->getCurrentQualityIndex());

        dropdown->registerAction(
            "", brls::ControllerButton::BUTTON_START,
            [dropdown](...) {
                dropdown->dismiss();
                return true;
            },
            true);
    });
}

void LiveActivity::onContentAvailable() {
    brls::Logger::debug("LiveActivity: onContentAvailable");

    MPVCore::instance().setAspect(
        ProgramConfig::instance().getSettingItem(SettingItem::PLAYER_ASPECT, std::string{"-1"}));

    this->video->registerAction("", brls::BUTTON_B, [this](...) {
        if (this->video->isOSDLock()) {
            this->video->toggleOSD();
        } else {
            if (this->video->getTvControlMode() && this->video->isOSDShown()) {
                this->video->toggleOSD();
                return true;
            }
            brls::Logger::debug("exit live");
            brls::Application::popActivity();
        }
        return true;
    });

    this->video->setLiveMode();
    this->video->hideVideoProgressSlider();
    this->video->hideDLNAButton();
    this->video->hideSubtitleSetting();
    this->video->hideVideoRelatedSetting();
    this->video->hideVideoSpeedButton();
    this->video->hideBottomLineSetting();
    this->video->hideHighlightLineSetting();
    this->video->hideSkipOpeningCreditsSetting();
    this->video->disableCloseOnEndOfFile();
    this->video->setFullscreenIcon(true);
    this->video->setTitle(liveData.title);
    this->video->setOnlineCount(liveData.watched_show.text_large);
    this->video->setStatusLabelLeft("");
    this->video->setCustomToggleAction([this]() {
        if (MPVCore::instance().isStopped()) {
            this->onLiveData(this->liveRoomPlayInfo);
        } else if (MPVCore::instance().isPaused()) {
            MPVCore::instance().resume();
        } else {
            this->video->showOSD(false);
            MPVCore::instance().pause();
            brls::cancelDelay(toggleDelayIter);
            ASYNC_RETAIN
            toggleDelayIter = brls::delay(5000, [ASYNC_TOKEN]() {
                ASYNC_RELEASE
                if (MPVCore::instance().isPaused()) {
                    MPVCore::instance().stop();
                }
            });
        }
    });

    // 调整清晰度
    this->registerAction("wiliwili/player/quality"_i18n, brls::ControllerButton::BUTTON_START,
                         [this](brls::View* view) -> bool {
                             this->setVideoQuality();
                             return true;
                         });

    // 根据房间号重新获取高清播放链接
    this->requestData(liveData.roomid);

    // 连接直播弹幕
    this->requestLiveDanmakuToken(this->liveData.roomid);

    // 获取直播间是否为大航海专属直播
    this->requestPayLiveInfo(liveData.roomid);

    GA("open_live", {{"id", std::to_string(this->liveData.roomid)}})
    GA("open_live", {{"live_id", std::to_string(this->liveData.roomid)}})
}

std::vector<std::string> LiveActivity::getQualityDescriptionList() {
    std::vector<std::string> res;
    for (auto& i : liveUrl.accept_qn) {
        res.push_back(getQualityDescription(i));
    }
    return res;
}

int LiveActivity::getCurrentQualityIndex() {
    for (size_t i = 0; i < liveUrl.accept_qn.size(); i++) {
        if (liveUrl.accept_qn[i] == this->liveUrl.current_qn) return i;
    }
    return 0;
}

void LiveActivity::onLiveData(const bilibili::LiveRoomPlayInfo& result) {
    // todo：定时获取在线人数
    this->video->setOnlineCount(liveData.watched_show.text_large);

    if (result.is_locked) {
        brls::Logger::error("LiveActivity: live {} is locked", result.room_id);
        this->video->showOSD(false);
        showDialog(fmt::format("这个房间已经被封禁（至 {}）！(╯°口°)╯(┴—┴", wiliwili::sec2FullDate(result.lock_till)),
                   "pictures/room-block.png", true);
        return;
    }
    // 0: 未开播 1: 直播中 2: 轮播中
    if (result.live_status == 0) {
        // 未开播
        this->video->showOSD(false);
        showDialog("未开播", "pictures/sorry.png", true);
        return;
    } else if (result.live_status == 2) {
        // todo: 支持轮播视频
        this->video->showOSD(false);
        showDialog("未开播", "pictures/sorry.png", true);
        return;
    }
    brls::Logger::debug("current quality: {}", liveUrl.current_qn);
    for (auto& i : liveUrl.accept_qn) {
        auto desc = getQualityDescription(i);
        brls::Logger::debug("live quality: {}/{}", desc, i);
        if (liveUrl.current_qn == i) {
            std::string quality = desc + " \uE0EF";
            APP_E->fire(VideoView::SET_QUALITY, (void*)quality.c_str());
        }
    }
    // todo: 允许使用备用链接
    for (const auto& i : liveUrl.url_info) {
        auto url = i.host + liveUrl.base_url + i.extra;

        // 设置视频链接
        brls::Logger::debug("Live stream url: {}", url);
        this->video->setUrl(url);
        return;
    }

    this->video->showOSD(false);
    showDialog("当前地区无法获取直播链接", "pictures/sorry.png", true);
}

void LiveActivity::onDanmakuInfo(int roomid, const bilibili::LiveDanmakuinfo& info) {
    danmaku.setonMessage(onDanmakuReceived);
    danmaku.connect(roomid, std::stoll(ProgramConfig::instance().getUserID()), info);
}

void LiveActivity::onError(const std::string& error) {
    brls::Logger::error("ERROR request live data: {}", error);
    this->video->showOSD(false);
    this->video->setOnlineCount(error);
    this->retryRequestData();
}

void LiveActivity::onNeedPay(const std::string& msg, const std::string& link, const std::string& startTime,
                             const std::string& endTime) {
    if (link.empty()) {
        showDialog(msg, "", true);
        return;
    }

    auto box      = new brls::Box();
    auto img      = new QRImage();
    auto label    = new brls::Label();
    auto header   = new brls::Label();
    auto subtitle = new brls::Label();
    header->setFontSize(24);
    header->setMargins(10, 0, 20, 0);
    header->setText(msg);
    subtitle->setTextColor(brls::Application::getTheme().getColor("font/grey"));
    subtitle->setText(startTime + " - " + endTime);
    subtitle->setMarginBottom(10);
    subtitle->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    header->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    label->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    label->setMargins(20, 0, 10, 0);
    label->setText("请使用手机客户端扫码开通");
    img->setHeight(240);
    img->setWidth(240);
    img->setImageFromQRContent(link);
    box->setAxis(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::CENTER);
    box->setMargins(20, 20, 20, 20);
    box->addView(header);
    box->addView(subtitle);
    box->addView(img);
    box->addView(label);
    auto dialog = new brls::Dialog(box);

    dialog->setCancelable(false);
    dialog->addButton("hints/ok"_i18n, []() { brls::sync([]() { brls::Application::popActivity(); }); });
    dialog->open();
}

void LiveActivity::retryRequestData() {
    // 每隔一段时间自动重试
    brls::cancelDelay(errorDelayIter);
    ASYNC_RETAIN
    errorDelayIter = brls::delay(2000, [ASYNC_TOKEN]() {
        ASYNC_RELEASE
        if (!MPVCore::instance().isPlaying()) this->requestData(liveData.roomid);
    });
}

LiveActivity::~LiveActivity() {
    brls::Logger::debug("LiveActivity: delete");
    danmaku.disconnect();
    // 取消监控mpv
    APP_E->unsubscribe(event_id);
    MPV_E->unsubscribe(tl_event_id);
    // 在取消监控之后再停止播放器，避免在播放器停止时触发事件 (尤其是：END_OF_FILE)
    this->video->stop();
    LiveDanmakuCore::instance().reset();
    brls::cancelDelay(toggleDelayIter);
    brls::cancelDelay(errorDelayIter);
}
