//
// Created by fang on 2022/8/4.
//

#include <borealis/core/thread.hpp>
#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/views/dialog.hpp>

#include "activity/live_player_activity.hpp"
#include "utils/number_helper.hpp"
#include "live/dl_emoticon.hpp"

#include <vector>
#include <chrono>

#include "utils/shader_helper.hpp"
#include "utils/config_helper.hpp"
#include "utils/dialog_helper.hpp"

#include "view/video_view.hpp"
#include "view/live_core.hpp"
#include "view/grid_dropdown.hpp"
#include "view/qr_image.hpp"
#include "view/mpv_core.hpp"
#include "view/user_info.hpp"
#include "view/live_danmaku_item.hpp"

#include "api/live/extract_messages.hpp"
#include "api/live/ws_utils.hpp"
#include "api/live/dl_emoticon.hpp"
#include "bilibili.h"
#include "bilibili/api.h"
#include "bilibili/util/http.hpp"
#include "bilibili/result/mine_result.h"

using namespace brls::literals;

// 处理弹幕展示
static void process_danmaku(const std::vector<LiveDanmakuItem>& danmaku_list) {
    // 弹幕加载到视频中去
    LiveDanmakuCore::instance().add(danmaku_list);
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
    this->liveData.title                   = name.empty() ? "直播间 " + std::to_string(roomid) : name;
    this->liveData.watched_show.text_large = views.empty() ? "获取中..." : views;
    this->liveData.uname                   = ""; // 初始为空，待获取
    this->liveData.cover                   = ""; // 初始为空，待获取
    
    this->setCommonData();
}

void LiveActivity::setCommonData() {
    // 重置播放器
    MPVCore::instance().reset();

    // 清空自定义着色器
    ShaderHelper::instance().clearShader(false);

    this->maxSidebarDanmakuCount = ProgramConfig::instance().getIntOption(SettingItem::LIVE_SIDEBAR_DANMAKU_COUNT);

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

    ASYNC_RETAIN
    brls::sync([ASYNC_TOKEN]() {
        ASYNC_RELEASE
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

void LiveActivity::onContentAvailable()
{   
    // 设置全屏按钮图标
    // 首先查找并绑定video组件，这在两种布局中都存在
    this->video = (VideoView*)this->getView("video");
    if (!this->video) {
        brls::Logger::error("LiveActivity: 找不到video视图");
        return;
    }
    
    this->video->setFullscreenIcon(this->video->isFullscreen());

    this->video->registerCommonActions(this);

    brls::Logger::debug("LiveActivity: onContentAvailable");

    MPVCore::instance().setAspect(
        ProgramConfig::instance().getSettingItem(SettingItem::PLAYER_ASPECT, std::string{"-1"}));

    // 根据侧边栏设置调整界面布局
    if (this->shouldShowSidebar()) {
        // 绑定侧边栏相关的UI元素
        this->liveAuthor = (UserInfoView*)this->getView("live_author");
        this->liveDanmakuContainer = (brls::Box*)this->getView("live_danmaku_container");
        this->liveDanmakuList = (brls::ScrollingFrame*)this->getView("live_danmaku_list");
        this->liveTitleLabel = (brls::Label*)this->getView("live/title");
        this->anchorTitleLabel = (brls::Label*)this->getView("anchor/title");
        this->liveDanmakuSidebar = (brls::Box*)this->getView("live_danmaku_sidebar");
        this->liveDetailLeftBox = (brls::Box*)this->getView("live_detail_left_box");
        
        // 设置直播标题
        if (this->liveTitleLabel) {
            this->liveTitleLabel->setText(liveData.title);
        }
        
        // 初始显示空的主播信息，等待API获取后更新
        if (this->liveAuthor) {
            this->liveAuthor->setUserInfo("", "加载中...", "");
            this->liveAuthor->setHintType(InfoHintType::NONE); // 初始不显示关注按钮

            // 注册点击
            this->liveAuthor->registerClickAction([this](...) -> bool {
                if (!DialogHelper::checkLogin()) return true;

                uint64_t uid = this->liveRoomPlayInfo.uid;
                if (uid == 0) {
                    brls::Logger::warning("LiveActivity: 主播信息尚未加载，无法操作关注状态");
                    return true;
                }
                
                if (std::to_string(uid) == ProgramConfig::instance().getUserID()) return true;

                if (this->anchor_following) {
                    auto dialog = new brls::Dialog("wiliwili/player/not_follow"_i18n);
                    dialog->addButton("hints/cancel"_i18n, []() {});
                    dialog->addButton("hints/ok"_i18n, [this]() { this->follow_anchor(false); });
                    dialog->open();
                } else {
                    this->follow_anchor(true);
                }
                return true;
            });

            // 添加手势识别器以支持鼠标点击
            this->liveAuthor->addGestureRecognizer(new brls::TapGestureRecognizer(this->liveAuthor));
        }
    }
    
    // 设置视频相关UI
    this->video->setLiveMode();
    this->video->hideSubtitleSetting();
    this->video->hideVideoRelatedSetting();
    this->video->hideBottomLineSetting();
    this->video->hideHighlightLineSetting();
    this->video->hideSkipOpeningCreditsSetting();
    this->video->hideOSDLockButton(); // 隐藏防误触按钮
    this->video->disableCloseOnEndOfFile();
    
    // 禁用底部固定进度条
    VideoView::BOTTOM_BAR = false;
    
    this->video->setTitle(liveData.title);
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

    // 连接直播弹幕 - 确保g_liveActivity在此之前已初始化
    this->requestLiveDanmakuToken(this->liveData.roomid);

    // 获取历史弹幕，填充侧边栏
    this->requestHistoryDanmaku(this->liveData.roomid);

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

void LiveActivity::onLiveData(const bilibili::LiveRoomPlayInfo &result)
{
    brls::Logger::debug("LiveActivity: onLiveData");
    // 判断房间是否被封禁
    if (result.is_locked) {
        brls::Logger::debug("LiveActivity: live {} is locked", result.room_id);
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
    // 更新画质信息，确保全屏模式下也能显示正确的画质
    std::string currentQualityDesc = "";
    for (auto &i : liveUrl.accept_qn) {
        auto desc = getQualityDescription(i);
        brls::Logger::debug("live quality: {}/{}", desc, i);
        if (liveUrl.current_qn == i) {
            currentQualityDesc = desc;
            this->video->setQuality(desc);
            // 向所有VideoView实例发送画质更新事件（确保全屏视图更新）
            APP_E->fire(VideoView::SET_QUALITY, (void*)desc.c_str());
        }
    }

    // 设置视频标题、在线人数和主播信息
    this->video->setTitle(liveData.title);
    this->video->setOnlineCount(liveData.watched_show.text_large);

    // 获取主播信息
    this->requestLiveAnchorInfo(liveData.roomid);
    
    // 获取主播称号信息
    this->requestLiveAnchorTitle(liveData.roomid);

    // 查询并设置主播关注状态
    if (result.uid > 0) {
        std::string uid = std::to_string(result.uid);

        ASYNC_RETAIN
        BILI::get_user_relation_detail(
            uid,
            [ASYNC_TOKEN](const bilibili::UserRelationDetail& r) {
                bool followed = (r.attribute == 2 || r.attribute == 6);
                brls::Logger::info("是否关注了该主播: {}", followed);
                brls::sync([ASYNC_TOKEN, followed]() {
                    ASYNC_RELEASE
                    this->anchor_following = followed;
                    if (this->shouldShowSidebar() && this->liveAuthor) {
                        if (ProgramConfig::instance().getUserID() == std::to_string(this->liveRoomPlayInfo.uid))
                            this->liveAuthor->setHintType(InfoHintType::NONE);
                        else
                            this->liveAuthor->setHintType(followed ? InfoHintType::UP_FOLLOWING
                                                                   : InfoHintType::UP_NOT_FOLLOWED);
                    }
                });
            },
            [ASYNC_TOKEN](BILI_ERR) {
                brls::sync([ASYNC_TOKEN, error]() {
                    ASYNC_RELEASE
                    brls::Logger::error("get_user_relation_detail: {}", error);
                    if (this->shouldShowSidebar() && this->liveAuthor) {
                        this->liveAuthor->setHintType(InfoHintType::NONE);
                    }
                });
            });
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
    // 保存必要的状态
    auto state = this->threadState;
    auto room_id_copy = roomid;
    auto emoticonsPtr = std::make_shared<lmp>();
    
    // 使用cpr::async在后台线程执行任务
    cpr::async([state, room_id_copy, emoticonsPtr, info, this]() {
        try {
            // 检查活动状态
            if (!state->isActive.load(std::memory_order_acquire)) {
                brls::Logger::debug("LiveActivity: 线程中断: 对象已被销毁");
                return;
            }
            
            // 在线程中安全地访问表情包映射
            *emoticonsPtr = dl_emoticon(room_id_copy);
            brls::Logger::debug("LiveActivity: 获取了 {} 个表情包URL", emoticonsPtr->size());
            
            // 将表情包映射传递给LiveDanmakuCore
            LiveDanmakuCore::instance().setEmoticons(emoticonsPtr);
            
            // 再次检查活动状态
            if (!state->isActive.load(std::memory_order_acquire)) {
                brls::Logger::debug("LiveActivity: 线程中断: 对象已被销毁");
                return;
            }
            
            // 设置消息回调,捕获共享状态
            this->danmaku.setonMessage([state, this](const std::string& msg) {
                // 检查活动状态
                if (!state->isActive.load(std::memory_order_acquire)) {
                    brls::Logger::debug("LiveActivity: 消息回调中断: 对象已被销毁");
                    return;
                }

                ASYNC_RETAIN
                // 使用brls::sync确保在UI线程中更新UI
                brls::sync([state, msg, ASYNC_TOKEN]() {
                    ASYNC_RELEASE
                    if (!state->isActive.load(std::memory_order_acquire)) {
                        brls::Logger::debug("LiveActivity: UI更新中断: 对象已被销毁");
                        return;
                    }
                    
                    std::vector<uint8_t> payload(msg.begin(), msg.end());
                    std::vector<std::string> messages = parse_packet(payload);
                    
                    if (messages.empty()) {
                        return;
                    }
                    
                    std::vector<LiveDanmakuItem> danmaku_list;
                    std::vector<LiveDanmakuItem> sc_list;
                    
                    for (const auto& live_msg : extract_messages(messages)) {
                        if (!live_msg) continue;
                        
                        if (live_msg->type == MessageType::DANMAKU) {
                            auto* danmaku_msg = dynamic_cast<message::LiveDanmaku*>(live_msg.get());
                            if (!danmaku_msg || !danmaku_msg->data) continue;
                            
                            danmaku_list.emplace_back(LiveDanmakuItem(danmaku_msg->data));
                        } else if (live_msg->type == MessageType::WATCHED_CHANGE) {
                            // 更新看过人数
                            auto* watched_msg = dynamic_cast<message::LiveWatchedChange*>(live_msg.get());
                            if (watched_msg && watched_msg->data) {
                                this->updateWatchedCount(watched_msg->data->num);
                            }
                        } else if (live_msg->type == MessageType::ONLINE_CNT) {
                            // 更新在线人数
                            auto* online_msg = dynamic_cast<message::LiveOnlineCount*>(live_msg.get());
                            if (online_msg && online_msg->data) {
                                this->updateOnlineCount(online_msg->data->count);
                            }
                        } else if (live_msg->type == MessageType::SUPER_CHAT) {
                            auto* sc_msg = dynamic_cast<message::LiveSuperChat*>(live_msg.get());
                            if (!sc_msg || !sc_msg->data) continue;
                            
                            sc_list.emplace_back(LiveDanmakuItem(sc_msg->data));
                        }
                    }
                    
                    // 处理弹幕到视频
                    process_danmaku(danmaku_list);
                    
                    // 处理弹幕到侧边栏
                    if (!danmaku_list.empty() && state->isActive.load(std::memory_order_acquire)) {
                        this->processDanmakuForSidebar(danmaku_list);
                    }
                    
                    // 处理SC消息到侧边栏
                    if (!sc_list.empty() && state->isActive.load(std::memory_order_acquire)) {
                        this->processSuperChatForSidebar(sc_list);
                    }
                });
            });
            
            // 连接弹幕服务器
            this->danmaku.connect(room_id_copy, std::stoll(ProgramConfig::instance().getUserID()), info);
            
        } catch (const std::exception& e) {
            brls::Logger::error("LiveActivity: 弹幕处理错误: {}", e.what());
        }
    });
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

void LiveActivity::processDanmakuForSidebar(const std::vector<LiveDanmakuItem>& danmaku_list) {
    // 如果设置为0，表示隐藏侧边栏，直接返回
    if (!this->shouldShowSidebar() || !this->liveDanmakuContainer) {
        return;
    }
    
    // 过滤弹幕列表
    std::vector<LiveDanmakuItem> filtered_danmakus;
    filtered_danmakus.reserve(danmaku_list.size());
    
    // 进行过滤
    for (const auto& danmaku : danmaku_list) {
        // 添加弹幕等级过滤，与LiveDanmakuCore::add方法中的过滤条件保持一致
        if (danmaku.danmaku->user_level < LiveDanmakuCore::DANMAKU_FILTER_LEVEL_LIVE) 
            continue;
        filtered_danmakus.push_back(danmaku);
    }
    
    // 如果没有通过过滤的弹幕，直接返回
    if (filtered_danmakus.empty()) {
        return;
    }
    
    auto state = this->threadState;
    // 确保UI更新在主线程进行
    ASYNC_RETAIN
    brls::sync([state, filtered_danmakus = std::move(filtered_danmakus), ASYNC_TOKEN]() mutable {
        ASYNC_RELEASE
        // 再次检查UI组件和活动状态
        if (!state->isActive.load(std::memory_order_acquire) || !this->shouldShowSidebar() || !this->liveDanmakuContainer) {
            return;
        }
        
        for (const auto& danmaku : filtered_danmakus) {
            auto* item = LiveDanmakuItemView::create();
            item->setDanmaku(danmaku);
            
            // 将新弹幕添加到顶部（但在置顶SC之下）
            if (this->liveDanmakuContainer->getChildren().size() > 0) {
                // 查找合适的插入位置：在所有置顶SC之后
                int insertPos = 0;
                for (size_t i = 0; i < this->liveDanmakuContainer->getChildren().size(); i++) {
                    auto* existingItem = dynamic_cast<LiveDanmakuItemView*>(this->liveDanmakuContainer->getChildren()[i]);
                    if (existingItem && existingItem->isPinned()) {
                        insertPos++;
                    } else {
                        break;
                    }
                }
                this->liveDanmakuContainer->addView(item, insertPos);
            } else {
                this->liveDanmakuContainer->addView(item);
            }
            
            // 控制侧边栏弹幕数量
            if (this->liveDanmakuContainer->getChildren().size() > this->maxSidebarDanmakuCount) {
                auto& children = this->liveDanmakuContainer->getChildren();
                
                // 从末尾开始查找非置顶的项目删除
                for (int i = children.size() - 1; i >= 0; i--) {
                    auto* danmakuItem = dynamic_cast<LiveDanmakuItemView*>(children[i]);
                    if (danmakuItem && !danmakuItem->isPinned()) {
                        this->liveDanmakuContainer->removeView(children[i]);
                        break;
                    }
                }
            }
        }
    });
}

void LiveActivity::processSuperChatForSidebar(const std::vector<LiveDanmakuItem>& sc_list) {
    // 如果设置为0，表示隐藏侧边栏，直接返回
    if (!this->shouldShowSidebar() || !this->liveDanmakuContainer) {
        return;
    }

    auto state = this->threadState;
    // 确保UI更新在主线程进行
    ASYNC_RETAIN
    brls::sync([sc_list, state, ASYNC_TOKEN]() {
        ASYNC_RELEASE
        // 再次检查UI组件和活动状态
        if (!state->isActive.load(std::memory_order_acquire) || !this->shouldShowSidebar() || !this->liveDanmakuContainer) {
            return;
        }
        
        for (const auto& sc : sc_list) {
            auto* item = LiveDanmakuItemView::create();
            item->setDanmaku(sc);
            
            // 将新SC添加到侧边栏顶部
            if (this->liveDanmakuContainer->getChildren().size() > 0) {
                this->liveDanmakuContainer->addView(item, 0);
            } else {
                this->liveDanmakuContainer->addView(item);
            }
            
            // 设置SC置顶（如果持续时间大于0）
            if (sc.super_chat && sc.super_chat->time > 0) {
                std::string scToken = this->generateSuperChatToken(sc);
                
                // 设置SC项的唯一标识
                item->setSuperChatToken(scToken);
                
                // 添加到置顶SC管理中
                this->addPinnedSuperChat(sc, scToken);
                
                // 设置视图项为置顶状态
                item->setPinned(true);
                
                // 保存视图项引用
                this->pinnedSuperChatViews[scToken] = item;
            }
            
            // 控制侧边栏弹幕数量
            if (this->liveDanmakuContainer->getChildren().size() > this->maxSidebarDanmakuCount) {
                auto& children = this->liveDanmakuContainer->getChildren();
                
                // 从末尾开始查找非置顶的项目删除
                for (int i = children.size() - 1; i >= 0; i--) {
                    auto* danmakuItem = dynamic_cast<LiveDanmakuItemView*>(children[i]);
                    if (danmakuItem && !danmakuItem->isPinned()) {
                        this->liveDanmakuContainer->removeView(children[i]);
                        break;
                    }
                }
                
                // 如果所有项都是置顶状态，则删除最后一个（最旧的）
                if (this->liveDanmakuContainer->getChildren().size() > this->maxSidebarDanmakuCount) {
                    auto& updatedChildren = this->liveDanmakuContainer->getChildren();
                    this->liveDanmakuContainer->removeView(updatedChildren[updatedChildren.size() - 1]);
                }
            }
        }
        
        // 启动SC过期检查定时器（如果尚未启动）
        if (this->pinnedSuperChats.size() > 0 && this->scExpiryCheckIter == 0) {
            this->startSuperChatExpiryTimer();
        }
    });
}

// 添加SC置顶
void LiveActivity::addPinnedSuperChat(const LiveDanmakuItem& sc, const std::string& scToken) {
    // 确保是SC消息
    if (sc.type != LiveDanmakuItem::Type::SUPER_CHAT || !sc.super_chat) {
        return;
    }
    
    // 检查提供的token是否为空
    if (scToken.empty()) {
        brls::Logger::error("LiveActivity: 添加置顶SC失败: token为空");
        return;
    }
    
    // 计算过期时间
    auto now = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> expiryTime;
    
    // 如果提供了end_time字段且有效，直接使用
    if (sc.super_chat->end_time > 0) {
        // 使用end_time作为过期时间点
        auto end_time_point = std::chrono::system_clock::from_time_t(sc.super_chat->end_time);
        expiryTime = end_time_point;
        brls::Logger::debug("LiveActivity: 使用SC提供的结束时间戳: {}", sc.super_chat->end_time);
    } else {
        // 使用当前时间加上持续时间
        expiryTime = now + std::chrono::seconds(sc.super_chat->time);
        brls::Logger::debug("LiveActivity: 使用SC持续时间计算过期时间: {}秒", sc.super_chat->time);
    }
    
    // 添加到置顶管理
    this->pinnedSuperChats[scToken] = expiryTime;
    
    brls::Logger::debug("LiveActivity: 添加置顶SC: Token={}, 用户ID={}, 持续时间={}秒", 
                       scToken, sc.super_chat->user_uid, sc.super_chat->time);
}

// 生成SC唯一标识
std::string LiveActivity::generateSuperChatToken(const LiveDanmakuItem& sc) const {
    if (sc.type != LiveDanmakuItem::Type::SUPER_CHAT || !sc.super_chat) {
        // 非SC消息，返回空字符串
        return "";
    }
    
    // 获取当前时间戳（毫秒级）
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    // 组合SC的属性：用户ID + 时间戳 + 金额
    std::string token = std::to_string(sc.super_chat->user_uid) + "_" + 
                        std::to_string(ms) + "_" + 
                        std::to_string(sc.super_chat->price);
    
    // 添加随机数以进一步确保唯一性
    // 使用当前时间作为随机数种子
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    token += "_" + std::to_string(std::rand() % 10000);
    
    return token;
}

// 移除SC置顶
void LiveActivity::removePinnedSuperChat(const std::string& scToken) {
    // 检查是否为空标识
    if (scToken.empty()) {
        return;
    }
    
    // 查找SC视图项
    auto viewIt = this->pinnedSuperChatViews.find(scToken);
    if (viewIt != this->pinnedSuperChatViews.end() && viewIt->second) {
        LiveDanmakuItemView* itemView = viewIt->second;
        
        // 先取消置顶状态
        itemView->setPinned(false);
        
        // 然后从界面中移除视图项
        brls::View* view = itemView;
        auto& children = this->liveDanmakuContainer->getChildren();
        
        // 找到当前视图位置
        int currentIndex = -1;
        for (size_t i = 0; i < children.size(); i++) {
            if (children[i] == view) {
                currentIndex = i;
                break;
            }
        }
        
        // 如果找到了，直接删除
        if (currentIndex >= 0) {
            this->liveDanmakuContainer->removeView(view);
        }
        
        // 从视图项映射中移除
        this->pinnedSuperChatViews.erase(scToken);
    }
    
    // 从置顶管理中移除
    this->pinnedSuperChats.erase(scToken);
    
    brls::Logger::debug("LiveActivity: 移除置顶SC: Token={}", scToken);
    
    // 如果没有置顶SC了，取消定时器
    if (this->pinnedSuperChats.empty()) {
        brls::cancelDelay(this->scExpiryCheckIter);
        this->scExpiryCheckIter = 0;
    }
}

// 检查SC是否置顶
bool LiveActivity::isPinnedSuperChat(const std::string& scToken) const {
    return !scToken.empty() && this->pinnedSuperChats.find(scToken) != this->pinnedSuperChats.end();
}

// 启动SC过期检查定时器
void LiveActivity::startSuperChatExpiryTimer() {
    // 取消已有的定时器
    brls::cancelDelay(this->scExpiryCheckIter);
    
    // 启动新的定时器，每秒检查一次
    ASYNC_RETAIN
    this->scExpiryCheckIter = brls::delay(1000, [ASYNC_TOKEN]() {
        ASYNC_RELEASE
        this->checkPinnedSuperChatExpiry();
    });
}

// 定时器回调，检查SC过期
void LiveActivity::checkPinnedSuperChatExpiry() {
    auto now = std::chrono::system_clock::now();
    std::vector<std::string> expiredScTokens;

    // 查找已过期的SC
    for (const auto& [scToken, expiryTime] : this->pinnedSuperChats) {
        if (now >= expiryTime) {
            expiredScTokens.push_back(scToken);
        }
    }
    
    // 移除已过期的SC
    for (const std::string& scToken : expiredScTokens) {
        this->removePinnedSuperChat(scToken);
    }
    
    // 如果还有置顶SC，继续检查
    if (!this->pinnedSuperChats.empty()) {
        this->startSuperChatExpiryTimer();
    }
}

LiveActivity::~LiveActivity() {
    brls::Logger::debug("LiveActivity: delete");
    
    // 恢复底部固定进度条的设置
    VideoView::BOTTOM_BAR = ProgramConfig::instance().getBoolOption(SettingItem::PLAYER_BOTTOM_BAR);
    
    threadState->isActive.store(false, std::memory_order_release);
    
    // 断开直播弹幕连接
    danmaku.disconnect();
    
    // 取消监控mpv
    APP_E->unsubscribe(event_id);
    MPV_E->unsubscribe(tl_event_id);
    
    // 在取消监控之后再停止播放器，避免在播放器停止时触发事件 (尤其是：END_OF_FILE)
    if (this->video) {
        this->video->stop();
    }
    
    // 重置直播弹幕核心
    LiveDanmakuCore::instance().reset();
    
    // 取消延时任务
    brls::cancelDelay(toggleDelayIter);
    brls::cancelDelay(errorDelayIter);
    brls::cancelDelay(scExpiryCheckIter);
    
    // 清空置顶SC数据
    this->pinnedSuperChats.clear();
    this->pinnedSuperChatViews.clear();
    
    brls::Logger::debug("LiveActivity: delete completed");
}

// 添加onAnchorInfo实现
void LiveActivity::onAnchorInfo(const std::string& face, const std::string& uname) {
    brls::Logger::debug("LiveActivity: onAnchorInfo: 用户名={}, 头像URL={}", uname, face);
    
    if (this->shouldShowSidebar() && this->liveAuthor) {
        // 设置主播信息
        if (!face.empty()) {
            this->liveAuthor->setUserInfo(face, uname, liveData.watched_show.text_large);
        } else {
            this->liveAuthor->setUserInfo("pictures/default_avatar.png", uname, liveData.watched_show.text_large);
        }
    }

    // 缓存主播头像与昵称，避免后续更新人数时将头像清空
    this->liveData.uname = uname;
    if (!face.empty())
        this->liveData.cover = face;
    else if (this->liveData.cover.empty())
        this->liveData.cover = "pictures/default_avatar.png";
    
    // 更新标题
    if (this->video) {
        this->video->setTitle(liveData.title);
    }
    
    // 同时更新侧边栏标题（如果存在）
    if (this->shouldShowSidebar() && this->liveTitleLabel) {
        this->liveTitleLabel->setText(liveData.title);
    }
}

void LiveActivity::follow_anchor(bool follow) {
    std::string csrf = ProgramConfig::instance().getCSRF();
    if (csrf.empty()) return;
    uint64_t uid = this->liveRoomPlayInfo.uid;
    if (uid == 0) return;

    // 预先更新 UI
    if (this->shouldShowSidebar() && this->liveAuthor) {
        this->liveAuthor->setHintType(follow ? InfoHintType::UP_FOLLOWING : InfoHintType::UP_NOT_FOLLOWED);
    }

    ASYNC_RETAIN
    BILI::follow_up(
        csrf, std::to_string(uid), follow,
        [ASYNC_TOKEN, follow]() {
            ASYNC_RELEASE
            this->anchor_following = follow;
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::Logger::error("follow_up: {}", error);
            brls::sync([ASYNC_TOKEN]() {
                ASYNC_RELEASE
                // 回滚 UI
                if (this->shouldShowSidebar() && this->liveAuthor) {
                    this->liveAuthor->setHintType(this->anchor_following ? InfoHintType::UP_FOLLOWING
                                                                         : InfoHintType::UP_NOT_FOLLOWED);
                }
            });
        });
}

// 添加主播称号信息处理函数
void LiveActivity::onAnchorTitleInfo(const std::string& title) {
    brls::Logger::debug("LiveActivity: onAnchorTitleInfo: 称号={}", title);
    
    // 保存称号
    this->anchorTitle = title;
    
    if (this->shouldShowSidebar() && this->anchorTitleLabel) {
        // 设置称号标签文本
        if (!title.empty()) {
            this->anchorTitleLabel->setText(title);
            this->anchorTitleLabel->setVisibility(brls::Visibility::VISIBLE);
        } else {
            // 如果没有称号，隐藏标签
            this->anchorTitleLabel->setVisibility(brls::Visibility::GONE);
        }
    }
}

static void append_danmaku_from_json_array(const nlohmann::json& arr, std::vector<LiveDanmakuItem>& dan_list) {
    for (const auto& item : arr) {
        auto dan = std::make_shared<message::Danmaku>();

        // 基础字段
        if (item.contains("text") && item["text"].is_string())
            dan->dan = item["text"].get<std::string>();
        if (item.contains("nickname") && item["nickname"].is_string())
            dan->user_name = item["nickname"].get<std::string>();
        if (item.contains("uid") && item["uid"].is_number_integer())
            dan->user_uid = item["uid"].get<int>();

        // 用户名颜色
        if (item.contains("uname_color") && item["uname_color"].is_string()) {
            const std::string& color_str = item["uname_color"].get<std::string>();
            if (!color_str.empty() && color_str[0] == '#' && color_str.length() == 7)
                dan->user_name_color = color_str;
        }

        // 用户等级
        if (item.contains("user_level") && item["user_level"].is_array() && !item["user_level"].empty())
            dan->user_level = item["user_level"][0].get<int>();

        // 房管标识
        if (item.contains("isadmin") && item["isadmin"].is_number())
            dan->is_guard = item["isadmin"].get<int>();

        // 粉丝牌信息
        if (item.contains("medal") && item["medal"].is_array() && item["medal"].size() >= 3) {
            const auto& medal = item["medal"];
            dan->fan_medal_level        = medal[0].get<int>();
            dan->fan_medal_name         = medal[1].get<std::string>();
            dan->fan_medal_liveuser_name = medal[2].get<std::string>();
            if (medal.size() >= 5)
                dan->fan_medal_start_color = medal[4].get<int>();
        }

        dan_list.emplace_back(LiveDanmakuItem(dan));
    }
}

static void parse_history_danmaku_data(const bilibili::LiveHistoryDanmakuData& data, std::vector<LiveDanmakuItem>& dan_list) {
    // room 弹幕添加到后面
    append_danmaku_from_json_array(data.room, dan_list);
    
    // admin 只保留一条，插入顶部
    if (data.room.empty() && !data.admin.empty()) {
        append_danmaku_from_json_array(nlohmann::json::array({data.admin[0]}), dan_list);
    }
}

void LiveActivity::requestHistoryDanmaku(int roomid) {
    if (!this->shouldShowSidebar())
        return;

    auto state = this->threadState;

    ASYNC_RETAIN
    bilibili::HTTP::getResultAsync<bilibili::LiveHistoryDanmakuData>(
        bilibili::Api::LiveHistoryDanmaku,
        cpr::Parameters{{"roomid", std::to_string(roomid)}},
        [state, ASYNC_TOKEN](const bilibili::LiveHistoryDanmakuData& data) {
            std::vector<LiveDanmakuItem> dan_list;
            parse_history_danmaku_data(data, dan_list);
            
            if (dan_list.empty()) {
                brls::Logger::debug("LiveActivity: 未获取到历史弹幕");
                ASYNC_RELEASE
                return;
            }

            brls::Logger::debug("LiveActivity: 成功解析 {} 条历史弹幕", dan_list.size());

            brls::sync([state, dan_list = std::move(dan_list), ASYNC_TOKEN]() mutable {
                ASYNC_RELEASE
                if (!state->isActive.load(std::memory_order_acquire))
                    return;
                this->processDanmakuForSidebar(dan_list);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            ASYNC_RELEASE
            brls::Logger::error("LiveActivity: 历史弹幕请求失败: {}, code: {}", error, code);
        }
    );
}

// 更新看过人数
void LiveActivity::updateWatchedCount(int watched_count) {
    brls::Logger::debug("LiveActivity: 更新看过人数: {}", watched_count);
    
    // 格式化看过人数显示文本
    std::string watched_text = wiliwili::num2w(watched_count) + "人看过";
    
    // 更新liveData中的看过人数
    this->liveData.watched_show.text_large = watched_text;
    
    // 如果侧边栏显示，同时更新主播信息中的人数显示
    if (this->shouldShowSidebar() && this->liveAuthor) {
        // 只更新人数（misc），不要触碰头像与昵称，避免头像被置空而消失
        this->liveAuthor->getLabelMisc()->setText(watched_text);
    }
}

// 更新在线人数
void LiveActivity::updateOnlineCount(int online_count) {
    brls::Logger::debug("LiveActivity: 更新在线人数: {}", online_count);
    
    // 格式化在线人数显示文本
    std::string online_text = wiliwili::num2w(online_count) + "人在线";
    
    // 更新视频播放器显示的在线人数（优先显示在线人数而非看过人数）
    if (this->video) {
        this->video->setOnlineCount(online_text);
    }
}
