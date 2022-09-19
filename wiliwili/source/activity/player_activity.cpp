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
    DataSourceList(std::vector<std::string> result, ChangeIndexEvent cb)
        : data(result), changeEpisodeEvent(cb) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler,
                                  size_t index) override {
        GridRadioCell* item =
            (GridRadioCell*)recycler->dequeueReusableCell("Cell");

        auto r = this->data[index];
        item->title->setText(this->data[index]);
        //        item->setSelected(index == dropdown->getSelected());

        return item;
    }

    size_t getItemCount() override { return data.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        changeEpisodeEvent.fire(index);
    }

private:
    std::vector<std::string> data;
    ChangeIndexEvent changeEpisodeEvent;
};

class DataSourceUserUploadedVideoList : public RecyclingGridDataSource {
public:
    DataSourceUserUploadedVideoList(
        bilibili::UserUploadedVideoListResult result, ChangeVideoEvent cb)
        : list(result), changeVideoEvent(cb) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemRelatedVideoCard* item =
            (RecyclingGridItemRelatedVideoCard*)recycler->dequeueReusableCell(
                "Cell");

        bilibili::UserUploadedVideoResult& r = this->list[index];
        item->setCard(r.pic + "@672w_378h_1c.jpg", r.title,
                      r.author + " · " + wiliwili::sec2TimeDate(r.created),
                      wiliwili::num2w(r.play), wiliwili::num2w(r.video_review),
                      r.length);
        return item;
    }

    size_t getItemCount() { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) {
        changeVideoEvent.fire(list[index]);
    }

    void appendData(const bilibili::UserUploadedVideoListResult& data) {
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

private:
    bilibili::UserUploadedVideoListResult list;
    ChangeVideoEvent changeVideoEvent;
};

class DataSourceRelatedVideoList : public RecyclingGridDataSource {
public:
    DataSourceRelatedVideoList(bilibili::VideoDetailListResult result,
                               ChangeVideoEvent cb)
        : list(result), changeVideoEvent(cb) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) {
        RecyclingGridItemRelatedVideoCard* item =
            (RecyclingGridItemRelatedVideoCard*)recycler->dequeueReusableCell(
                "Cell");
        auto& r = this->list[index];
        item->setCard(r.pic + "@672w_378h_1c.jpg", r.title,
                      r.owner.name + " · " + wiliwili::sec2TimeDate(r.pubdate),
                      wiliwili::num2w(r.stat.view),
                      wiliwili::num2w(r.stat.danmaku),
                      wiliwili::sec2Time(r.duration));
        return item;
    }

    size_t getItemCount() { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) {
        changeVideoEvent.fire(list[index]);
    }

    void appendData(const bilibili::VideoDetailListResult& data) {
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

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

PlayerActivity::PlayerActivity(std::string bvid) {
    videoDetailResult.bvid = bvid;
    brls::Logger::debug("create PlayerActivity: {}", videoDetailResult.bvid);
}

PlayerActivity::PlayerActivity(std::string bvid, unsigned int cid,
                               int progress) {
    videoDetailResult.bvid   = bvid;
    videoDetailPage.cid      = cid;
    videoDetailPage.progress = progress;
    brls::Logger::debug("create PlayerActivity: bvid: {} cid: {} progress: {}",
                        bvid, cid, progress);
}

void PlayerActivity::onContentAvailable() {
    this->PlayerActivity::setCommonData();

    // 点击标题查看简介
    this->videoTitleBox->registerAction(
        "查看简介", brls::ControllerButton::BUTTON_A,
        [this](brls::View* view) -> bool {
            auto dialog = new brls::Dialog(this->videoDetailResult.desc);
            dialog->addButton("hints/ok"_i18n, []() {});
            dialog->open();
            return true;
        });

    // 切换视频分P
    changePEvent.subscribe([this](int index) {
        brls::Logger::debug("切换分区: {}", index);
        videoDetailPage = videoDetailResult.pages[index];
        this->requestVideoUrl(videoDetailResult.bvid, videoDetailPage.cid);
        //上报历史记录
        this->reportHistory(videoDetailResult.aid, videoDetailPage.cid, 0);
    });

    // 切换到其他视频
    changeVideoEvent.subscribe([this](bilibili::Video videoData) {
        //上报历史记录
        this->reportHistory(videoDetailResult.aid, videoDetailPage.cid,
                            MPVCore::instance().video_progress);

        // 停止播放视频
        this->video->stop();

        // 先重置一下tabFrame的焦点，避免空指针问题
        // 第0个tab是评论页面，这个tab固定存在，所以不会产生空指针的问题
        this->tabFrame->focusTab(0);
        // 焦点放在video上
        brls::Application::giveFocus(this->video);

        // 清空无用的tab
        this->tabFrame->clearTab("分集");
        this->tabFrame->clearTab("推荐");
        this->tabFrame->clearTab("投稿");

        // 清空评论
        this->recyclingGrid->showSkeleton(4);

        // 请求新视频的数据
        videoDetailPage.cid = 0;  // cid 设置为0，新视频默认打开PV1
        this->requestVideoInfo(videoData.bvid);
    });

    //todo: X键 刷新播放页

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

void PlayerActivity::setCommonData() {
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

    this->btnQR->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnQR->getParent()));

    if (brls::Application::getThemeVariant() == brls::ThemeVariant::LIGHT)
        btnQR->setImageFromSVGRes("svg/widget-qrcode-dark.svg");
    else
        btnQR->setImageFromSVGRes("svg/widget-qrcode.svg");

    this->setRelationButton(false, false, false);

    eventSubscribeID =
        MPVCore::instance().getEvent()->subscribe([this](MpvEventEnum event) {
            static int64_t lastProgress = MPVCore::instance().video_progress;
            switch (event) {
                case MpvEventEnum::UPDATE_PROGRESS:
                    // 每15秒同步一次进度
                    if (lastProgress + 15 <
                        MPVCore::instance().video_progress) {
                        lastProgress = MPVCore::instance().video_progress;
                        auto self = dynamic_cast<PlayerSeasonActivity*>(this);
                        if (self) {
                            this->reportHistory(episodeResult.aid,
                                                episodeResult.cid, lastProgress,
                                                4);
                        } else {
                            this->reportHistory(videoDetailResult.aid,
                                                videoDetailPage.cid,
                                                lastProgress, 3);
                        }
                    } else if (MPVCore::instance().video_progress <
                               lastProgress) {
                        lastProgress = MPVCore::instance().video_progress;
                    }
                    break;
                default:
                    break;
            }
        });
}

void PlayerActivity::showShareDialog(const std::string link) {
    auto container = new brls::Box(brls::Axis::COLUMN);
    container->setJustifyContent(brls::JustifyContent::CENTER);
    container->setAlignItems(brls::AlignItems::CENTER);
    auto qr = new QRImage();
    qr->setSize(brls::Size(256, 256));
    qr->setImageFromQRContent(link);
    qr->setMargins(20, 10, 10, 10);
    container->addView(qr);
    auto hint = new brls::Label();
    hint->setText("手机扫码观看/分享");
    hint->setMargins(0, 10, 10, 10);
    container->addView(hint);
    auto dialog = new brls::Dialog(container);
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->open();
}

void PlayerActivity::onVideoInfo(const bilibili::VideoDetailResult& result) {
    brls::Logger::debug("[onVideoInfo] title:{} author:{}", result.title,
                        result.owner.name);

    // user info
    auto& user = this->userDetailResult;
    this->videoUserInfo->setUserInfo(
        user.card.face + "@96w_96h_1c.jpg", user.card.name,
        wiliwili::num2w(user.follower) + "粉丝 · " +
            wiliwili::num2w(user.like_num) + "点赞");
    if (user.card.mid == ProgramConfig::instance().getUserID()) {
        this->videoUserInfo->setHintType(InfoHintType::NONE);
    } else if (user.following) {
        this->videoUserInfo->setHintType(InfoHintType::UP_FOLLOWING);
    } else {
        this->videoUserInfo->setHintType(InfoHintType::UP_NOT_FOLLOWED);
    }

    // videoView osd
    this->video->setTitle(result.title);

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
    item->setLabel("分集");
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
        item->setLabel("投稿");
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
        auto tab = this->tabFrame->getTab("投稿");
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

void PlayerActivity::onVideoPlayUrl(const bilibili::VideoUrlResult& result) {
    brls::Logger::debug("quality: {}", result.quality);
    // todo: 将多个文件加入播放列表
    //todo: 播放失败时可以尝试备用播放链接

    int progress = videoDetailPage.progress;
    auto self    = dynamic_cast<PlayerSeasonActivity*>(this);
    if (self) {
        progress = episodeResult.progress;
    }

    // 向前回退5秒
    progress -= 5;
    if (progress > 0) {
        for (const auto& i : result.durl) {
            this->video->setUrl(i.url, progress);
            break;
        }
    } else {
        for (const auto& i : result.durl) {
            this->video->start(i.url);
            break;
        }
    }

    brls::Logger::debug("PlayerActivity::onVideoPlayUrl done");
}

void PlayerActivity::onCommentInfo(
    const bilibili::VideoCommentResultWrapper& result) {
    DataSourceCommentList* datasource =
        (DataSourceCommentList*)recyclingGrid->getDataSource();
    if (result.cursor.prev == 1) {
        // 第一页评论
        //整合置顶评论
        std::vector<bilibili::VideoCommentResult> comments(result.top_replies);
        comments.insert(comments.end(), result.replies.begin(),
                        result.replies.end());
        this->recyclingGrid->setDataSource(new DataSourceCommentList(comments));
        // 设置评论数量提示
        auto item = this->tabFrame->getTab("评论");
        if (item) item->setSubtitle(wiliwili::num2w(result.cursor.all_count));
    } else if (datasource) {
        // 第N页评论
        datasource->appendData(result.replies);
        recyclingGrid->notifyDataChanged();
    }
}

void PlayerActivity::onRequestCommentError(const std::string& error) {
    brls::sync([this, error]() { this->recyclingGrid->setError(error); });
}

void PlayerActivity::onVideoOnlineCount(
    const bilibili::VideoOnlineTotal& result) {
    this->videoPeopleLabel->setText(result.total + "人正在看");
    this->video->setOnlineCount(result.total + "人正在看");
}

void PlayerActivity::onVideoRelationInfo(
    const bilibili::VideoRelation& result) {
    brls::Logger::debug("onVideoRelationInfo: {} {} {}", result.like,
                        result.coin, result.favorite);
    this->setRelationButton(result.like, result.coin, result.favorite);
}

void PlayerActivity::setRelationButton(bool liked, bool coin, bool favorite) {
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

void PlayerActivity::onRelatedVideoList(
    const bilibili::VideoDetailListResult& result) {
    if (result.size() <= 1) {
        return;
    }
    AutoSidebarItem* item = new AutoSidebarItem();
    item->setTabStyle(AutoTabBarStyle::ACCENT);
    item->setFontSize(18);
    item->setLabel("推荐");
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

void PlayerActivity::onDanmaku(const std::string& filePath) {
    const char* cmd[] = {"sub-add", filePath.c_str(), "select", "danmaku",
                         NULL};
    MPVCore::instance().command_async(cmd);
}

void PlayerActivity::onError(const std::string& error) {
    ASYNC_RETAIN
    brls::sync([ASYNC_TOKEN, error]() {
        ASYNC_RELEASE
        auto dialog = new brls::Dialog(error);
        dialog->setCancelable(false);
        dialog->addButton("OK", []() { brls::Application::popActivity(); });
        dialog->open();
    });
}

PlayerActivity::~PlayerActivity() {
    brls::Logger::debug("del PlayerActivity");
    //上报历史记录
    this->reportHistory(videoDetailResult.aid, videoDetailPage.cid,
                        MPVCore::instance().video_progress);
    // 取消监控mpv
    MPVCore::instance().getEvent()->unsubscribe(eventSubscribeID);
    // 停止视频播放
    this->video->stop();
}

/// season player

PlayerSeasonActivity::PlayerSeasonActivity(const unsigned int id,
                                           PGC_ID_TYPE type, int progress)
    : pgc_id(id), pgcIdType(type) {
    if (type == PGC_ID_TYPE::SEASON_ID) {
        brls::Logger::debug("open season: {}", id);
    } else if (type == PGC_ID_TYPE::EP_ID) {
        brls::Logger::debug("open ep: {}", id);
    }

    episodeResult.progress = progress;
}

PlayerSeasonActivity::~PlayerSeasonActivity() {
    brls::Logger::debug("del PlayerSeasonActivity");
    //上报历史记录
    this->reportHistory(episodeResult.aid, episodeResult.cid,
                        MPVCore::instance().video_progress, 4);
}

void PlayerSeasonActivity::onContentAvailable() {
    this->PlayerActivity::setCommonData();

    this->videoTitleBox->registerAction(
        "查看简介", brls::ControllerButton::BUTTON_A,
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
    changeEpisodeEvent.subscribe([this](int index) {
        this->reportHistory(episodeResult.aid, episodeResult.cid,
                            MPVCore::instance().video_progress, 4);
        this->changeEpisode(seasonInfo.episodes[index]);
    });

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
    if (!avatar.empty()) avatar += "@96w_96h_1c.jpg";
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
    this->labelQR->setText("分享");

    AutoSidebarItem* item = new AutoSidebarItem();
    item->setTabStyle(AutoTabBarStyle::ACCENT);
    item->setFontSize(18);
    item->setLabel("分集");
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