//
// Created by fang on 2023/1/3.
//

#include "activity/player_activity.hpp"
#include "fragment/player_collection.hpp"
#include "fragment/player_fragments.hpp"
#include "view/video_view.hpp"
#include "utils/config_helper.hpp"
#include "utils/dialog_helper.hpp"
#include "bilibili/result/home_pgc_season_result.h"

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
    if (index >= episodeList.size()) {
        brls::Logger::error("unaccepted index: {}, accepted range 0 - {}",
                            index, episodeList.size() - 1);
        return;
    }
    // 如果遇到id为0，说明是标题，那么就停止播放
    // 也就是说目前的设定是自动连播不会跨越section播放
    if (episodeList[index].id == 0) return;
    this->changeEpisode(episodeList[index]);
    this->changeIndexEvent.fire(index);
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

    // 二维码按钮
    this->btnQR->getParent()->registerClickAction([this](...) {
        this->showShareDialog(this->episodeResult.link);
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

    this->requestData(this->pgc_id, this->pgcIdType);
}

void PlayerSeasonActivity::onSeasonEpisodeInfo(
    const bilibili::SeasonEpisodeResult& result) {
    this->video->setTitle(this->seasonInfo.season_title + " - " + result.title);
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

    // 设置分集信息
    changeIndexEvent.clear();
    AutoSidebarItem* item = new AutoSidebarItem();
    item->setTabStyle(AutoTabBarStyle::ACCENT);
    item->setFontSize(18);
    item->setLabel("wiliwili/player/p"_i18n);
    this->tabFrame->addTab(item, [this, result, item]() {
        // 设置分集页面
        auto container =
            new BasePlayerTabFragment<bilibili::SeasonEpisodeResult>(
                // 列表数据
                episodeList,
                // 分集标题设置回调
                [](auto recycler, auto ds, auto& d) -> RecyclingGridItem* {
                    if (!d.id) {
                        // 显示项为标题
                        PlayerTabHeader* item =
                            (PlayerTabHeader*)recycler->dequeueReusableCell(
                                "Header");
                        item->title->setText(d.title);
                        return item;
                    }

                    // 显示分集项
                    PlayerTabCell* item =
                        (PlayerTabCell*)recycler->dequeueReusableCell("Cell");
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
                        PlayerTabCell* item = dynamic_cast<PlayerTabCell*>(
                            recycler->getGridItemByIndex(index));
                        if (!item) return;
                        std::vector<RecyclingGridItem*>& items =
                            recycler->getGridItems();
                        for (auto& i : items) {
                            PlayerTabCell* cell =
                                dynamic_cast<PlayerTabCell*>(i);
                            if (cell) cell->setSelected(false);
                        }
                        item->setSelected(true);
                        recycler->selectRowAt(index, false);
                    });
                },
                // 默认的选中索引
                episodeResult.index);

        // 分集点击回调
        container->getSelectEvent()->subscribe(
            [this](auto recycler, auto ds, size_t index, const auto& r) {
                if (r.id == 0 || r.cid == 0 || r.aid == 0) return;
                // 触发播放对应的剧集
                changeEpisodeEvent.fire(r);

                // 更新ui
                PlayerTabCell* item = dynamic_cast<PlayerTabCell*>(
                    recycler->getGridItemByIndex(index));
                if (!item) return;
                std::vector<RecyclingGridItem*>& items =
                    recycler->getGridItems();
                for (auto& i : items) {
                    PlayerTabCell* cell = dynamic_cast<PlayerTabCell*>(i);
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
}