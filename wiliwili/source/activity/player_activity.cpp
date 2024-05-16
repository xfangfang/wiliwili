//
// Created by fang on 2022/7/10.
//

#include <utility>
#include <cstdlib>
#include <fmt/format.h>
#include <borealis/views/dialog.hpp>
#include <borealis/core/touch/tap_gesture.hpp>

#include "activity/player_activity.hpp"
#include "utils/number_helper.hpp"
#include "utils/config_helper.hpp"
#include "utils/dialog_helper.hpp"
#include "utils/activity_helper.hpp"
#include "utils/image_helper.hpp"
#include "fragment/player_collection.hpp"
#include "fragment/player_fragments.hpp"
#include "fragment/player_evaluate.hpp"
#include "fragment/share_dialog.hpp"
#include "view/grid_dropdown.hpp"
#include "view/svg_image.hpp"
#include "view/video_view.hpp"
#include "view/video_card.hpp"
#include "view/user_info.hpp"
#include "view/mpv_core.hpp"

using namespace brls::literals;

class DataSourceUserUploadedVideoList : public RecyclingGridDataSource {
public:
    DataSourceUserUploadedVideoList(bilibili::UserUploadedVideoListResult result, ChangeVideoEvent cb)
        : list(std::move(result)), changeVideoEvent(std::move(cb)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemRelatedVideoCard* item =
            (RecyclingGridItemRelatedVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::UserUploadedVideoResult& r = this->list[index];
        item->setCard(r.pic + ImageHelper::h_ext, r.title, r.author + " · " + wiliwili::sec2TimeDate(r.created),
                      r.play == -1 ? "-" : wiliwili::num2w(r.play), wiliwili::num2w(r.video_review), r.length);
        item->setCharging(r.is_charging_arc);
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override { changeVideoEvent.fire(list[index]); }

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
    DataSourceRelatedVideoList(bilibili::VideoDetailListResult result, ChangeVideoEvent cb)
        : list(std::move(result)), changeVideoEvent(std::move(cb)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        RecyclingGridItemRelatedVideoCard* item =
            (RecyclingGridItemRelatedVideoCard*)recycler->dequeueReusableCell("Cell");
        auto& r = this->list[index];
        item->setCard(r.pic + ImageHelper::h_ext, r.title, r.owner.name + " · " + wiliwili::sec2TimeDate(r.pubdate),
                      wiliwili::num2w(r.stat.view), wiliwili::num2w(r.stat.danmaku), wiliwili::sec2Time(r.duration));
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override { changeVideoEvent.fire(list[index]); }

    void appendData(const bilibili::VideoDetailListResult& data) {
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

    void clearData() override { this->list.clear(); };

private:
    bilibili::VideoDetailListResult list;
    ChangeVideoEvent changeVideoEvent;
};

class UGCSeasonHeader : public RecyclingGridItem {
public:
    UGCSeasonHeader() { this->inflateFromXMLRes("xml/views/season_ugc_header_cell.xml"); }

    static RecyclingGridItem* create() { return new UGCSeasonHeader(); }

    BRLS_BIND(brls::Label, title, "player/tab/title");
    BRLS_BIND(brls::Label, subtitle, "player/tab/subtitle");
};

/// PlayerActivity

PlayerActivity::PlayerActivity(const std::string& bvid, unsigned int cid, int progress) {
    videoDetailResult.bvid = bvid;
    videoDetailPage.cid    = cid;
    this->setProgress(progress);
    brls::Logger::debug("create PlayerActivity: bvid: {} cid: {} progress: {}", bvid, cid, progress);

    // 切换到其他视频
    changeVideoEvent.subscribe([this](const bilibili::Video& videoData) {
        //上报历史记录
        this->reportCurrentProgress(MPVCore::instance().video_progress, MPVCore::instance().duration);

        // 停止播放视频
        this->video->stop();
        // 允许加载历史记录
        this->setProgress(0);
        this->video->setLastPlayedPosition(VideoView::POSITION_UNDEFINED);

        brls::View* currentFocus = brls::Application::getCurrentFocus();

        // 先重置一下tabFrame的焦点，避免空指针问题
        // 第0个tab是评论页面，这个tab固定存在，所以不会产生空指针的问题
        this->tabFrame->focusTab(0);
        // 焦点放在video上
        if (currentFocus->getParentActivity() == this)
            brls::Application::giveFocus(this->video);
        else
            brls::Application::giveFocus(currentFocus);

        // 清空无用的tab
        this->tabFrame->clearTab("wiliwili/player/p"_i18n);
        this->tabFrame->clearTab("wiliwili/player/ugc_season"_i18n);
        this->tabFrame->clearTab("wiliwili/player/related"_i18n);
        this->tabFrame->clearTab("wiliwili/player/uploaded"_i18n);

        // 清空评论
        // 强制设置高度100，提升骨架屏显示效果
        this->recyclingGrid->estimatedRowHeight = 100;
        this->recyclingGrid->showSkeleton();

        // 请求新视频的数据
        videoDetailPage.cid = 0;  // cid 设置为0，新视频默认打开PV1
        this->requestVideoInfo(videoData.bvid);
    });
}

void PlayerActivity::onContentAvailable() {
    this->setCommonData();

    // 点击标题查看简介
    this->videoTitleBox->registerAction("wiliwili/player/intro"_i18n, brls::ControllerButton::BUTTON_A,
                                        [this](brls::View* view) -> bool {
                                            auto* evaluate = new PlayerEvaluate();
                                            evaluate->setContent(this->videoDetailResult.desc);
                                            auto dialog = new brls::Dialog(evaluate);
                                            dialog->open();
                                            return true;
                                        });
    this->videoTitleBox->addGestureRecognizer(new brls::TapGestureRecognizer(this->videoTitleBox));

    // 自动加载下一页评论
    this->recyclingGrid->onNextPage([this]() { this->requestVideoComment(std::to_string(this->videoDetailResult.aid)); });

    this->requestData(this->videoDetailResult);

    // 点赞按钮
    this->btnAgree->getParent()->registerClickAction([this](...) {
        if (!DialogHelper::checkLogin()) return true;
        this->beAgree(this->videoDetailResult.aid);
        return true;
    });

    // 投币按钮
    this->btnCoin->getParent()->registerClickAction([this](...) {
        showCoinDialog(this->videoDetailResult.aid);
        return true;
    });

    // 收藏按钮
    this->btnFavorite->getParent()->registerClickAction([this](...) {
        this->showCollectionDialog(videoDetailResult.aid, (int)VideoType::Plain);
        return true;
    });

    // 二维码按钮
    this->btnQR->getParent()->registerClickAction([this](...) {
        auto dialog = new ShareDialog();
#if defined(__APPLE__) || defined(__linux__) || defined(_WIN32)
        dialog->open(fmt::format("https://www.bilibili.com/video/{}/", videoDetailResult.bvid), videoDetailResult.title,
                     videoDetailResult.desc, videoDetailResult.pic, videoDetailResult.owner.name);
#else
        dialog->open("https://www.bilibili.com/video/" + this->videoDetailResult.bvid);
#endif
        return true;
    });

    // 用户头像框
    this->videoUserInfo->registerClickAction([this](...) {
        if (!DialogHelper::checkLogin()) return true;
        if (this->userDetailResult.following) {
            auto dialog = new brls::Dialog("wiliwili/player/not_follow"_i18n);
            dialog->addButton("hints/cancel"_i18n, []() {});
            dialog->addButton("hints/ok"_i18n, [this]() {
                this->followUp(this->userDetailResult.card.mid, !this->userDetailResult.following);
            });
            dialog->open();
        } else {
            this->followUp(this->userDetailResult.card.mid, !this->userDetailResult.following);
        }
        return true;
    });

    // 隐藏跳过片头片尾，因为这个是番剧专属的设置
    this->video->hideSkipOpeningCreditsSetting();
}

void PlayerActivity::onVideoInfo(const bilibili::VideoDetailResult& result) {
    brls::Logger::debug("[onVideoInfo] title:{} author:{}", result.title, result.owner.name);
    // 只在分P数大于1时显示分P标题
    std::string subtitle = result.pages.size() > 1 ? " - " + videoDetailPage.part : "";

    // videoView osd
    std::string title = result.title + subtitle;
    APP_E->fire(VideoView::SET_TITLE, (void*)title.c_str());

    // video title
    this->videoTitleLabel->setText(result.title);

    // bottom area
    this->videoBVIDLabel->setText(result.bvid);
    this->videoCountLabel->setText(result.stat.view == -1 ? "-" : wiliwili::num2w(result.stat.view));
    this->videoDanmakuLabel->setText(wiliwili::num2w(result.stat.danmaku));
    this->videoTimeLabel->setText(wiliwili::sec2FullDate(result.pubdate));
    if (result.rights.no_reprint == 1) {
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

void PlayerActivity::onUpInfo(const bilibili::UserDetailResultWrapper& user) {
    // user info
    this->videoUserInfo->setUserInfo(user.card.face + ImageHelper::face_ext, user.card.name,
                                     wiliwili::num2w(user.follower) + "粉丝" +
                                         (brls::Application::ORIGINAL_WINDOW_HEIGHT < 720 ? "\n" : " · ") +
                                         wiliwili::num2w(user.like_num) + "点赞");
    if (user.card.mid == ProgramConfig::instance().getUserID()) {
        this->videoUserInfo->setHintType(InfoHintType::NONE);
    } else if (user.following) {
        this->videoUserInfo->setHintType(InfoHintType::UP_FOLLOWING);
    } else {
        this->videoUserInfo->setHintType(InfoHintType::UP_NOT_FOLLOWED);
    }
}

void PlayerActivity::onVideoPageListInfo(const bilibili::VideoDetailPageListResult& result) {
    for (const auto& i : result) {
        brls::Logger::debug("cid:{} title:{}", i.cid, i.part);
    }

    if (result.size() <= 1) {
        return;
    }

    changeIndexEvent.clear();
    this->tabFrame->clearTab("wiliwili/player/p"_i18n);
    auto* item = new AutoSidebarItem();
    item->setTabStyle(AutoTabBarStyle::ACCENT);
    item->setFontSize(18);
    item->setLabel("wiliwili/player/p"_i18n);
    this->tabFrame->addTab(item, [this, result, item]() {
        // 设置分集页面
        auto container = new BasePlayerTabFragment<bilibili::VideoDetailPage>(
            // 列表数据
            result,
            // 分集标题设置回调
            [](auto recycler, auto ds, auto& d) -> RecyclingGridItem* {
                auto* item = (PlayerTabCell*)recycler->dequeueReusableCell("Cell");
                item->title->setText(fmt::format("P{} {}", d.page, d.part));
                item->setSelected(ds->getCurrentIndex() == (d.page - 1));
                item->setBadge(wiliwili::sec2MinSec(d.duration), nvgRGBA(0, 0, 0, 0),
                               brls::Application::getTheme().getColor("font/grey"));
                return item;
            },
            // container的构造函数
            [this](auto recycler, auto ds) {
                changeIndexEvent.subscribe([ds, recycler](size_t index) {
                    ds->setCurrentIndex(index);

                    // 更新ui
                    auto* item = dynamic_cast<PlayerTabCell*>(recycler->getGridItemByIndex(index));
                    if (!item) return;
                    std::vector<RecyclingGridItem*>& items = recycler->getGridItems();
                    for (auto& i : items) {
                        auto* cell = dynamic_cast<PlayerTabCell*>(i);
                        if (cell) cell->setSelected(false);
                    }
                    item->setSelected(true);
                    recycler->selectRowAt(index, false);
                });
            },
            // 默认的选中索引
            videoDetailPage.page - 1);

        // 分集点击回调
        container->getSelectEvent()->subscribe([this](auto recycler, auto ds, size_t index, const auto& r) {
            if (r.cid == 0) return;
            // 触发播放对应的分集
            this->onIndexChange(r.page - 1);

            // 更新ui
            auto* item = dynamic_cast<PlayerTabCell*>(recycler->getGridItemByIndex(index));
            if (!item) return;
            std::vector<RecyclingGridItem*>& items = recycler->getGridItems();
            for (auto& i : items) {
                auto* cell = dynamic_cast<PlayerTabCell*>(i);
                if (cell) cell->setSelected(false);
            }
            item->setSelected(true);
            ds->setCurrentIndex(index);
        });

        // 设置标题上方的数字
        item->setSubtitle(wiliwili::num2w(result.size()));
        return container;
    });
}

void PlayerActivity::onUGCSeasonInfo(const bilibili::UGCSeason& result) {
    brls::Logger::debug("UGC Season: {}/{}", result.title, result.id);

    this->tabFrame->clearTab("wiliwili/player/ugc_season"_i18n);
    auto* item = new AutoSidebarItem();
    item->setTabStyle(AutoTabBarStyle::ACCENT);
    item->setFontSize(18);
    item->setLabel("wiliwili/player/ugc_season"_i18n);
    this->tabFrame->addTab(item, [this, result, item]() {
        // 设置分集页面
        auto container = new BasePlayerTabFragment<bilibili::UGCSeasonEpisode>(
            // 列表数据
            result.episodes,
            // 分集标题设置回调
            [result](auto recycler, auto ds, auto& d) -> RecyclingGridItem* {
                if (d.index == 0) {
                    // 显示项为 season 标题
                    auto* item = (UGCSeasonHeader*)recycler->dequeueReusableCell("HeaderUGC");
                    item->title->setText(d.title);
                    item->subtitle->setText(wiliwili::num2w(result.stat.view) + "播放");
                    return item;
                }
                if (!d.id) {
                    // 显示项为 section 标题
                    auto* item = (PlayerTabHeader*)recycler->dequeueReusableCell("Header");
                    item->title->setText(d.title);
                    return item;
                }

                auto* item = (PlayerTabCell*)recycler->dequeueReusableCell("Cell");
                item->title->setText(d.title);
                item->setSelected(ds->getCurrentIndex() == (size_t)d.index);
                item->setBadge(wiliwili::sec2MinSec(d.page.duration), nvgRGBA(0, 0, 0, 0),
                               brls::Application::getTheme().getColor("font/grey"));
                return item;
            },
            // container的构造函数
            [](auto recycler, auto ds) {
                recycler->registerCell("HeaderUGC", []() { return UGCSeasonHeader::create(); });
            },
            // 默认的选中索引
            result.currentIndex,
            // 组件高度
            [](auto recycler, auto ds, auto& d) -> float {
                if (d.index == 0) return 80;
                return 50;
            });

        // 分集点击回调
        container->getSelectEvent()->subscribe([this, result](auto recycler, auto ds, size_t index, const auto& r) {
            if (index == 0) {
                auto* evaluate = new PlayerEvaluate();
                evaluate->setContent(result.intro);
                auto dialog = new brls::Dialog(evaluate);
                dialog->open();
                return;
            }
            if (r.bvid.empty()) return;
            bilibili::Video video;
            video.bvid = r.bvid;
            this->changeVideoEvent.fire(video);
        });

        // 设置标题上方的数字
        item->setSubtitle(fmt::format("{}/{}", result.currentIndexWithoutHeader + 1, wiliwili::num2w(result.count)));
        return container;
    });
}

void PlayerActivity::onUploadedVideos(const bilibili::UserUploadedVideoResultWrapper& result) {
    for (const auto& i : result.list) {
        brls::Logger::debug("up videos: bvid:{} title:{}", i.bvid, i.title);
    }

    if (result.page.pn == 1) {
        this->tabFrame->clearTab("wiliwili/player/uploaded"_i18n);
        // 加载第一页，添加tab
        auto* item = new AutoSidebarItem();
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
            grid->registerCell("Cell", []() { return RecyclingGridItemRelatedVideoCard::create(); });
            grid->onNextPage([this]() { this->requestUploadedVideos(videoDetailResult.owner.mid); });
            item->setSubtitle(wiliwili::num2w(result.page.count));
            container->addView(grid);
            grid->setDataSource(new DataSourceUserUploadedVideoList(result.list, changeVideoEvent));
            return container;
        });
    } else {
        // 加载第N页
        auto tab = this->tabFrame->getTab("wiliwili/player/uploaded"_i18n);
        if (!tab) return;
        auto view = (AttachedView*)tab->getAttachedView();
        if (!view) return;
        auto grid        = (RecyclingGrid*)view->getChildren()[0];
        auto* datasource = (DataSourceUserUploadedVideoList*)grid->getDataSource();
        if (!result.list.empty()) {
            datasource->appendData(result.list);
            grid->notifyDataChanged();
        }
    }
}

void PlayerActivity::onRelatedVideoList(const bilibili::VideoDetailListResult& result) {
    if (result.size() <= 1) {
        return;
    }
    this->tabFrame->clearTab("wiliwili/player/related"_i18n);
    auto* item = new AutoSidebarItem();
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
        grid->registerCell("Cell", []() { return RecyclingGridItemRelatedVideoCard::create(); });
        item->setSubtitle(wiliwili::num2w(result.size()));
        container->addView(grid);
        grid->setDataSource(new DataSourceRelatedVideoList(result, changeVideoEvent));
        return container;
    });
}

void PlayerActivity::onRedirectToEp(const std::string& epid) {
    brls::Logger::debug("redirect to ep: {}", epid);
    // 一般来说这种情况对应播放器全屏状态，将全屏的播放器先取消全屏
    auto* view = brls::Application::getCurrentFocus();
    if (view->getParentActivity() != this) {
        brls::Application::popActivity(brls::TransitionAnimation::NONE);
    }
    // 跳转到番剧播放页
    brls::Application::popActivity(brls::TransitionAnimation::NONE,
                                   [epid]() { Intent::openSeasonByEpId(std::stoi(epid)); });
}

void PlayerActivity::setProgress(int p) { videoDetailPage.progress = p; }

int PlayerActivity::getProgress() { return videoDetailPage.progress; }

void PlayerActivity::reportCurrentProgress(size_t progress, size_t duration) {
    this->reportHistory(videoDetailResult.aid, videoDetailPage.cid, progress, duration, 3);
}

void PlayerActivity::onIndexChange(size_t index) {
    if (index >= videoDetailResult.pages.size()) {
        brls::Logger::error("unaccepted index: {}, accepted range 0 - {}", index, videoDetailResult.pages.size() - 1);
        return;
    }

    brls::Logger::debug("切换分P: {}", index);
    // 上报历史记录
    this->reportCurrentProgress(MPVCore::instance().video_progress, MPVCore::instance().duration);
    // 焦点放在video上
    if (brls::Application::getCurrentFocus()->getParentActivity() == this) brls::Application::giveFocus(this->video);
    // 设置当前分P数据
    videoDetailPage = videoDetailResult.pages[index];
    // 设置播放器标题
    std::string title = fmt::format("{} - {}", videoDetailResult.title, videoDetailPage.part);
    APP_E->fire(VideoView::SET_TITLE, (void*)title.c_str());
    // 允许加载历史记录
    this->setProgress(0);
    this->video->setLastPlayedPosition(VideoView::POSITION_UNDEFINED);
    // 请求视频链接
    this->requestVideoUrl(videoDetailResult.bvid, videoDetailPage.cid);
}

void PlayerActivity::onIndexChangeToNext() {
    // videoDetailPage.page 是从1开始计数的单调递增序号，所以这里是尝试加载下一分P
    if (videoDetailPage.page < videoDetailResult.pages.size()) {
        changeIndexEvent.fire(videoDetailPage.page);
        this->onIndexChange(videoDetailPage.page);
        return;
    }

    // 分集播放结束，判断是否要播放合集视频
    auto& ugc = videoDetailResult.ugc_season;
    if (ugc.currentIndex >= 0) {
        for (size_t i = ugc.currentIndex + 1; i < ugc.episodes.size(); i++) {
            if (ugc.episodes[i].bvid.empty()) continue;
            bilibili::Video video;
            video.bvid = ugc.episodes[i].bvid;
            this->changeVideoEvent.fire(video);
            return;
        }
    }

    // 合集播放结束，判断是否要播放推荐视频
    if (PLAYER_STRATEGY == PlayerStrategy::RCMD && !videDetailRelated.empty()) {
        changeVideoEvent.fire(videDetailRelated[0]);
    } else {
        // 无下一集可播，显示重播按钮
        APP_E->fire(VideoView::REPLAY, nullptr);
    }
}

size_t PlayerActivity::getAid() { return videoDetailResult.aid; }

PlayerActivity::~PlayerActivity() {
    brls::Logger::debug("del PlayerActivity");
    //上报历史记录
    this->reportCurrentProgress(MPVCore::instance().video_progress, MPVCore::instance().duration);
}
void PlayerActivity::requestCastUrl() { this->requestCastVideoUrl(videoDetailResult.aid, videoDetailPage.cid, 1); }

void PlayerActivity::onCastPlayUrl(const bilibili::VideoUrlResult& result) {
    brls::Logger::debug("onCastPlayUrl: {}", result.durl[0].url);

    bilibili::VideoCastData data;
    data.url   = result.durl[0].url;
    data.title = videoDetailResult.title;
    APP_E->fire("CAST_URL", (void*)&data);
}
