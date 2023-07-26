//
// Created by fang on 2023/1/3.
//

#include <utility>

#include "activity/player_activity.hpp"
#include "fragment/player_collection.hpp"
#include "fragment/player_coin.hpp"
#include "fragment/player_single_comment.hpp"
#include "view/qr_image.hpp"
#include "view/video_view.hpp"
#include "view/grid_dropdown.hpp"
#include "view/subtitle_core.hpp"
#include "utils/config_helper.hpp"
#include "utils/dialog_helper.hpp"
#include "utils/number_helper.hpp"
#include "presenter/comment_related.hpp"

class DataSourceCommentList : public RecyclingGridDataSource,
                              public CommentRequest {
public:
    DataSourceCommentList(bilibili::VideoCommentListResult result, size_t aid,
                          int mode, std::function<void(void)> cb)
        : dataList(std::move(result)),
          aid(aid),
          commentMode(mode),
          switchModeCallback(cb) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler,
                                  size_t index) override {
        if (index == 0) {
            VideoCommentSort* item =
                (VideoCommentSort*)recycler->dequeueReusableCell("Sort");
            item->setHeight(30);
            item->setFocusable(false);
            if (commentMode == 3) {
                item->hintLabel->setText(
                    "wiliwili/player/comment_sort/top"_i18n);
                item->sortLabel->setText(
                    "wiliwili/player/comment_sort/sort_top"_i18n);
            } else {
                item->hintLabel->setText(
                    "wiliwili/player/comment_sort/new"_i18n);
                item->sortLabel->setText(
                    "wiliwili/player/comment_sort/sort_new"_i18n);
            }
            return item;
        }
        if (index == 1) {
            VideoCommentReply* item =
                (VideoCommentReply*)recycler->dequeueReusableCell("Reply");
            item->setHeight(40);
            return item;
        }

        //从缓存列表中取出 或者 新生成一个表单项
        VideoComment* item =
            (VideoComment*)recycler->dequeueReusableCell("Cell");

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
                [this, recycler](std::string text) {
                    this->commentReply(
                        text, aid, 0, 0,
                        [this, recycler](
                            const bilibili::VideoCommentAddResult& result) {
                            this->dataList.insert(dataList.begin(),
                                                  result.reply);
                            recycler->reloadData();
                        });
                },
                "", "", 500, "", 0);
            return;
        }

        auto* item =
            dynamic_cast<VideoComment*>(recycler->getGridItemByIndex(index));
        if (!item) return;

        auto* view = new PlayerSingleComment();
        view->setCommentData(dataList[index - 2], item->getY());
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
    int commentMode = 3;  // 2: 按时间；3: 按热度
    std::function<void(void)> switchModeCallback = nullptr;
};

class QualityCell : public RecyclingGridItem {
public:
    QualityCell() {
        this->inflateFromXMLRes("xml/views/player_quality_cell.xml");
    }

    void setSelected(bool selected) {
        brls::Theme theme = brls::Application::getTheme();

        this->selected = selected;
        this->checkbox->setVisibility(selected ? brls::Visibility::VISIBLE
                                               : brls::Visibility::GONE);
        this->title->setTextColor(selected
                                      ? theme["brls/list/listItem_value_color"]
                                      : theme["brls/text"]);
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

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler,
                                  size_t index) override {
        QualityCell* item = (QualityCell*)recycler->dequeueReusableCell("Cell");

        int quality = data.accept_quality[index];

        item->loginLabel->setVisibility(brls::Visibility::GONE);
        item->vipLabel->setVisibility(brls::Visibility::GONE);

        if (quality > 80) {
            item->vipLabel->setVisibility(brls::Visibility::VISIBLE);
        } else if (quality >= 32) {
            if (!login)
                item->loginLabel->setVisibility(brls::Visibility::VISIBLE);
        }

        auto r = this->data.accept_description[index];
        item->title->setText(this->data.accept_description[index]);
        item->setSelected(index == dropdown->getSelected());
        return item;
    }

    size_t getItemCount() override {
        return (std::min)(data.accept_quality.size(),
                        data.accept_description.size());
    }

    void clearData() override {}

private:
    bilibili::VideoUrlResult data;
    bool login;
};

/// BasePlayerActivity

void BasePlayerActivity::onContentAvailable() { this->setCommonData(); }

void BasePlayerActivity::setCommonData() {
    // 视频评论
    recyclingGrid->registerCell("Cell",
                                []() { return VideoComment::create(); });

    recyclingGrid->registerCell("Reply",
                                []() { return VideoCommentReply::create(); });

    recyclingGrid->registerCell("Sort",
                                []() { return VideoCommentSort::create(); });

    recyclingGrid->setDefaultCellFocus(1);

    recyclingGrid->registerAction("wiliwili/home/common/switch"_i18n,
                                  brls::ControllerButton::BUTTON_X,
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
    this->registerAction("wiliwili/player/quality"_i18n,
                         brls::ControllerButton::BUTTON_START,
                         [this](brls::View* view) -> bool {
                             this->setVideoQuality();
                             return true;
                         });

    this->btnQR->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnQR->getParent()));

    this->btnAgree->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnAgree->getParent()));

    this->btnCoin->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnCoin->getParent()));

    this->btnFavorite->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnFavorite->getParent()));

    this->videoUserInfo->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->videoUserInfo));

    this->setRelationButton(false, false, false);

    eventSubscribeID = MPV_E->subscribe([this](MpvEventEnum event) {
        // 上一次报告历史记录的时间点
        static int64_t lastProgress = MPVCore::instance().video_progress;
        switch (event) {
            case MpvEventEnum::UPDATE_PROGRESS:
                // 每15秒同步一次进度
                if (lastProgress + 15 < MPVCore::instance().video_progress) {
                    lastProgress = MPVCore::instance().video_progress;
                    this->reportCurrentProgress(lastProgress,
                                                MPVCore::instance().duration);
                } else if (MPVCore::instance().video_progress < lastProgress) {
                    // 当前播放时间小于上一次上传历史记录的时间点
                    // 发生于向前拖拽进度的时候，此时重置lastProgress的值
                    lastProgress = MPVCore::instance().video_progress;
                }
                break;
            case MpvEventEnum::END_OF_FILE:
                // 尝试自动加载下一分集
                // 如果当前最顶层是Dialog就放弃自动播放，因为有可能是用户点开了收藏或者投币对话框
                {
                    auto stack    = brls::Application::getActivitiesStack();
                    Activity* top = stack[stack.size() - 1];
                    if (!dynamic_cast<BasePlayerActivity*>(top)) {
                        // 判断最顶层是否为video
                        if (!dynamic_cast<VideoView*>(
                                top->getContentView()->getView("video")))
                            return;
                    }
                    if (AUTO_NEXT_PART) this->onIndexChangeToNext();
                }
                break;
            default:
                break;
        }
    });

    customEventSubscribeID =
        MPV_CE->subscribe([this](const std::string& event, void* data) {
            if (event == VideoView::QUALITY_CHANGE) {
                this->setVideoQuality();
            } else if (event == "REQUEST_CAST_URL") {
                this->requestCastUrl();
            }
        });
}

void BasePlayerActivity::showShareDialog(const std::string& link) {
    auto container = new brls::Box(brls::Axis::COLUMN);
    container->setJustifyContent(brls::JustifyContent::CENTER);
    container->setAlignItems(brls::AlignItems::CENTER);
    auto qr = new QRImage();
    qr->setSize(brls::Size(256, 256));
    qr->setImageFromQRContent(link);
    qr->setMargins(20, 10, 10, 10);
    container->addView(qr);
    auto hint = new brls::Label();
    hint->setText("wiliwili/player/qr"_i18n);
    hint->setMargins(0, 10, 10, 10);
    container->addView(hint);
    auto dialog = new brls::Dialog(container);
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->open();
}

void BasePlayerActivity::showCollectionDialog(int64_t id, int videoType) {
    if (!DialogHelper::checkLogin()) return;
    auto playerCollection = new PlayerCollection(id, videoType);
    auto dialog           = new brls::Dialog(playerCollection);
    dialog->addButton("wiliwili/home/common/save"_i18n, [this, id, videoType,
                                                         playerCollection]() {
        this->addResource(id, videoType, playerCollection->isFavorite(),
                          playerCollection->getAddCollectionList(),
                          playerCollection->getDeleteCollectionList());
    });
    playerCollection->registerAction(
        "", brls::ControllerButton::BUTTON_START,
        [this, id, videoType, playerCollection, dialog](...) {
            this->addResource(id, videoType, playerCollection->isFavorite(),
                              playerCollection->getAddCollectionList(),
                              playerCollection->getDeleteCollectionList());
            dialog->dismiss();
            return true;
        },
        true);
    dialog->open();
}

void BasePlayerActivity::showCoinDialog(size_t aid) {
    if (!DialogHelper::checkLogin()) return;

    if (std::to_string(videoDetailResult.owner.mid) ==
        ProgramConfig::instance().getUserID()) {
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
    playerCoin->getSelectEvent()->subscribe([this, playerCoin, aid](int value) {
        this->addCoin(aid, value, playerCoin->likeAtTheSameTime());
    });
    auto dialog = new brls::Dialog(playerCoin);
    dialog->open();
}

void BasePlayerActivity::setVideoQuality() {
    if (this->videoUrlResult.accept_description.empty()) return;

    auto* dropdown = new BaseDropdown(
        "wiliwili/player/quality"_i18n,
        [this](int selected) {
            int code = this->videoUrlResult.accept_quality[selected];
            BasePlayerActivity::defaultQuality = code;
            ProgramConfig::instance().setSettingItem(SettingItem::VIDEO_QUALITY,
                                                     code);

            // 如果未登录选择了大于等于480P清晰度的视频
            if (ProgramConfig::instance().getCSRF().empty() &&
                defaultQuality >= 32) {
                DialogHelper::showDialog("wiliwili/home/common/no_login"_i18n);
                return;
            }

            // 在加载视频时，若设置了进度，会自动向前跳转5秒，
            // 这里提前加上5s用来抵消播放视频时的进度问题。
            setProgress(MPVCore::instance().video_progress + 5);

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
                this->requestSeasonVideoUrl(episodeResult.bvid,
                                            episodeResult.cid);
            } else {
                this->requestVideoUrl(videoDetailResult.bvid,
                                      videoDetailPage.cid);
            }
        },
        getQualityIndex());
    auto* recycler = dropdown->getRecyclingList();
    recycler->registerCell("Cell", []() { return new QualityCell(); });
    dropdown->setDataSource(
        new QualityDataSource(this->videoUrlResult, dropdown));

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
    requestVideoComment(this->getAid(), 0, getVideoCommentMode() == 3 ? 2 : 3);
}

void BasePlayerActivity::onVideoPlayUrl(
    const bilibili::VideoUrlResult& result) {
    brls::Logger::debug("onVideoPlayUrl quality: {}", result.quality);

    // 进度向前回退5秒，避免当前进度过于接近结尾出现一加载就结束的情况
    int progress = this->getProgress() - 5;

    // 针对用户上传的视频，尝试加载上一次播放的进度
    if (videoDetailPage.cid && progress <= 0) {
        auto data = SubtitleCore::instance().getSubtitleList();
        if (data.last_play_cid == videoDetailPage.cid &&
            data.last_play_time > 0) {
            this->video->setLastPlayedPosition(data.last_play_time / 1000);
        }
    } else {
        // 设置为 POSITION_DISCARD 后，不会加载网络历史记录，而是直接使用 setProgress 指定的位置
        // 一般来说 setProgress 是打开播放页面时指定的播放时间，通常是从历史记录页面进入时设置的
        this->video->setLastPlayedPosition(VideoView::POSITION_DISCARD);
        if (progress > 0)
            MPV_CE->fire(VideoView::HINT,
                         (void*)fmt::format("已为您定位至: {}",
                                            wiliwili::sec2Time(progress))
                             .c_str());
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

        // 给播放器设置链接
        if (result.dash.audio.empty()) {
            // 无音频视频
            this->video->setUrl(v.base_url, progress);
            for (auto& url : v.backup_url) {
                this->video->setBackupUrl(url, progress);
            }
            brls::Logger::debug("Dash quality: {}; video: {}",
                                videoUrlResult.quality, v.codecid);
        } else {
            // 匹配当前设定的音频码率
            bilibili::DashMedia a = result.dash.audio[0];  // High
            for (auto& i : result.dash.audio) {
                if (BILI::AUDIO_QUALITY == i.id) {
                    a = i;
                    break;
                }
            }

            // 将主音频和备份音频链接合并，当作不同的音轨传给播放器，在播放失败时自动切换
            std::vector<std::string> audios = {a.base_url};
            audios.insert(audios.end(), a.backup_url.begin(),
                          a.backup_url.end());

            this->video->setUrl(v.base_url, progress, audios);

            // 设置备份视频链接
            for (size_t i = 0; i < v.backup_url.size(); ++i) {
                this->video->setBackupUrl(v.backup_url[i], progress, audios);
            }
            brls::Logger::debug("Dash quality: {}; video: {}; audio: {}",
                                videoUrlResult.quality, v.codecid, a.id);
        }
    } else {
        // flv
        brls::Logger::debug("Video type: flv");
        if (result.durl.empty()) {
            brls::Logger::error("No media");
        } else if (result.durl.size() == 1) {
            this->video->setUrl(result.durl[0].url, progress);
        } else {
            std::vector<EDLUrl> urls;
            for (auto& i : result.durl) {
                urls.emplace_back(EDLUrl(i.url, i.length / 1000.0f));
            }
            this->video->setUrl(urls, progress);
        }
    }

    // 设置mpv事件，更新清晰度
    std::string quality = videoUrlResult.accept_description[getQualityIndex()];
    MPV_CE->fire(VideoView::SET_QUALITY, (void*)quality.c_str());

    brls::Logger::debug("BasePlayerActivity::onVideoPlayUrl done");
}

void BasePlayerActivity::onCommentInfo(
    const bilibili::VideoCommentResultWrapper& result) {
    auto* datasource =
        dynamic_cast<DataSourceCommentList*>(recyclingGrid->getDataSource());
    if (!datasource && result.requestIndex == 0) {
        // 第一页评论
        //整合置顶评论
        std::vector<bilibili::VideoCommentResult> comments(result.top_replies);
        comments.insert(comments.end(), result.replies.begin(),
                        result.replies.end());
        // 为了加载骨架屏美观，设置为了100，在加载评论时手动修改回来
        // 这里限制的是评论的最大高度，实际评论高度还受评论组件的最大行数限制
        this->recyclingGrid->estimatedRowHeight = 600;
        this->recyclingGrid->setDataSource(new DataSourceCommentList(
            comments, this->getAid(), this->getVideoCommentMode(),
            [this]() { this->setCommentMode(); }));
        this->recyclingGrid->selectRowAt(comments.empty() ? 1 : 2, false);
        // 设置评论数量提示
        auto item = this->tabFrame->getTab("wiliwili/player/comment"_i18n);
        if (item) item->setSubtitle(wiliwili::num2w(result.cursor.all_count));
    } else if (datasource) {
        // 第N页评论
        datasource->appendData(result.replies);
        recyclingGrid->notifyDataChanged();
    } else {
        brls::Logger::error("onCommentInfo ds: {} index: {} end: {}",
                            (bool)datasource, result.requestIndex,
                            result.cursor.is_end);
    }
}

void BasePlayerActivity::onRequestCommentError(const std::string& error) {
    brls::sync([this, error]() { this->recyclingGrid->setError(error); });
}

void BasePlayerActivity::onVideoOnlineCount(
    const bilibili::VideoOnlineTotal& result) {
    std::string count = result.total + "wiliwili/player/current"_i18n;
    this->videoPeopleLabel->setText(count);
    MPV_CE->fire(VideoView::SET_ONLINE_NUM, (void*)count.c_str());
}

void BasePlayerActivity::onVideoRelationInfo(
    const bilibili::VideoRelation& result) {
    brls::Logger::debug("onVideoRelationInfo: {} {} {}", result.like,
                        result.coin, result.favorite);
    this->setRelationButton(result.like, result.coin, result.favorite);
}

void BasePlayerActivity::setRelationButton(bool liked, bool coin,
                                           bool favorite) {
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
        btnFavorite->setImageFromSVGRes(
            "svg/bpx-svg-sprite-collection-active.svg");
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
    if (pystring::count(error, "10403") > 0) {
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
    MPV_CE->unsubscribe(customEventSubscribeID);
    // 停止视频播放
    this->video->stop();
}