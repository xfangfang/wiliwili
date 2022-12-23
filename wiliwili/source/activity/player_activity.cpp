//
// Created by fang on 2022/7/10.
//

#include <borealis.hpp>
#include "view/video_view.hpp"
#include "view/video_card.hpp"
#include "view/user_info.hpp"
#include "activity/player_activity.hpp"
#include "view/grid_dropdown.hpp"
#include "view/svg_image.hpp"
#include "view/qr_image.hpp"
#include "fmt/format.h"
#include "utils/number_helper.hpp"
#include "utils/config_helper.hpp"

using namespace brls::literals;

class DataSourceList : public RecyclingGridDataSource {
public:
    DataSourceList(std::vector<std::string> result, ChangeIndexEvent cb,
                   size_t defaultIndex = 0)
        : data(result), changeIndexEvent(cb), currentIndex(defaultIndex) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler,
                                  size_t index) override {
        GridRadioCell* item =
            (GridRadioCell*)recycler->dequeueReusableCell("Cell");

        auto r = this->data[index];
        item->title->setText(this->data[index]);
        //todo: 实现选中标识
        //        item->setSelected(index == defaultIndex);

        return item;
    }

    size_t getItemCount() override { return data.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        changeIndexEvent.fire(index);
        currentIndex = index;
    }

    void clearData() override { this->data.clear(); }

private:
    std::vector<std::string> data;
    ChangeIndexEvent changeIndexEvent;
    size_t currentIndex;
};

class DataSourceUserUploadedVideoList : public RecyclingGridDataSource {
public:
    DataSourceUserUploadedVideoList(
        bilibili::UserUploadedVideoListResult result, ChangeVideoEvent cb)
        : list(result), changeVideoEvent(cb) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler,
                                  size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemRelatedVideoCard* item =
            (RecyclingGridItemRelatedVideoCard*)recycler->dequeueReusableCell(
                "Cell");

        bilibili::UserUploadedVideoResult& r = this->list[index];
        item->setCard(r.pic + ImageHelper::h_ext, r.title,
                      r.author + " · " + wiliwili::sec2TimeDate(r.created),
                      wiliwili::num2w(r.play), wiliwili::num2w(r.video_review),
                      r.length);
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        changeVideoEvent.fire(list[index]);
    }

    void appendData(const bilibili::UserUploadedVideoListResult& data) {
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

    void clearData() override { this->list.clear(); };

private:
    bilibili::UserUploadedVideoListResult list;
    ChangeVideoEvent changeVideoEvent;
};

class DataSourceRelatedVideoList : public RecyclingGridDataSource {
public:
    DataSourceRelatedVideoList(bilibili::VideoDetailListResult result,
                               ChangeVideoEvent cb)
        : list(result), changeVideoEvent(cb) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler,
                                  size_t index) override {
        RecyclingGridItemRelatedVideoCard* item =
            (RecyclingGridItemRelatedVideoCard*)recycler->dequeueReusableCell(
                "Cell");
        auto& r = this->list[index];
        item->setCard(r.pic + ImageHelper::h_ext, r.title,
                      r.owner.name + " · " + wiliwili::sec2TimeDate(r.pubdate),
                      wiliwili::num2w(r.stat.view),
                      wiliwili::num2w(r.stat.danmaku),
                      wiliwili::sec2Time(r.duration));
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        changeVideoEvent.fire(list[index]);
    }

    void appendData(const bilibili::VideoDetailListResult& data) {
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

    void clearData() override { this->list.clear(); };

private:
    bilibili::VideoDetailListResult list;
    ChangeVideoEvent changeVideoEvent;
};

class DataSourceCommentList : public RecyclingGridDataSource {
public:
    DataSourceCommentList(bilibili::VideoCommentListResult result)
        : dataList(result) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler,
                                  size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        VideoComment* item =
            (VideoComment*)recycler->dequeueReusableCell("Cell");

        item->setData(this->dataList[index]);
        return item;
    }

    size_t getItemCount() override { return dataList.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {}

    void appendData(const bilibili::VideoCommentListResult& data) {
        this->dataList.insert(this->dataList.end(), data.begin(), data.end());
    }

    void clearData() override { this->dataList.clear(); }

private:
    bilibili::VideoCommentListResult dataList;
};

/// BasePlayerActivity

void BasePlayerActivity::onContentAvailable() {
    this->setCommonData();

    // 点击标题查看简介
    this->videoTitleBox->registerAction(
        "wiliwili/player/intro"_i18n, brls::ControllerButton::BUTTON_A,
        [this](brls::View* view) -> bool {
            auto dialog = new brls::Dialog(this->videoDetailResult.desc);
            dialog->addButton("hints/ok"_i18n, []() {});
            dialog->open();
            return true;
        });

    // 自动加载下一页评论
    this->recyclingGrid->onNextPage(
        [this]() { this->requestVideoComment(this->videoDetailResult.aid); });

    // 点赞按钮
    this->btnAgree->getParent()->registerClickAction([this](...) {
        this->beAgree(this->videoDetailResult.aid);
        /// 点赞投币后，需要等待反馈后，再更新UI
        std::thread updateUI([this]() {
            sleep(1);
            this->requestVideoRelationInfo(this->videoDetailResult.bvid);
        });
        updateUI.detach();

        return true;
    });

    // 投币按钮
    this->btnCoin->getParent()->registerClickAction([this](...) {
        this->addCoin((unsigned int)this->videoDetailResult.aid);
        /// 点赞投币后，需要等待反馈后，再更新UI
        std::thread updateUI([this]() {
            sleep(1);
            this->requestVideoRelationInfo(this->videoDetailResult.bvid);
        });
        updateUI.detach();

        return true;
    });

    // 收藏按钮
    this->btnFavorite->getParent()->registerClickAction([this](...) {
        this->addResource((unsigned int)this->videoDetailResult.aid);
        /// 收藏后，需要等待反馈后，再更新UI
        std::thread updateUI([this]() {
            sleep(1);
            this->requestVideoRelationInfo(this->videoDetailResult.bvid);
        });
        updateUI.detach();

        return true;
    });

    // 二维码按钮
    this->btnQR->getParent()->registerClickAction([this](...) {
        this->showShareDialog("https://www.bilibili.com/video/" +
                              this->videoDetailResult.bvid);
        return true;
    });

    this->requestData(this->videoDetailResult);
}

void BasePlayerActivity::setCommonData() {
    // 视频评论
    recyclingGrid->registerCell("Cell",
                                []() { return VideoComment::create(); });

    // 切换右侧Tab
    this->registerAction(
        "上一项", brls::ControllerButton::BUTTON_LT,
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

    // 调整清晰度
    this->registerAction(
        "wiliwili/player/quality"_i18n, brls::ControllerButton::BUTTON_START,
        [this](brls::View* view) -> bool {
            if (this->videoUrlResult.accept_description.empty()) return true;
            brls::Application::pushActivity(
                new brls::Activity(new brls::Dropdown(
                    "wiliwili/player/quality"_i18n,
                    this->videoUrlResult.accept_description,
                    [this](int _selected) {
                        BasePlayerActivity::defaultQuality =
                            this->videoUrlResult.accept_quality[_selected];

                        // 在加载视频时，若设置了进度，会自动向前跳转5秒，
                        // 这里提前加上5s用来抵消播放视频时的进度问题。
                        setProgress(MPVCore::instance().video_progress + 5);

                        // dash
                        if (!this->videoUrlResult.dash.video.empty()) {
                            // dash格式的视频无需重复请求视频链接，这里简单的设置清晰度即可
                            videoUrlResult.quality =
                                BasePlayerActivity::defaultQuality;
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
                    getQualityIndex())));

            return true;
        });

    this->btnQR->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnQR->getParent()));

    if (brls::Application::getThemeVariant() == brls::ThemeVariant::LIGHT)
        btnQR->setImageFromSVGRes("svg/widget-qrcode-dark.svg");
    else
        btnQR->setImageFromSVGRes("svg/widget-qrcode.svg");

    this->setRelationButton(false, false, false);

    eventSubscribeID =
        MPVCore::instance().getEvent()->subscribe([this](MpvEventEnum event) {
            // 上一次报告历史记录的时间点
            static int64_t lastProgress = MPVCore::instance().video_progress;
            switch (event) {
                case MpvEventEnum::UPDATE_PROGRESS:
                    // 每15秒同步一次进度
                    if (lastProgress + 15 <
                        MPVCore::instance().video_progress) {
                        lastProgress = MPVCore::instance().video_progress;
                        this->reportCurrentProgress(
                            lastProgress, MPVCore::instance().duration);
                    } else if (MPVCore::instance().video_progress <
                               lastProgress) {
                        // 当前播放时间小于上一次上传历史记录的时间点
                        // 发生于向前拖拽进度的时候，此时重置lastProgress的值
                        lastProgress = MPVCore::instance().video_progress;
                    }
                    break;
                case MpvEventEnum::END_OF_FILE:
                    // 尝试自动加载下一分P
                    if (AUTO_NEXT_PART) this->onIndexChangeToNext();
                    break;
                default:
                    break;
            }
        });
}

void BasePlayerActivity::showShareDialog(const std::string link) {
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

void BasePlayerActivity::onVideoPlayUrl(
    const bilibili::VideoUrlResult& result) {
    brls::Logger::debug("onVideoPlayUrl quality: {}", result.quality);
    //todo: 播放失败时可以尝试备用播放链接

    // 进度向前回退5秒，避免当前进度过于接近结尾出现一加载就结束的情况
    int progress = this->getProgress() - 5;

    if (!result.dash.video.empty()) {
        // dash
        brls::Logger::debug("Video type: dash");
        for (const auto& i : result.dash.video) {
            // todo: 相同清晰度的码率选择方案
            if (result.quality >= i.id) {
                // 手动设置当前选择的清晰度
                videoUrlResult.quality = i.id;
                for (const auto& j : result.dash.audio) {
                    this->video->setUrl(i.base_url, progress, j.base_url);
                    break;
                }
                break;
            }
        }
    } else {
        // flv
        brls::Logger::debug("Video type: flv");
        if (result.durl.size() == 0) {
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

    brls::Logger::debug("BasePlayerActivity::onVideoPlayUrl done");
}

void BasePlayerActivity::onCommentInfo(
    const bilibili::VideoCommentResultWrapper& result) {
    DataSourceCommentList* datasource =
        dynamic_cast<DataSourceCommentList*>(recyclingGrid->getDataSource());
    if (result.cursor.prev == 1) {
        // 第一页评论
        //整合置顶评论
        std::vector<bilibili::VideoCommentResult> comments(result.top_replies);
        comments.insert(comments.end(), result.replies.begin(),
                        result.replies.end());
        this->recyclingGrid->setDataSource(new DataSourceCommentList(comments));
        // 设置评论数量提示
        auto item = this->tabFrame->getTab("wiliwili/player/comment"_i18n);
        if (item) item->setSubtitle(wiliwili::num2w(result.cursor.all_count));
    } else if (datasource) {
        // 第N页评论
        datasource->appendData(result.replies);
        recyclingGrid->notifyDataChanged();
    }
}

void BasePlayerActivity::onRequestCommentError(const std::string& error) {
    brls::sync([this, error]() { this->recyclingGrid->setError(error); });
}

void BasePlayerActivity::onVideoOnlineCount(
    const bilibili::VideoOnlineTotal& result) {
    this->videoPeopleLabel->setText(result.total +
                                    "wiliwili/player/current"_i18n);
    this->video->setOnlineCount(result.total + "wiliwili/player/current"_i18n);
}

void BasePlayerActivity::onVideoRelationInfo(
    const bilibili::VideoRelation& result) {
    brls::Logger::debug("onVideoRelationInfo: {} {} {}", result.like,
                        result.coin, result.favorite);
    this->setRelationButton(result.like, result.coin, result.favorite);
}

void BasePlayerActivity::setRelationButton(bool liked, bool coin,
                                           bool favorite) {
    bool lightTheme =
        brls::Application::getThemeVariant() == brls::ThemeVariant::LIGHT;

    if (liked) {
        btnAgree->setImageFromSVGRes("svg/bpx-svg-sprite-liked-active.svg");
    } else {
        if (lightTheme)
            btnAgree->setImageFromSVGRes("svg/widget-agree-dark.svg");
        else
            btnAgree->setImageFromSVGRes("svg/widget-agree.svg");
    }
    if (coin) {
        btnCoin->setImageFromSVGRes("svg/bpx-svg-sprite-coin-active.svg");
    } else {
        if (lightTheme)
            btnCoin->setImageFromSVGRes("svg/widget-coin-dark.svg");
        else
            btnCoin->setImageFromSVGRes("svg/widget-coin.svg");
    }
    if (favorite) {
        btnFavorite->setImageFromSVGRes(
            "svg/bpx-svg-sprite-collection-active.svg");
    } else {
        if (lightTheme)
            btnFavorite->setImageFromSVGRes("svg/widget-favorite-dark.svg");
        else
            btnFavorite->setImageFromSVGRes("svg/widget-favorite.svg");
    }
}

void BasePlayerActivity::onError(const std::string& error) {
    ASYNC_RETAIN
    brls::sync([ASYNC_TOKEN, error]() {
        ASYNC_RELEASE
        auto dialog = new brls::Dialog(error);
        dialog->setCancelable(false);
        dialog->addButton("hints/ok"_i18n, []() {
            brls::sync([]() { brls::Application::popActivity(); });
        });
        dialog->open();
    });
}

BasePlayerActivity::~BasePlayerActivity() {
    brls::Logger::debug("del BasePlayerActivity");
    // 取消监控mpv
    MPVCore::instance().getEvent()->unsubscribe(eventSubscribeID);
    // 停止视频播放
    this->video->stop();
}

/// PlayerActivity

PlayerActivity::PlayerActivity(std::string bvid, unsigned int cid,
                               int progress) {
    videoDetailResult.bvid = bvid;
    videoDetailPage.cid    = cid;
    this->setProgress(progress);
    brls::Logger::debug("create PlayerActivity: bvid: {} cid: {} progress: {}",
                        bvid, cid, progress);

    // 切换视频分P
    changePEvent.subscribe([this](int index) { this->onIndexChange(index); });

    // 切换到其他视频
    changeVideoEvent.subscribe([this](bilibili::Video videoData) {
        //上报历史记录
        this->reportCurrentProgress(MPVCore::instance().video_progress,
                                    MPVCore::instance().duration);

        // 停止播放视频
        this->video->stop();

        // 先重置一下tabFrame的焦点，避免空指针问题
        // 第0个tab是评论页面，这个tab固定存在，所以不会产生空指针的问题
        this->tabFrame->focusTab(0);
        // 焦点放在video上
        brls::Application::giveFocus(this->video);

        // 清空无用的tab
        this->tabFrame->clearTab("wiliwili/player/p"_i18n);
        this->tabFrame->clearTab("wiliwili/player/related"_i18n);
        this->tabFrame->clearTab("wiliwili/player/uploaded"_i18n);

        // 清空评论
        this->recyclingGrid->showSkeleton(4);

        // 请求新视频的数据
        videoDetailPage.cid = 0;  // cid 设置为0，新视频默认打开PV1
        this->requestVideoInfo(videoData.bvid);
    });
}

void PlayerActivity::onVideoInfo(const bilibili::VideoDetailResult& result) {
    brls::Logger::debug("[onVideoInfo] title:{} author:{}", result.title,
                        result.owner.name);

    // user info
    auto& user = this->userDetailResult;
    this->videoUserInfo->setUserInfo(
        user.card.face + ImageHelper::face_ext, user.card.name,
        wiliwili::num2w(user.follower) + "粉丝 · " +
            wiliwili::num2w(user.like_num) + "点赞");
    if (user.card.mid == ProgramConfig::instance().getUserID()) {
        this->videoUserInfo->setHintType(InfoHintType::NONE);
    } else if (user.following) {
        this->videoUserInfo->setHintType(InfoHintType::UP_FOLLOWING);
    } else {
        this->videoUserInfo->setHintType(InfoHintType::UP_NOT_FOLLOWED);
    }

    // 只在分P数大于1时显示分P标题
    std::string subtitle =
        result.pages.size() > 1 ? " - " + videoDetailPage.part : "";

    // videoView osd
    this->video->setTitle(result.title + subtitle);

    // video title
    this->videoTitleLabel->setText(result.title);

    // bottom area
    this->videoBVIDLabel->setText(result.bvid);
    this->videoCountLabel->setText(wiliwili::num2w(result.stat.view));
    this->videoDanmakuLabel->setText(wiliwili::num2w(result.stat.danmaku));
    this->videoTimeLabel->setText(wiliwili::sec2FullDate(result.pubdate));
    if (result.copyright == 1) {
        this->videoCopyRightBox->setVisibility(brls::Visibility::VISIBLE);
    } else {
        this->videoCopyRightBox->setVisibility(brls::Visibility::GONE);
    }

    // like/coins/favorite/share
    this->labelAgree->setText(wiliwili::num2w(result.stat.like));
    this->labelCoin->setText(wiliwili::num2w(result.stat.coin));
    this->labelFavorite->setText(wiliwili::num2w(result.stat.favorite));
    this->labelQR->setText(wiliwili::num2w(result.stat.share));
}

void PlayerActivity::onVideoPageListInfo(
    const bilibili::VideoDetailPageListResult& result) {
    for (const auto& i : result) {
        brls::Logger::debug("cid:{} title:{}", i.cid, i.part);
    }

    if (result.size() <= 1) {
        return;
    }

    AutoSidebarItem* item = new AutoSidebarItem();
    item->setTabStyle(AutoTabBarStyle::ACCENT);
    item->setFontSize(18);
    item->setLabel("wiliwili/player/p"_i18n);
    this->tabFrame->addTab(item, [this, result, item]() {
        auto container = new AttachedView();
        container->setMarginTop(12);
        auto grid = new RecyclingGrid();
        grid->setPadding(0, 40, 0, 20);
        grid->setGrow(1);
        grid->applyXMLAttribute("spanCount", "1");
        grid->applyXMLAttribute("itemSpace", "0");
        grid->applyXMLAttribute("itemHeight", "50");
        grid->registerCell("Cell", []() { return GridRadioCell::create(); });

        std::vector<std::string> items;
        for (unsigned int i = 0; i < result.size(); ++i) {
            auto title = result[i].part;
            items.push_back(fmt::format("PV{} {}", i + 1, title));
        }
        container->addView(grid);
        grid->setDataSource(new DataSourceList(items, changePEvent));
        item->setSubtitle(wiliwili::num2w(result.size()));
        return container;
    });
}

void PlayerActivity::onUploadedVideos(
    const bilibili::UserUploadedVideoResultWrapper& result) {
    for (const auto& i : result.list) {
        brls::Logger::debug("up videos: bvid:{} title:{}", i.bvid, i.title);
    }

    if (result.page.pn == 1) {
        // 加载第一页，添加tab
        AutoSidebarItem* item = new AutoSidebarItem();
        item->setTabStyle(AutoTabBarStyle::ACCENT);
        item->setFontSize(18);
        item->setLabel("wiliwili/player/uploaded"_i18n);
        this->tabFrame->addTab(item, [this, result, item]() {
            auto container = new AttachedView();
            container->setMarginTop(12);
            auto grid = new RecyclingGrid();
            grid->setPadding(0, 20, 0, 20);
            grid->setGrow(1);
            grid->applyXMLAttribute("spanCount", "1");
            grid->applyXMLAttribute("itemSpace", "20");
            grid->applyXMLAttribute("itemHeight", "100");
            grid->registerCell("Cell", []() {
                return RecyclingGridItemRelatedVideoCard::create();
            });
            grid->onNextPage([this]() {
                this->requestUploadedVideos(videoDetailResult.owner.mid);
            });
            item->setSubtitle(wiliwili::num2w(result.page.count));
            container->addView(grid);
            grid->setDataSource(new DataSourceUserUploadedVideoList(
                result.list, changeVideoEvent));
            return container;
        });
    } else {
        // 加载第N页
        auto tab = this->tabFrame->getTab("wiliwili/player/uploaded"_i18n);
        if (!tab) return;
        auto view = (AttachedView*)tab->getAttachedView();
        if (!view) return;
        auto grid = (RecyclingGrid*)view->getChildren()[0];
        DataSourceUserUploadedVideoList* datasource =
            (DataSourceUserUploadedVideoList*)grid->getDataSource();
        datasource->appendData(result.list);
        grid->notifyDataChanged();
    }
}

void PlayerActivity::onRelatedVideoList(
    const bilibili::VideoDetailListResult& result) {
    if (result.size() <= 1) {
        return;
    }
    AutoSidebarItem* item = new AutoSidebarItem();
    item->setTabStyle(AutoTabBarStyle::ACCENT);
    item->setFontSize(18);
    item->setLabel("wiliwili/player/related"_i18n);
    this->tabFrame->addTab(item, [this, result, item]() {
        auto container = new AttachedView();
        container->setMarginTop(12);
        auto grid = new RecyclingGrid();
        grid->setPadding(0, 20, 0, 20);
        grid->setGrow(1);
        grid->applyXMLAttribute("spanCount", "1");
        grid->applyXMLAttribute("itemSpace", "15");
        grid->applyXMLAttribute("itemHeight", "100");
        grid->registerCell("Cell", []() {
            return RecyclingGridItemRelatedVideoCard::create();
        });
        item->setSubtitle(wiliwili::num2w(result.size()));
        container->addView(grid);
        grid->setDataSource(
            new DataSourceRelatedVideoList(result, changeVideoEvent));
        return container;
    });
}

void PlayerActivity::setProgress(int p) { videoDetailPage.progress = p; }

int PlayerActivity::getProgress() { return videoDetailPage.progress; }

void PlayerActivity::reportCurrentProgress(size_t progress, size_t duration) {
    this->reportHistory(videoDetailResult.aid, videoDetailPage.cid, progress,
                        duration, 3);
}

void PlayerActivity::onIndexChange(size_t index) {
    if (index >= videoDetailResult.pages.size()) {
        brls::Logger::error("unaccepted index: {}, accepted range 0 - {}",
                            index, videoDetailResult.pages.size() - 1);
        return;
    }

    brls::Logger::debug("切换分区: {}", index);
    // 焦点放在video上
    brls::Application::giveFocus(this->video);
    // 设置当前分P数据
    videoDetailPage = videoDetailResult.pages[index];
    // 设置播放器标题
    this->video->setTitle(
        fmt::format("{} - {}", videoDetailResult.title, videoDetailPage.part));
    // 请求视频链接
    this->requestVideoUrl(videoDetailResult.bvid, videoDetailPage.cid);
    // 上报历史记录
    this->reportCurrentProgress(0, 0);
}

void PlayerActivity::onIndexChangeToNext() {
    // videoDetailPage.page 是从1开始计数的单调递增序号，所以这里是尝试加载下一分P
    if (videoDetailPage.page < videoDetailResult.pages.size()) {
        this->onIndexChange(videoDetailPage.page);
    } else if (AUTO_NEXT_RCMD) {
        // 分集播放结束，判断是否要播放推荐视频
        if (videDetailRelated.size() > 0)
            changeVideoEvent.fire(videDetailRelated[0]);
    }
}

PlayerActivity::~PlayerActivity() {
    brls::Logger::debug("del PlayerActivity");
    //上报历史记录
    this->reportCurrentProgress(MPVCore::instance().video_progress,
                                MPVCore::instance().duration);
}

/// PlayerSeasonActivity

PlayerSeasonActivity::PlayerSeasonActivity(const unsigned int id,
                                           PGC_ID_TYPE type, int progress)
    : pgc_id(id), pgcIdType(type) {
    if (type == PGC_ID_TYPE::SEASON_ID) {
        brls::Logger::debug("open season: {}", id);
    } else if (type == PGC_ID_TYPE::EP_ID) {
        brls::Logger::debug("open ep: {}", id);
    }

    this->setProgress(progress);
}

PlayerSeasonActivity::~PlayerSeasonActivity() {
    brls::Logger::debug("del PlayerSeasonActivity");
    //上报历史记录
    this->reportCurrentProgress(MPVCore::instance().video_progress,
                                MPVCore::instance().duration);
}

void PlayerSeasonActivity::setProgress(int p) { episodeResult.progress = p; }

int PlayerSeasonActivity::getProgress() { return episodeResult.progress; }

void PlayerSeasonActivity::reportCurrentProgress(size_t progress,
                                                 size_t duration) {
    this->reportHistory(episodeResult.aid, episodeResult.cid, progress,
                        duration, 4);
}

void PlayerSeasonActivity::onIndexChange(size_t index) {
    if (index >= seasonInfo.episodes.size()) {
        brls::Logger::error("unaccepted index: {}, accepted range 0 - {}",
                            index, seasonInfo.episodes.size() - 1);
        return;
    }
    this->changeEpisode(seasonInfo.episodes[index]);
}

void PlayerSeasonActivity::onIndexChangeToNext() {
    // episodeResult.index 是从0开始计数的分集序号
    this->onIndexChange(episodeResult.index + 1);
}

void PlayerSeasonActivity::onContentAvailable() {
    this->setCommonData();

    this->videoTitleBox->registerAction(
        "wiliwili/player/intro"_i18n, brls::ControllerButton::BUTTON_A,
        [this](brls::View* view) -> bool {
            auto dialog = new brls::Dialog(this->seasonInfo.evaluate);
            dialog->addButton("hints/ok"_i18n, []() {});
            dialog->open();
            return true;
        });

    //评论加载下一页
    recyclingGrid->onNextPage([this]() {
        if (this->episodeResult.aid != 0)
            this->requestVideoComment(this->episodeResult.aid);
    });

    //切换分集
    changeEpisodeEvent.subscribe([this](int i) { this->onIndexChange(i); });

    // 二维码按钮
    this->btnQR->getParent()->registerClickAction([this](...) {
        this->showShareDialog(this->episodeResult.link);
        return true;
    });

    this->requestData(this->pgc_id, this->pgcIdType);
}

void PlayerSeasonActivity::onSeasonEpisodeInfo(
    const bilibili::SeasonEpisodeResult& result) {
    auto title = result.long_title;
    if (title.empty()) {
        title = result.title;
    }
    this->video->setTitle(this->seasonInfo.season_title + " - " + title);
    this->videoBVIDLabel->setText(result.bvid);
}

void PlayerSeasonActivity::onSeasonVideoInfo(
    const bilibili::SeasonResultWrapper& result) {
    brls::Logger::debug("[onSeasonVideoInfo] title:{} author:{} seasonID: {}",
                        result.season_title, result.up_info.uname,
                        result.season_id);

    auto avatar = result.up_info.avatar;
    if (!avatar.empty()) avatar += ImageHelper::face_ext;
    auto desc = result.season_desc;
    if (result.rating.score >= 0)
        desc += fmt::format(" - {}分", result.rating.score);
    else
        desc += " - 暂无评分";
    this->videoUserInfo->setUserInfo(avatar, result.up_info.uname, desc);
    this->boxFavorites->setVisibility(brls::Visibility::VISIBLE);

    // videoView osd
    this->video->setTitle(result.season_title);

    // video title
    this->videoTitleLabel->setText(result.season_title);

    // bottom area
    this->videoCountLabel->setText(wiliwili::num2w(result.stat.views));
    this->videoDanmakuLabel->setText(wiliwili::num2w(result.stat.danmakus));
    this->videoFavoritesLabel->setText(wiliwili::num2w(result.stat.favorites));
    this->videoTimeLabel->setText(result.publish.pub_time_show);

    // like/coins/favorite/share
    this->labelAgree->setText(wiliwili::num2w(result.stat.likes));
    this->labelCoin->setText(wiliwili::num2w(result.stat.coins));
    this->labelFavorite->setText(wiliwili::num2w(result.stat.favorite));
    this->labelQR->setText("wiliwili/player/share"_i18n);

    AutoSidebarItem* item = new AutoSidebarItem();
    item->setTabStyle(AutoTabBarStyle::ACCENT);
    item->setFontSize(18);
    item->setLabel("wiliwili/player/p"_i18n);
    this->tabFrame->addTab(item, [this, result, item]() {
        auto container = new AttachedView();
        container->setMarginTop(12);
        auto grid = new RecyclingGrid();
        grid->setPadding(0, 40, 0, 20);
        grid->setGrow(1);
        grid->applyXMLAttribute("spanCount", "1");
        grid->applyXMLAttribute("itemSpace", "0");
        grid->applyXMLAttribute("itemHeight", "50");
        grid->registerCell("Cell", []() { return GridRadioCell::create(); });

        std::vector<std::string> items;
        for (unsigned int i = 0; i < result.episodes.size(); ++i) {
            auto title = result.episodes[i].long_title;
            if (title.empty()) {
                title = result.episodes[i].title;
            }
            items.push_back(fmt::format("PV{} {}", i + 1, title));
        }
        container->addView(grid);
        grid->setDataSource(new DataSourceList(items, changeEpisodeEvent));
        item->setSubtitle(wiliwili::num2w(result.episodes.size()));
        return container;
    });
}