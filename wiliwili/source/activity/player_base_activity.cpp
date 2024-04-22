//
// Created by fang on 2023/1/3.
//

#include <utility>
#include <borealis/core/thread.hpp>
#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/views/applet_frame.hpp>
#include <borealis/views/dialog.hpp>

#include "activity/player_activity.hpp"
#include "fragment/player_collection.hpp"
#include "fragment/player_coin.hpp"
#include "fragment/player_single_comment.hpp"
#include "utils/config_helper.hpp"
#include "utils/dialog_helper.hpp"
#include "utils/number_helper.hpp"
#include "presenter/comment_related.hpp"
#include "view/qr_image.hpp"
#include "view/video_view.hpp"
#include "view/grid_dropdown.hpp"
#include "view/subtitle_core.hpp"
#include "view/mpv_core.hpp"

class DataSourceCommentList : public RecyclingGridDataSource, public CommentAction {
public:
    DataSourceCommentList(bilibili::VideoCommentListResult result, size_t aid, int mode, std::function<void(void)> cb)
        : dataList(std::move(result)), aid(aid), commentMode(mode), switchModeCallback(cb) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        if (index == 0) {
            VideoCommentSort* item = (VideoCommentSort*)recycler->dequeueReusableCell("Sort");
            item->setHeight(30);
            item->setFocusable(false);
            if (commentMode == 3) {
                item->hintLabel->setText("wiliwili/player/comment_sort/top"_i18n);
                item->sortLabel->setText("wiliwili/player/comment_sort/sort_top"_i18n);
            } else {
                item->hintLabel->setText("wiliwili/player/comment_sort/new"_i18n);
                item->sortLabel->setText("wiliwili/player/comment_sort/sort_new"_i18n);
            }
            return item;
        }
        if (index == 1) {
            VideoCommentReply* item = (VideoCommentReply*)recycler->dequeueReusableCell("Reply");
            item->setHeight(40);
            return item;
        }

        //从缓存列表中取出 或者 新生成一个表单项
        VideoComment* item = (VideoComment*)recycler->dequeueReusableCell("Cell");

        item->setData(this->dataList[index - 2]);
        return item;
    }

    size_t getItemCount() override { return dataList.size() + 2; }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        if (index == 0) {
            if (switchModeCallback) switchModeCallback();
            return;
        }
        if (index == 1) {
            if (!DialogHelper::checkLogin()) return;
            // 回复评论
            brls::Application::getImeManager()->openForText(
                [this, recycler](const std::string& text) {
                    if (text.empty()) return;
                    this->commentReply(text, std::to_string(aid), 0, 0, 1,
                                       [this, recycler](const bilibili::VideoCommentAddResult& result) {
                                           this->dataList.insert(dataList.begin(), result.reply);
                                           recycler->reloadData();
                                       });
                },
                "wiliwili/player/single_comment/hint"_i18n, "", 500, "", 0);
            return;
        }

        auto* item = dynamic_cast<VideoComment*>(recycler->getGridItemByIndex(index));
        if (!item) return;

        auto* view = new PlayerSingleComment();
        view->setCommentData(dataList[index - 2], item->getY(), 1);
        auto container = new brls::AppletFrame(view);
        container->setHeaderVisibility(brls::Visibility::GONE);
        container->setFooterVisibility(brls::Visibility::GONE);
        container->setInFadeAnimation(true);
        brls::Application::pushActivity(new brls::Activity(container));

        view->likeStateEvent.subscribe([this, item, index](bool value) {
            auto& itemData  = dataList[index - 2];
            itemData.action = value;
            item->setLiked(value);
        });
        view->likeNumEvent.subscribe([this, item, index](size_t value) {
            auto& itemData = dataList[index - 2];
            itemData.like  = value;
            item->setLikeNum(value);
        });
        view->replyNumEvent.subscribe([this, item, index](size_t value) {
            auto& itemData  = dataList[index - 2];
            itemData.rcount = value;
            item->setReplyNum(value);
        });
        view->deleteEvent.subscribe([this, recycler, index]() {
            dataList.erase(dataList.begin() + index - 2);
            recycler->reloadData();
            // 重新设置一下焦点到 recycler 的默认 cell （顶部）
            brls::Application::giveFocus(recycler);
        });
    }

    void appendData(const bilibili::VideoCommentListResult& data) {
        bool skip = false;
        for (auto& i : data) {
            skip = false;
            for (auto& j : this->dataList) {
                if (j.rpid == i.rpid) {
                    skip = true;
                    break;
                }
            }
            if (!skip) this->dataList.push_back(i);
        }
    }

    void clearData() override { this->dataList.clear(); }

private:
    bilibili::VideoCommentListResult dataList;
    size_t aid;
    int commentMode                              = 3;  // 2: 按时间；3: 按热度
    std::function<void(void)> switchModeCallback = nullptr;
};

class QualityCell : public RecyclingGridItem {
public:
    QualityCell() { this->inflateFromXMLRes("xml/views/player_quality_cell.xml"); }

    void setSelected(bool selected) {
        brls::Theme theme = brls::Application::getTheme();

        this->selected = selected;
        this->checkbox->setVisibility(selected ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
        this->title->setTextColor(selected ? theme["brls/list/listItem_value_color"] : theme["brls/text"]);
    }

    bool getSelected() { return this->selected; }

    BRLS_BIND(brls::Label, title, "cell/title");
    BRLS_BIND(brls::Box, loginLabel, "cell/login");
    BRLS_BIND(brls::Box, vipLabel, "cell/vip");
    BRLS_BIND(brls::CheckBox, checkbox, "cell/checkbox");

    static RecyclingGridItem* create() { return new GridRadioCell(); }

private:
    bool selected = false;
};

class QualityDataSource : public DataSourceDropdown {
public:
    QualityDataSource(bilibili::VideoUrlResult result, BaseDropdown* view)
        : DataSourceDropdown(view), data(std::move(result)) {
        login = !ProgramConfig::instance().getCSRF().empty();
    }

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        QualityCell* item = (QualityCell*)recycler->dequeueReusableCell("Cell");

        int quality = data.accept_quality[index];

        item->loginLabel->setVisibility(brls::Visibility::GONE);
        item->vipLabel->setVisibility(brls::Visibility::GONE);

        if (quality > 80) {
            item->vipLabel->setVisibility(brls::Visibility::VISIBLE);
        } else if (quality >= 32) {
            if (!login) item->loginLabel->setVisibility(brls::Visibility::VISIBLE);
        }

        auto r = this->data.accept_description[index];
        item->title->setText(this->data.accept_description[index]);
        item->setSelected(index == dropdown->getSelected());
        return item;
    }

    size_t getItemCount() override { return (std::min)(data.accept_quality.size(), data.accept_description.size()); }

    void clearData() override {}

private:
    bilibili::VideoUrlResult data;
    bool login;
};

/// BasePlayerActivity

void BasePlayerActivity::onContentAvailable() { this->setCommonData(); }

void BasePlayerActivity::setCommonData() {
    // 视频评论
    recyclingGrid->registerCell("Cell", []() { return VideoComment::create(); });

    recyclingGrid->registerCell("Reply", []() { return VideoCommentReply::create(); });

    recyclingGrid->registerCell("Sort", []() { return VideoCommentSort::create(); });

    recyclingGrid->setDefaultCellFocus(1);

    recyclingGrid->registerAction("wiliwili/home/common/switch"_i18n, brls::ControllerButton::BUTTON_X,
                                  [this](brls::View* view) -> bool {
                                      this->setCommentMode();
                                      return true;
                                  });

    // 切换右侧Tab
    this->registerAction(
        "上一项", brls::ControllerButton::BUTTON_LT,
        [this](brls::View* view) -> bool {
            tabFrame->focus2LastTab();
            return true;
        },
        true);
    this->registerAction(
        "上一项", brls::ControllerButton::BUTTON_LB,
        [this](brls::View* view) -> bool {
            tabFrame->focus2LastTab();
            return true;
        },
        true);

    this->registerAction(
        "下一项", brls::ControllerButton::BUTTON_RT,
        [this](brls::View* view) -> bool {
            tabFrame->focus2NextTab();
            return true;
        },
        true);
    this->registerAction(
        "下一项", brls::ControllerButton::BUTTON_RB,
        [this](brls::View* view) -> bool {
            tabFrame->focus2NextTab();
            return true;
        },
        true);

    // 调整清晰度
    this->registerAction("wiliwili/player/quality"_i18n, brls::ControllerButton::BUTTON_START,
                         [this](brls::View* view) -> bool {
                             this->setVideoQuality();
                             return true;
                         });

    // 暂停
    this->registerAction("toggle", brls::ControllerButton::BUTTON_SPACE, [this](...) -> bool {
        this->video->togglePlay();
        return true;
    }, true);

    this->btnQR->getParent()->addGestureRecognizer(new brls::TapGestureRecognizer(this->btnQR->getParent()));

    this->btnAgree->getParent()->addGestureRecognizer(new brls::TapGestureRecognizer(this->btnAgree->getParent()));

    this->btnCoin->getParent()->addGestureRecognizer(new brls::TapGestureRecognizer(this->btnCoin->getParent()));

    this->btnFavorite->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnFavorite->getParent()));

    this->videoUserInfo->addGestureRecognizer(new brls::TapGestureRecognizer(this->videoUserInfo));

    this->setRelationButton(false, false, false);

    eventSubscribeID = MPV_E->subscribe([this](MpvEventEnum event) {
        // 上一次报告历史记录的时间点
        static int64_t lastProgress = MPVCore::instance().video_progress;
        switch (event) {
            case MpvEventEnum::UPDATE_PROGRESS: {
                // 每15秒同步一次进度
                if (lastProgress + 15 < MPVCore::instance().video_progress) {
                    lastProgress = MPVCore::instance().video_progress;
                    this->reportCurrentProgress(lastProgress, MPVCore::instance().duration);
                } else if (MPVCore::instance().video_progress < lastProgress) {
                    // 当前播放时间小于上一次上传历史记录的时间点
                    // 发生于向前拖拽进度的时候，此时重置lastProgress的值
                    lastProgress = MPVCore::instance().video_progress;
                }
                // 检查视频链接是否有效
                auto timeNow = std::chrono::system_clock::now();
                if (timeNow > videoDeadline) {
                    // 设置视频加载后跳转的时间
                    setProgress(MPVCore::instance().video_progress);

                    // 暂停播放
                    MPVCore::instance().pause();

                    // 10s 后重新尝试
                    videoDeadline = timeNow + std::chrono::seconds(10);

                    // 有效期已过，重新请求视频链接
                    auto self = dynamic_cast<PlayerSeasonActivity*>(this);
                    if (self) {
                        this->requestSeasonVideoUrl(episodeResult.bvid, episodeResult.cid, false);
                    } else {
                        this->requestVideoUrl(videoDetailResult.bvid, videoDetailPage.cid, false);
                    }

                    //todo: 如果有选择的字幕加载对应的字幕
                }
                break;
            }
            case MpvEventEnum::END_OF_FILE:
                // 尝试自动加载下一分集
                // 如果当前最顶层是Dialog就放弃自动播放，因为有可能是用户点开了收藏或者投币对话框
                {
                    int64_t& progress = MPVCore::instance().video_progress;
                    int64_t& duration = MPVCore::instance().duration;
                    int clipEnd       = videoUrlResult.clipEnd;
                    auto seasonCustom = ProgramConfig::instance().getSeasonCustom(seasonInfo.season_id);
                    if (seasonCustom.custom_clip) {
                        clipEnd = duration - seasonCustom.clip_end;
                    }

                    // 播放到一半没网时也会触发EOF，这里简单判断一下结束播放时的播放条位置是否在片尾或视频结尾附近
                    if (fabs(duration - progress) > 5 && !(clipEnd > 0 && clipEnd - progress < 5)) {
                        brls::Logger::error("EOF: video: {} duration: {} clipEnd: {}", progress, duration, clipEnd);
                        return;
                    }
                    if (PLAYER_STRATEGY == PlayerStrategy::LOOP) {
                        MPVCore::instance().seek(0);
                        return;
                    }
                    auto stack    = brls::Application::getActivitiesStack();
                    Activity* top = stack[stack.size() - 1];
                    if (!dynamic_cast<BasePlayerActivity*>(top) &&
                        !dynamic_cast<VideoView*>(top->getContentView()->getView("video"))) {
                        // 最顶层没有 video 组件，说明用户打开了评论或者其他菜单
                        // 在这种情况下不执行自动播放其他视频显示重播按钮
                        APP_E->fire(VideoView::REPLAY, nullptr);
                    } else if (PLAYER_STRATEGY == PlayerStrategy::NEXT || PLAYER_STRATEGY == PlayerStrategy::RCMD) {
                        this->onIndexChangeToNext();
                    } else {
                        // 对于其他情况，显示重播按钮
                        APP_E->fire(VideoView::REPLAY, nullptr);
                    }
                }
                break;
            default:
                break;
        }
    });

    customEventSubscribeID = APP_E->subscribe([this](const std::string& event, void* data) {
        if (event == VideoView::QUALITY_CHANGE) {
            this->setVideoQuality();
        } else if (event == "REQUEST_CAST_URL") {
            this->requestCastUrl();
        }
    });

    if (brls::Application::ORIGINAL_WINDOW_HEIGHT < 720) video->hideStatusLabel();

    video->hideOSDLockButton();
}

void BasePlayerActivity::showCollectionDialog(int64_t id, int videoType) {
    if (!DialogHelper::checkLogin()) return;
    auto playerCollection = new PlayerCollection(id, videoType);
    auto dialog           = new brls::Dialog(playerCollection);
    dialog->addButton("wiliwili/home/common/save"_i18n, [this, id, videoType, playerCollection]() {
        this->addResource(id, videoType, playerCollection->isFavorite(), playerCollection->getAddCollectionList(),
                          playerCollection->getDeleteCollectionList());
    });
    playerCollection->registerAction(
        "", brls::ControllerButton::BUTTON_START,
        [this, id, videoType, playerCollection, dialog](...) {
            this->addResource(id, videoType, playerCollection->isFavorite(), playerCollection->getAddCollectionList(),
                              playerCollection->getDeleteCollectionList());
            dialog->dismiss();
            return true;
        },
        true);
    dialog->open();
}

void BasePlayerActivity::showCoinDialog(size_t aid) {
    if (!DialogHelper::checkLogin()) return;

    if (std::to_string(videoDetailResult.owner.mid) == ProgramConfig::instance().getUserID()) {
        DialogHelper::showDialog("wiliwili/player/coin/own"_i18n);
        return;
    }

    int coins = getCoinTolerate();
    if (coins <= 0) {
        DialogHelper::showDialog("wiliwili/player/coin/run_out"_i18n);
        return;
    }

    auto playerCoin = new PlayerCoin();
    if (coins == 1) playerCoin->hideTwoCoin();
    playerCoin->getSelectEvent()->subscribe(
        [this, playerCoin, aid](int value) { this->addCoin(aid, value, playerCoin->likeAtTheSameTime()); });
    auto dialog = new brls::Dialog(playerCoin);
    dialog->open();
}

void BasePlayerActivity::setVideoQuality() {
    if (this->videoUrlResult.accept_description.empty()) return;

    auto* dropdown = new BaseDropdown(
        "wiliwili/player/quality"_i18n,
        [this](int selected) {
            int code                           = this->videoUrlResult.accept_quality[selected];
            BasePlayerActivity::defaultQuality = code;
            ProgramConfig::instance().setSettingItem(SettingItem::VIDEO_QUALITY, code);

            // 如果未登录选择了大于等于480P清晰度的视频
            if (ProgramConfig::instance().getCSRF().empty() && defaultQuality >= 32) {
                DialogHelper::showDialog("wiliwili/home/common/no_login"_i18n);
                return;
            }

            // 设置视频加载后跳转的时间
            setProgress(MPVCore::instance().video_progress);

            // dash
            if (!this->videoUrlResult.dash.video.empty()) {
                // dash格式的视频无需重复请求视频链接，这里简单的设置清晰度即可
                videoUrlResult.quality = BasePlayerActivity::defaultQuality;
                this->onVideoPlayUrl(videoUrlResult);
                return;
            }

            // flv
            auto self = dynamic_cast<PlayerSeasonActivity*>(this);
            if (self) {
                this->requestSeasonVideoUrl(episodeResult.bvid, episodeResult.cid);
            } else {
                this->requestVideoUrl(videoDetailResult.bvid, videoDetailPage.cid);
            }
        },
        getQualityIndex());
    auto* recycler = dropdown->getRecyclingList();
    recycler->registerCell("Cell", []() { return new QualityCell(); });
    dropdown->setDataSource(new QualityDataSource(this->videoUrlResult, dropdown));
    dropdown->registerAction(
        "", brls::ControllerButton::BUTTON_START,
        [dropdown](...) {
            dropdown->dismiss();
            return true;
        },
        true);

    // 因为触摸的问题 视频组件上开启新的 activity 需要同步执行
    // 不然在某些情况下焦点会错乱
    ASYNC_RETAIN
    brls::sync([ASYNC_TOKEN, dropdown]() {
        ASYNC_RELEASE
        brls::Application::pushActivity(new brls::Activity(dropdown));
    });
}

void BasePlayerActivity::setCommentMode() {
    this->recyclingGrid->estimatedRowHeight = 100;
    this->recyclingGrid->showSkeleton();
    tabFrame->focusTab(0);
    requestVideoComment(std::to_string(this->getAid()), 0, getVideoCommentMode() == 3 ? 2 : 3);
}

void BasePlayerActivity::onVideoPlayUrl(const bilibili::VideoUrlResult& result) {
    brls::Logger::debug("onVideoPlayUrl quality: {}", result.quality);

    // 有效期 110 分钟
    videoDeadline = std::chrono::system_clock::now() + std::chrono::seconds(6600);

    // 获取预设的跳转位置
    int start    = this->getProgress();
    int end      = -1;
    int clipOpen = result.clipOpen, clipEnd = result.clipEnd;
    int time_sec = result.timelength / 1000;
    std::string customAspect;

    // 加载覆盖设置
    if (seasonInfo.season_id > 0) {
        auto seasonSetting = ProgramConfig::instance().getSeasonCustom(seasonInfo.season_id);
        customAspect       = seasonSetting.player_aspect;

        if (seasonSetting.custom_clip) {
            // 设置了自定义的片头片尾
            // 判断是否是正片
            for (auto& e : seasonInfo.episodes) {
                if (episodeResult.id == e.id) {
                    // 如果当前播放的是正片，则使用自定义的片头片尾
                    clipOpen = seasonSetting.clip_start;
                    if (clipOpen > time_sec - 5) clipOpen = 0;
                    clipEnd = time_sec - seasonSetting.clip_end;
                    if (seasonSetting.clip_end <= 0 || clipEnd < clipOpen) clipEnd = 0;
                    break;
                }
            }
        }
    }
    if (customAspect.empty()) {
        customAspect = ProgramConfig::instance().getSettingItem(SettingItem::PLAYER_ASPECT, std::string{"-1"});
    }
    MPVCore::instance().setAspect(customAspect);

    // 当要跳转的进度距离尾部只有 5s，就重新播放
    if (start > 0 && abs(time_sec - start) <= 5) start = 0;

    // 跳过片头片尾
    if (PLAYER_SKIP_OPENING_CREDITS) {
        start = std::max(start, clipOpen);
        if (clipEnd > 0) start = std::min(start, clipEnd);
        end = clipEnd;
        brls::Logger::debug("片头片尾: {}/{}", clipOpen, clipEnd);
    }

    // 针对用户上传的视频，尝试加载上一次播放的进度
    if (videoDetailPage.cid && start < 0) {
        auto data = SubtitleCore::instance().getSubtitleList();
        if (data.last_play_cid == videoDetailPage.cid && data.last_play_time > 0) {
            APP_E->fire(VideoView::LAST_TIME, (void*)&(data.last_play_time));
        }
    } else {
        // 设置为 POSITION_DISCARD 后，不会加载网络历史记录，而是直接使用 setProgress 指定的位置
        // 番剧视频或指定了进度的用户视频
        int64_t position = 0;
        APP_E->fire(VideoView::LAST_TIME, (void*)&(position));
        if (start > 0) {
            std::string hint = fmt::format("已为您定位至: {}", wiliwili::sec2Time(start));
            APP_E->fire(VideoView::HINT, (void*)hint.c_str());
        }
    }

    if (!result.dash.video.empty()) {
        // dash
        brls::Logger::debug("Video type: dash");

        // 找到当前可用的清晰度
        for (const auto& i : result.dash.video) {
            if (result.quality >= i.id) {
                videoUrlResult.quality = i.id;
                break;
            }
        }

        // 找到当前清晰度下可用的视频
        std::vector<bilibili::DashMedia> codecs;
        for (const auto& i : result.dash.video) {
            if (i.id == videoUrlResult.quality) {
                codecs.emplace_back(i);
            }
        }

        // 匹配当前设定的视频编码
        bilibili::DashMedia v = codecs[0];  // 默认是 AVC/H.264
        for (const auto& i : codecs) {
            if (BILI::VIDEO_CODEC == i.codecid) {
                v = i;
                break;
            }
        }

        // 将主音频和备份音频链接合并，当作不同的音轨传给播放器，可以实现在播放失败时自动切换
        std::vector<std::string> audios;
        if (!result.dash.audio.empty()) {
            // 匹配当前设定的音频码率
            bilibili::DashMedia a = result.dash.audio[0];  // High
            for (auto& i : result.dash.audio) {
                if (BILI::AUDIO_QUALITY == i.id) {
                    a = i;
                    break;
                }
            }
            // 生成音频列表
            audios.emplace_back(a.base_url);
            audios.insert(audios.end(), a.backup_url.begin(), a.backup_url.end());
            brls::Logger::debug("Dash quality: {}; video: {}; audio: {}", videoUrlResult.quality, v.codecid, a.id);
        }

        // 给播放器设置链接
        this->video->setUrl(v.base_url, start, end, audios);

        // 设置备份视频链接
        for (const auto& backup_url : v.backup_url) {
            this->video->setBackupUrl(backup_url, start, end, audios);
        }
    } else {
        // flv
        brls::Logger::debug("Video type: flv");
        if (result.durl.empty()) {
            brls::Logger::error("No media");
        } else if (result.durl.size() == 1) {
            this->video->setUrl(result.durl[0].url, start, end);
        } else {
            std::vector<EDLUrl> urls;
            urls.reserve(result.durl.size());
            for (auto& i : result.durl) {
                urls.emplace_back(i.url, i.length / 1000.0f);
            }
            this->video->setUrl(urls, start, end);
        }
    }

    // 设置mpv事件
    // 1.更新清晰度
    std::string quality = videoUrlResult.accept_description[getQualityIndex()];
    APP_E->fire(VideoView::SET_QUALITY, (void*)quality.c_str());
    // 2.绘制进度条标记点（例如：片头片尾）
    if (clipOpen > 0) {
        float data = clipOpen * 1.0f / time_sec;
        APP_E->fire(VideoView::CLIP_INFO, (void*)&data);
    }
    if (clipEnd > 0) {
        float data2 = clipEnd * 1.0f / time_sec;
        APP_E->fire(VideoView::CLIP_INFO, (void*)&data2);
    }
    // 3. 设置视频时长
    APP_E->fire(VideoView::REAL_DURATION, (void*)&time_sec);

    brls::Logger::debug("BasePlayerActivity::onVideoPlayUrl done");
}

void BasePlayerActivity::onCommentInfo(const bilibili::VideoCommentResultWrapper& result) {
    auto* datasource = dynamic_cast<DataSourceCommentList*>(recyclingGrid->getDataSource());
    if (!datasource && result.requestIndex == 0) {
        // 第一页评论
        //整合置顶评论
        std::vector<bilibili::VideoCommentResult> comments(result.top_replies);
        comments.insert(comments.end(), result.replies.begin(), result.replies.end());
        // 为了加载骨架屏美观，设置为了100，在加载评论时手动修改回来
        // 这里限制的是评论的最大高度，实际评论高度还受评论组件的最大行数限制
        this->recyclingGrid->estimatedRowHeight = 600;
        this->recyclingGrid->setDataSource(new DataSourceCommentList(
            comments, this->getAid(), this->getVideoCommentMode(), [this]() { this->setCommentMode(); }));
        this->recyclingGrid->selectRowAt(comments.empty() ? 1 : 2, false);
        // 设置评论数量提示
        auto item = this->tabFrame->getTab("wiliwili/player/comment"_i18n);
        if (item) item->setSubtitle(wiliwili::num2w(result.cursor.all_count));
    } else if (datasource) {
        // 第N页评论
        if (!result.replies.empty()) {
            datasource->appendData(result.replies);
            recyclingGrid->notifyDataChanged();
        }
    } else {
        brls::Logger::error("onCommentInfo ds: {} index: {} end: {}", (bool)datasource, result.requestIndex,
                            result.cursor.is_end);
    }
}

void BasePlayerActivity::onRequestCommentError(const std::string& error) {
    brls::sync([this, error]() { this->recyclingGrid->setError(error); });
}

void BasePlayerActivity::onVideoOnlineCount(const bilibili::VideoOnlineTotal& result) {
    std::string count = result.total + "wiliwili/player/current"_i18n;
    this->videoPeopleLabel->setText(count);
    APP_E->fire(VideoView::SET_ONLINE_NUM, (void*)count.c_str());
}

void BasePlayerActivity::onVideoRelationInfo(const bilibili::VideoRelation& result) {
    brls::Logger::debug("onVideoRelationInfo: {} {} {}", result.like, result.coin, result.favorite);
    this->setRelationButton(result.like, result.coin, result.favorite);
}

void BasePlayerActivity::onHighlightProgress(const bilibili::VideoHighlightProgress& result) {
    brls::Logger::debug("highlight: {}/{}", result.step_sec, result.data.size());
    VideoHighlightData data{result.step_sec, result.data};
    APP_E->fire(VideoView::HIGHLIGHT_INFO, (void*)&data);
}

void BasePlayerActivity::setRelationButton(bool liked, bool coin, bool favorite) {
    if (liked) {
        btnAgree->setImageFromSVGRes("svg/bpx-svg-sprite-liked-active.svg");
    } else {
        btnAgree->setImageFromSVGRes("svg/bpx-svg-sprite-liked.svg");
    }
    if (coin) {
        btnCoin->setImageFromSVGRes("svg/bpx-svg-sprite-coin-active.svg");
    } else {
        btnCoin->setImageFromSVGRes("svg/bpx-svg-sprite-coin.svg");
    }
    if (favorite) {
        btnFavorite->setImageFromSVGRes("svg/bpx-svg-sprite-collection-active.svg");
    } else {
        btnFavorite->setImageFromSVGRes("svg/bpx-svg-sprite-collection.svg");
    }
}

void BasePlayerActivity::onError(const std::string& error) {
    if (!activityShown) return;
    MPVCore::instance().stop();
    MPVCore::instance().reset();
    bool forceClose = true;
    std::string msg = error;
    if (pystring::count(error, "87007") > 0 || pystring::count(error, "87008") > 0) {
        forceClose = false;
        msg        = "该视频为「充电」专属视频";
    } else if (pystring::count(error, "10403") > 0) {
        forceClose = false;
        msg        = "大会员专享限制";
    } else if (pystring::count(error, "404") > 0) {
        msg = "啥都木有";
    } else if (pystring::count(error, "62002") > 0) {
        msg = "稿件不可见";
    }
    auto dialog = new brls::Dialog(msg);
    dialog->setCancelable(false);
    dialog->addButton("hints/ok"_i18n, [forceClose]() {
        if (forceClose) brls::sync([]() { brls::Application::popActivity(); });
    });
    dialog->open();
}

void BasePlayerActivity::willDisappear(bool resetState) {
    activityShown = false;
    brls::Activity::willDisappear(resetState);
}

void BasePlayerActivity::willAppear(bool resetState) {
    activityShown = true;
    brls::Activity::willAppear(resetState);
}

BasePlayerActivity::~BasePlayerActivity() {
    brls::Logger::debug("del BasePlayerActivity");
    // 取消监控mpv
    MPV_E->unsubscribe(eventSubscribeID);
    APP_E->unsubscribe(customEventSubscribeID);
    // 停止视频播放
    this->video->stop();
}