//
// Created by fang on 2023/1/3.
//

#include <borealis/views/dialog.hpp>
#include <borealis/views/dropdown.hpp>
#include <borealis/core/touch/tap_gesture.hpp>

#include "bilibili/result/home_pgc_season_result.h"
#include "activity/player_activity.hpp"
#include "fragment/player_collection.hpp"
#include "fragment/player_fragments.hpp"
#include "fragment/season_evaluate.hpp"
#include "fragment/share_dialog.hpp"
#include "utils/config_helper.hpp"
#include "utils/dialog_helper.hpp"
#include "utils/number_helper.hpp"
#include "utils/image_helper.hpp"
#include "view/video_view.hpp"
#include "view/video_card.hpp"
#include "view/svg_image.hpp"
#include "view/mpv_core.hpp"
#include "view/grid_dropdown.hpp"

/// PlayerSeasonActivity

PlayerSeasonActivity::PlayerSeasonActivity(const unsigned int id, PGC_ID_TYPE type, int progress)
    : pgc_id(id), pgcIdType(type) {
    if (type == PGC_ID_TYPE::SEASON_ID) {
        brls::Logger::debug("open season: {}", id);
    } else if (type == PGC_ID_TYPE::EP_ID) {
        brls::Logger::debug("open ep: {}", id);
    }

    this->changeEpisodeEvent.subscribe([this](auto t) {
        if (t.id == 0) return;

        // 设置评论区骨架屏幕
        this->recyclingGrid->estimatedRowHeight = 100;
        this->recyclingGrid->showSkeleton();

        auto view = brls::Application::getCurrentFocus();
        if (view->getParentActivity() == this &&
            (dynamic_cast<VideoComment*>(view) != nullptr || dynamic_cast<VideoCommentReply*>(view) != nullptr)) {
            // 焦点位于当前页面，且在 评论 或 回复 上时，将焦点转移到tab栏
            tabFrame->focusTab(0);
        }

        // 因为是手动切换分集，所以将分集进度设置为 0
        t.progress = 0;
        this->changeEpisode(t);
    });

    this->setProgress(progress);
}

PlayerSeasonActivity::~PlayerSeasonActivity() {
    brls::Logger::debug("del PlayerSeasonActivity");
    //上报历史记录
    this->reportCurrentProgress(MPVCore::instance().video_progress, MPVCore::instance().duration);
}

void PlayerSeasonActivity::setProgress(int p) { episodeResult.progress = p; }

int PlayerSeasonActivity::getProgress() { return episodeResult.progress; }

void PlayerSeasonActivity::reportCurrentProgress(size_t progress, size_t duration) {
    this->reportHistory(episodeResult.aid, episodeResult.cid, progress, duration, 4);
}

void PlayerSeasonActivity::onIndexChange(size_t index) {
    if (index >= episodeList.size()) {
        brls::Logger::error("unaccepted index: {}, accepted range 0 - {}", index, episodeList.size() - 1);
        return;
    }
    // 如果遇到id为0，说明是标题，那么就停止播放
    // 也就是说目前的设定是自动连播不会跨越section播放
    if (episodeList[index].id == 0) {
        // 无下一集可播，显示重播按钮
        APP_E->fire(VideoView::REPLAY, nullptr);
        return;
    }

    this->changeEpisodeEvent.fire(episodeList[index]);

    this->changeIndexEvent.fire(index);
}

void PlayerSeasonActivity::onIndexChangeToNext() {
    // episodeResult.index 是从0开始计数的分集序号
    this->onIndexChange(episodeResult.index + 1);
}

void PlayerSeasonActivity::onContentAvailable() {
    this->setCommonData();

    this->videoTitleBox->registerAction("wiliwili/player/intro"_i18n, brls::ControllerButton::BUTTON_A,
                                        [this](brls::View* view) -> bool {
                                            auto* evaluate = new SeasonEvaluate();
                                            evaluate->setKeyword(this->seasonInfo.season_title);
                                            evaluate->setContent(this->seasonInfo.evaluate);
                                            auto dialog = new brls::Dialog(evaluate);
                                            dialog->open();
                                            return true;
                                        });
    this->videoTitleBox->addGestureRecognizer(new brls::TapGestureRecognizer(this->videoTitleBox));

    //评论加载下一页
    recyclingGrid->onNextPage([this]() {
        if (this->episodeResult.aid != 0) this->requestVideoComment(std::to_string(this->episodeResult.aid));
    });

    // 二维码按钮
    this->btnQR->getParent()->registerClickAction([this](...) {
        auto dialog = new ShareDialog();
#if defined(__APPLE__) || defined(__linux__) || defined(_WIN32)
        dialog->open(episodeResult.link, seasonInfo.season_title + " " + episodeResult.title, seasonInfo.evaluate,
                     seasonInfo.cover);
#else
        dialog->open(this->episodeResult.link);
#endif
        return true;
    });

    // 投币按钮
    this->btnCoin->getParent()->registerClickAction([this](...) {
        showCoinDialog(this->episodeResult.aid);
        return true;
    });

    // 点赞按钮
    this->btnAgree->getParent()->registerClickAction([this](...) {
        if (!DialogHelper::checkLogin()) return true;
        this->beAgree(episodeResult.aid);
        return true;
    });

    // 收藏按钮
    this->btnFavorite->getParent()->registerClickAction([this](...) {
        this->showCollectionDialog(episodeResult.id, (int)VideoType::Episode);
        return true;
    });

    this->videoUserInfo->registerClickAction([this](...) {
        if (!DialogHelper::checkLogin()) return true;
        this->followSeason(this->seasonInfo.season_id, !this->seasonStatus.follow);
        return true;
    });

    this->requestData(this->pgc_id, this->pgcIdType);
}

void PlayerSeasonActivity::onSeasonEpisodeInfo(const bilibili::SeasonEpisodeResult& result) {
    std::string title = this->seasonInfo.season_title + " - " + result.title;
    APP_E->fire(VideoView::SET_TITLE, (void*)title.c_str());
    this->videoBVIDLabel->setText(result.bvid);
}

void PlayerSeasonActivity::onSeasonStatus(const bilibili::SeasonStatusResult& result) {
    if (seasonInfo.type == 1 || seasonInfo.type == 4) {
        if (result.follow) {
            this->videoUserInfo->setHintType(InfoHintType::BANGUMI_FOLLOWING);
        } else {
            this->videoUserInfo->setHintType(InfoHintType::BANGUMI_NOT_FOLLOWED);
        }
    } else {
        if (result.follow) {
            this->videoUserInfo->setHintType(InfoHintType::CINEMA_FOLLOWING);
        } else {
            this->videoUserInfo->setHintType(InfoHintType::CINEMA_NOT_FOLLOWED);
        }
    }
}

void PlayerSeasonActivity::onSeasonVideoInfo(const bilibili::SeasonResultWrapper& result) {
    brls::Logger::debug("[onSeasonVideoInfo] title:{} author:{} seasonID: {}", result.season_title,
                        result.up_info.uname, result.season_id);

    auto avatar = result.up_info.avatar;
    if (!avatar.empty()) avatar += ImageHelper::face_ext;
    auto desc = result.season_desc;
    if (result.rating.score >= 0)
        desc += fmt::format(" - {}分", result.rating.score);
    else
        desc += " - 暂无评分";
    this->videoUserInfo->setUserInfo(avatar, result.up_info.uname, desc);
    this->boxFavorites->setVisibility(brls::Visibility::VISIBLE);

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

    // videoView bangumi custom setting
    video->setBangumiCustomSetting(result.season_title, result.season_id);

    // 设置分集信息
    changeIndexEvent.clear();
    this->tabFrame->clearTab("wiliwili/player/p"_i18n);
    auto* item = new AutoSidebarItem();
    item->setTabStyle(AutoTabBarStyle::ACCENT);
    item->setFontSize(18);
    item->setLabel("wiliwili/player/p"_i18n);
    this->tabFrame->addTab(item, [this, result, item]() {
        // 设置分集页面
        auto container = new BasePlayerTabFragment<bilibili::SeasonEpisodeResult>(
            // 列表数据
            episodeList,
            // 分集标题设置回调
            [](auto recycler, auto ds, auto& d) -> RecyclingGridItem* {
                if (!d.id) {
                    // 显示项为标题
                    auto* item = (PlayerTabHeader*)recycler->dequeueReusableCell("Header");
                    item->title->setText(d.title);
                    return item;
                }

                // 显示分集项
                auto* item = (PlayerTabCell*)recycler->dequeueReusableCell("Cell");
                item->title->setText(d.title);
                item->setSelected(ds->getCurrentIndex() == d.index);
                item->setBadge(d.badge_info.text, d.badge_info.bg_color);
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
            episodeResult.index);

        // 分集点击回调
        container->getSelectEvent()->subscribe([this](auto recycler, auto ds, size_t index, const auto& r) {
            if (r.id == 0 || r.cid == 0 || r.aid == 0) return;
            // 触发播放对应的剧集
            changeEpisodeEvent.fire(r);

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
        size_t whole = result.episodes.size();
        for (auto& s : result.section) whole += s.episodes.size();
        item->setSubtitle(wiliwili::num2w(whole));

        return container;
    });

    video->setSeasonAction([this](brls::View* view) {
        auto* dropdown = new BaseDropdown(
            "wiliwili/player/p"_i18n, [this](int selected) { this->onIndexChange(selected); }, episodeResult.index);
        dropdown->getRecyclingList()->registerCell("Cell", []() { return PlayerTabCell::create(); });
        dropdown->getRecyclingList()->registerCell("Header", []() { return PlayerTabHeader::create(); });
        dropdown->setDataSource(new CommonDataSourceDropdown<bilibili::SeasonEpisodeResult>(
            this->episodeList, dropdown, [dropdown](auto recycler, auto d) {
                if (!d.id) {
                    // 显示项为标题
                    auto* item = (PlayerTabHeader*)recycler->dequeueReusableCell("Header");
                    item->title->setText(d.title);
                    return (RecyclingGridItem*)item;
                }
                // 显示分集项
                auto* item = (PlayerTabCell*)recycler->dequeueReusableCell("Cell");
                item->title->setText(d.title);
                item->setSelected(dropdown->getSelected() == d.index);
                item->setBadge(d.badge_info.text, d.badge_info.bg_color);
                return (RecyclingGridItem*)item;
            }));
        brls::Application::pushActivity(new brls::Activity(dropdown));
        return true;
    });
}

void PlayerSeasonActivity::onSeasonSeriesInfo(const bilibili::SeasonSeries& result) {
    if (result.size() <= 1) return;

    this->tabFrame->clearTab("wiliwili/player/series"_i18n);
    auto* item = new AutoSidebarItem();
    item->setTabStyle(AutoTabBarStyle::ACCENT);
    item->setFontSize(18);
    item->setLabel("wiliwili/player/series"_i18n);
    this->tabFrame->addTab(item, [this, result, item]() {
        // 设置分集页面
        auto container = new BasePlayerTabFragment<bilibili::SeasonSeriesItem>(
            // 列表数据
            result,
            // 分集标题设置回调
            [](auto recycler, auto ds, auto& d) -> RecyclingGridItem* {
                // 显示分集项
                auto* item = (RecyclingGridItemSeasonSeriesVideoCard*)recycler->dequeueReusableCell("Card");
                auto cover = d.cover.empty() ? "" : d.cover + ImageHelper::h_ext;
                item->setCard(cover, d.season_title, d.subtitle, wiliwili::num2w(d.stat.views),
                              wiliwili::num2w(d.stat.series_follow), d.badge_info.text, d.badge_info.bg_color);
                return item;
            },
            // container的构造函数
            [](auto recycler, auto ds) {
                recycler->estimatedRowHeight = 100;
                recycler->estimatedRowSpace  = 15;
                recycler->registerCell("Card", []() { return RecyclingGridItemSeasonSeriesVideoCard::create(); });
            });

        // 分集点击回调
        container->getSelectEvent()->subscribe(
            [this](auto recycler, auto ds, size_t index, const auto& r) { this->playSeason(r.season_id); });

        // 设置标题上方的数字
        item->setSubtitle(wiliwili::num2w(result.size()));

        return container;
    });
}

void PlayerSeasonActivity::onSeasonRecommend(const bilibili::SeasonRecommendWrapper& result) {
    if (result.season.empty()) return;

    this->tabFrame->clearTab("wiliwili/player/recommend"_i18n);
    auto* item = new AutoSidebarItem();
    item->setTabStyle(AutoTabBarStyle::ACCENT);
    item->setFontSize(18);
    item->setLabel("wiliwili/player/recommend"_i18n);
    this->tabFrame->addTab(item, [this, result, item]() {
        // 设置分集页面
        auto container = new BasePlayerTabFragment<bilibili::SeasonRecommendItem>(
            // 列表数据
            result.season,
            // 分集标题设置回调
            [](auto recycler, auto ds, auto& d) -> RecyclingGridItem* {
                // 显示分集项
                auto* item        = (RecyclingGridItemSeasonSeriesVideoCard*)recycler->dequeueReusableCell("Card");
                std::string score = d.score > 0 ? fmt::format("{}分", d.score) : "暂无评分";
                auto cover        = d.cover.empty() ? "" : d.cover + ImageHelper::h_ext;
                item->setCard(cover, d.title, d.subtitle, wiliwili::num2w(d.stat.view), wiliwili::num2w(d.stat.follow),
                              score, "#D97607");
                return item;
            },
            // container的构造函数
            [](auto recycler, auto ds) {
                recycler->estimatedRowHeight = 100;
                recycler->estimatedRowSpace  = 15;
                recycler->registerCell("Card", []() { return RecyclingGridItemSeasonSeriesVideoCard::create(); });
            });

        // 分集点击回调
        container->getSelectEvent()->subscribe(
            [this](auto recycler, auto ds, size_t index, const auto& r) { this->playSeason(r.season_id); });

        // 设置标题上方的数字
        item->setSubtitle(wiliwili::num2w(result.season.size()));

        return container;
    });
}

void PlayerSeasonActivity::playSeason(uint64_t season_id) {
    //上报历史记录
    this->reportCurrentProgress(MPVCore::instance().video_progress, MPVCore::instance().duration);

    brls::View* currentFocus = brls::Application::getCurrentFocus();

    // 停止播放视频
    this->video->stop();

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
    this->tabFrame->clearTab("wiliwili/player/series"_i18n);
    this->tabFrame->clearTab("wiliwili/player/recommend"_i18n);

    // 清空评论
    // 强制设置高度100，提升骨架屏显示效果
    this->recyclingGrid->estimatedRowHeight = 100;
    this->recyclingGrid->showSkeleton();

    this->setProgress(0);
    this->requestData(season_id, PGC_ID_TYPE::SEASON_ID);
}

uint64_t PlayerSeasonActivity::getAid() { return episodeResult.aid; }

void PlayerSeasonActivity::requestCastUrl() { this->requestCastVideoUrl(episodeResult.id, episodeResult.cid, 2); }

void PlayerSeasonActivity::onCastPlayUrl(const bilibili::VideoUrlResult& result) {
    brls::Logger::debug("onCastPlayUrl: {}", result.durl[0].url);
    bilibili::VideoCastData data;
    data.url   = result.durl[0].url;
    data.title = seasonInfo.season_title + " " + episodeResult.title;
    APP_E->fire("CAST_URL", (void*)&data);
}
