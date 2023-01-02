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
#include "utils/dialog_helper.hpp"
#include "fragment/player_coin.hpp"
#include "fragment/player_collection.hpp"
#include "fragment/player_fragments.hpp"
#include "bilibili/result/home_pgc_season_result.h"

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

void BasePlayerActivity::onContentAvailable() { this->setCommonData(); }

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

    this->btnQR->setImageFromSVGRes("svg/bpx-svg-sprite-share.svg");

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
                    // 尝试自动加载下一分集
                    // 如果当前最顶层是Dialog就放弃自动播放，因为有可能是用户点开了收藏或者投币对话框
                    {
                        auto stack    = brls::Application::getActivitiesStack();
                        Activity* top = stack[stack.size() - 1];
                        if (dynamic_cast<brls::Dialog*>(top->getContentView()))
                            return;
                        if (AUTO_NEXT_PART) this->onIndexChangeToNext();
                    }
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

void BasePlayerActivity::showCollectionDialog(int64_t id, int videoType) {
    if (!checkLogin()) return;
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
        // 为了加载骨架屏美观，设置为了100，在加载评论时手动修改回来
        this->recyclingGrid->estimatedRowHeight = 416;
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
        // 强制设置高度100，提升骨架屏显示效果
        this->recyclingGrid->estimatedRowHeight = 100;
        this->recyclingGrid->showSkeleton(6);

        // 请求新视频的数据
        videoDetailPage.cid = 0;  // cid 设置为0，新视频默认打开PV1
        this->requestVideoInfo(videoData.bvid);
    });
}

void PlayerActivity::onContentAvailable() {
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

    this->requestData(this->videoDetailResult);

    // 点赞按钮
    this->btnAgree->getParent()->registerClickAction([this](...) {
        if (!checkLogin()) return true;
        this->beAgree(this->videoDetailResult.aid);
        return true;
    });

    // 投币按钮
    this->btnCoin->getParent()->registerClickAction([this](...) {
        if (!checkLogin()) return true;

        if (std::to_string(videoDetailResult.owner.mid) ==
            ProgramConfig::instance().getUserID()) {
            showDialog("wiliwili/player/coin/own"_i18n);
            return true;
        }

        int coins = getCoinTolerate();
        if (coins <= 0) {
            showDialog("wiliwili/player/coin/run_out"_i18n);
            return true;
        }

        auto playerCoin = new PlayerCoin();
        if (coins == 1) playerCoin->hideTwoCoin();
        playerCoin->getSelectEvent()->subscribe([this, playerCoin](int value) {
            this->addCoin((unsigned int)this->videoDetailResult.aid, value,
                          playerCoin->likeAtTheSameTime());
        });
        auto dialog = new brls::Dialog(playerCoin);
        dialog->open();
        return true;
    });

    // 收藏按钮
    this->btnFavorite->getParent()->registerClickAction([this](...) {
        this->showCollectionDialog(videoDetailResult.aid,
                                   (int)VideoType::Plain);
        return true;
    });

    // 二维码按钮
    this->btnQR->getParent()->registerClickAction([this](...) {
        this->showShareDialog("https://www.bilibili.com/video/" +
                              this->videoDetailResult.bvid);
        return true;
    });

    // 用户头像框
    this->videoUserInfo->registerClickAction([this](...) {
        if (this->userDetailResult.following) {
            auto dialog = new brls::Dialog("wiliwili/player/not_follow"_i18n);
            dialog->addButton("hints/cancel"_i18n, []() {});
            dialog->addButton("hints/ok"_i18n, [this]() {
                this->followUp(this->userDetailResult.card.mid,
                               !this->userDetailResult.following);
            });
            dialog->open();
        } else {
            this->followUp(this->userDetailResult.card.mid,
                           !this->userDetailResult.following);
        }
        return true;
    });
}

void PlayerActivity::onVideoInfo(const bilibili::VideoDetailResult& result) {
    brls::Logger::debug("[onVideoInfo] title:{} author:{}", result.title,
                        result.owner.name);
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

void PlayerActivity::onRedirectToEp(const std::string& epid) {
    brls::Logger::debug("redirect to ep: {}", epid);
    brls::Application::popActivity(brls::TransitionAnimation::NONE, [epid]() {
        brls::Application::pushActivity(
            new PlayerSeasonActivity(std::stoi(epid), PGC_ID_TYPE::EP_ID));
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
    if (index >= episodeList.size()) {
        brls::Logger::error("unaccepted index: {}, accepted range 0 - {}",
                            index, episodeList.size() - 1);
        return;
    }
    // 如果遇到id为0，说明是标题，那么就停止播放
    // 也就是说目前的设定是自动连播不会跨越section播放
    if (episodeList[index].id == 0) return;
    this->changeEpisode(episodeList[index]);
    this->changeEpisodeIDEvent.fire(index);
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

    this->btnAgree->getParent()->registerClickAction([this](...) {
        if (!checkLogin()) return true;
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
                    changeEpisodeIDEvent.subscribe(
                        [ds, recycler](size_t index) {
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