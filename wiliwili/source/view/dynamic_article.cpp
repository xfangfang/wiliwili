//
// Created by fang on 2024/3/22.
//

#include <borealis/views/applet_frame.hpp>
#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/core/thread.hpp>
#include <utility>

#include "view/dynamic_article.hpp"
#include "view/text_box.hpp"
#include "view/dynamic_video_card.hpp"
#include "view/video_comment.hpp"
#include "view/svg_image.hpp"
#include "view/button_close.hpp"
#include "utils/image_helper.hpp"
#include "utils/number_helper.hpp"
#include "fragment/player_single_comment.hpp"
#include "utils/dialog_helper.hpp"
#include "utils/activity_helper.hpp"

using namespace brls::literals;

class DynamicCommentAction : public brls::Box {
public:
    DynamicCommentAction() {
        this->inflateFromXMLRes("xml/fragment/dynamic_card_action.xml");

        this->article->setGrow();

        this->svgLike->registerClickAction([this](...) {
            this->dismiss();
            this->likeClickEvent.fire();
            return true;
        });
        this->svgOpen->registerClickAction([this](...) {
            brls::Application::popActivity(brls::TransitionAnimation::NONE);
            if (this->live.room_id != 0) {
                Intent::openLive(this->live.room_id, this->live.title, this->live.watched_show.text_large);
            } else if (video.epid != 0) {
                Intent::openSeasonByEpId(video.epid);
            } else {
                Intent::openBV(video.bvid);
            }
            return true;
        });
        this->svgGallery->registerClickAction([this](...) {
            Intent::openGallery(images);
            return true;
        });

        this->svgLike->addGestureRecognizer(new brls::TapGestureRecognizer(this->svgLike));
        this->svgOpen->addGestureRecognizer(new brls::TapGestureRecognizer(this->svgOpen));
        this->svgGallery->addGestureRecognizer(new brls::TapGestureRecognizer(this->svgGallery));

        this->closeBtn->registerClickAction([this](...) {
            this->dismiss();
            return true;
        });

        this->cancel->registerClickAction([this](...) {
            this->dismiss();
            return true;
        });
        this->cancel->addGestureRecognizer(new brls::TapGestureRecognizer(this->cancel));

        this->position.setTickCallback([this] {
            this->actionBox->setPositionTop(this->position);
            this->article->getParent()->setPositionTop(this->position - defaultPosition);
            float alpha =
                1 - fabs((this->position - defaultPosition) / fabs(commentOriginalPosition - defaultPosition));
            this->backgroundBox->setAlpha(alpha);
            this->actionBox->setAlpha(alpha);
            this->cancel->setAlpha(alpha);
        });
    }

    void setGalleryData(const bilibili::DynamicArticleModuleDraw* imageData) {
#ifdef __PSV__
        const std::string note_raw_ext = "@300h.jpg";
#else
        const std::string note_raw_ext = "@!web-comment-note.jpg";
#endif
        this->svgGallery->setVisibility(brls::Visibility::VISIBLE);
        for (auto& i : imageData->items) {
            std::string raw_ext = ImageHelper::note_raw_ext;
            if (i.src.size() > 4 && i.src.substr(i.src.size() - 4, 4) == ".gif") {
                // gif 图片暂时按照 jpg 来解析
                raw_ext = note_raw_ext;
            }
            this->images.emplace_back(i.src + raw_ext);
        }
    }

    void setActionData(const bilibili::DynamicArticleResult& data, const bilibili::DynamicArticleModuleState& state,
                       float y, float width) {
        this->article->setCard(data);
        this->article->setForwardNum(state.forward.count);
        this->article->setReplyNum(state.comment.count);
        this->article->setLikeNum(state.like.count);
        this->article->setLiked(state.like.like_state);
        this->backgroundBox->setWidth(width + 48);
        this->live.room_id = 0;
        this->video.epid   = 0;
        this->video.bvid   = "";

        for (auto& module : data.modules) {
            auto moduleType = (bilibili::DynamicArticleModuleType)module.data.index();
            if (moduleType != bilibili::DynamicArticleModuleType::MODULE_TYPE_DATA) continue;
            auto* desc = std::get_if<bilibili::DynamicArticleModuleData>(&module.data);
            if (!desc) break;
            auto dataType = (bilibili::DynamicArticleModuleDataType)desc->data.index();
            if (dataType == bilibili::DynamicArticleModuleDataType::MODULE_TYPE_IMAGE) {
                // 正文图片
                auto* imageData = std::get_if<bilibili::DynamicArticleModuleDraw>(&desc->data);
                if (!imageData) break;
                this->setGalleryData(imageData);
            } else if (dataType == bilibili::DynamicArticleModuleDataType::MODULE_TYPE_VIDEO) {
                // 正文视频
                auto* videoData = std::get_if<bilibili::DynamicArticleModuleArchive>(&desc->data);
                if (!videoData) break;
                this->svgOpen->setVisibility(brls::Visibility::VISIBLE);
                video = *videoData;
            } else if (dataType == bilibili::DynamicArticleModuleDataType::MODULE_TYPE_LIVE) {
                // 正文直播
                auto* liveData = std::get_if<bilibili::DynamicArticleModuleLive>(&desc->data);
                if (!liveData) break;
                this->svgOpen->setVisibility(brls::Visibility::VISIBLE);
                live = liveData->card_info.live_play_info;
            } else if (dataType == bilibili::DynamicArticleModuleDataType::MODULE_TYPE_FORWARD) {
                auto* forwardData = std::get_if<bilibili::DynamicArticleModuleForward>(&desc->data);
                if (!forwardData) break;
                // 转发内容
                for (auto& moduleForward : forwardData->item.modules) {
                    auto forwardModuleType = (bilibili::DynamicArticleModuleType)moduleForward.data.index();
                    if (forwardModuleType != bilibili::DynamicArticleModuleType::MODULE_TYPE_DATA) continue;
                    auto* descForward =
                        std::get_if<bilibili::dynamic_forward::DynamicArticleModuleData>(&moduleForward.data);
                    if (!descForward) break;
                    auto forwardDataType = (bilibili::DynamicArticleModuleDataType)descForward->data.index();
                    if (forwardDataType == bilibili::DynamicArticleModuleDataType::MODULE_TYPE_IMAGE) {
                        // 转发图片
                        auto* imageData = std::get_if<bilibili::DynamicArticleModuleDraw>(&descForward->data);
                        if (!imageData) break;
                        this->setGalleryData(imageData);
                    } else if (forwardDataType == bilibili::DynamicArticleModuleDataType::MODULE_TYPE_VIDEO) {
                        // 转发视频
                        auto* videoData = std::get_if<bilibili::DynamicArticleModuleArchive>(&descForward->data);
                        if (!videoData) break;
                        this->svgOpen->setVisibility(brls::Visibility::VISIBLE);
                        video = *videoData;
                    } else if (forwardDataType == bilibili::DynamicArticleModuleDataType::MODULE_TYPE_LIVE) {
                        // 转发直播
                        auto* liveData = std::get_if<bilibili::DynamicArticleModuleLive>(&descForward->data);
                        if (!liveData) break;
                        this->svgOpen->setVisibility(brls::Visibility::VISIBLE);
                        live = liveData->card_info.live_play_info;
                    }
                }
            }
            break;
        }
        commentOriginalPosition = y;
        this->showStartAnimation();
    }

    void showStartAnimation() {
        if (commentOriginalPosition == defaultPosition) return;

        brls::Application::blockInputs();
        this->position.stop();
        this->position.reset(commentOriginalPosition);
        this->position.addStep(defaultPosition, 300, brls::EasingFunction::quadraticOut);
        this->position.setEndCallback([](bool finished) { brls::Application::unblockInputs(); });
        this->position.start();
    }

    void showDismissAnimation() {
        if (commentOriginalPosition == defaultPosition) {
            brls::Application::popActivity(brls::TransitionAnimation::NONE);
            return;
        }

        brls::Application::blockInputs();
        this->position.stop();
        this->position.reset(defaultPosition);
        this->position.addStep(commentOriginalPosition, 300, brls::EasingFunction::quadraticIn);
        this->position.setEndCallback([](bool finished) {
            brls::Application::unblockInputs();
            brls::Application::popActivity(brls::TransitionAnimation::NONE);
        });
        this->position.start();
    }

    brls::View* getDefaultFocus() override {
        if (this->svgGallery->getVisibility() == brls::Visibility::VISIBLE) {
            return this->svgGallery;
        } else if (this->svgOpen->getVisibility() == brls::Visibility::VISIBLE) {
            return this->svgOpen;
        }
        return this->svgLike;
    }

    void dismiss(std::function<void(void)> cb = nullptr) override { this->showDismissAnimation(); }

    brls::Event<> likeClickEvent;

private:
    BRLS_BIND(SVGImage, svgLike, "action/svg/like");
    BRLS_BIND(SVGImage, svgOpen, "action/svg/open");
    BRLS_BIND(SVGImage, svgGallery, "action/svg/gallery");
    BRLS_BIND(brls::Box, actionBox, "action/box");
    BRLS_BIND(DynamicArticleView, article, "action/article");
    BRLS_BIND(ButtonClose, closeBtn, "button/close");
    BRLS_BIND(brls::Box, backgroundBox, "box/background");
    BRLS_BIND(brls::Box, cancel, "player/cancel");
    BRLS_BIND(brls::ScrollingFrame, scroll, "action/scroll");

    brls::Animatable position     = 0.0f;
    float commentOriginalPosition = 1.0f;
    std::vector<std::string> images;
    const float defaultPosition = 20.0f;

    bilibili::DynamicArticleModuleDraw image;
    bilibili::DynamicArticleModuleArchive video;
    bilibili::DynamicArticleModuleLiveCardInfo live{};
};

class DataSourceDynamicDetailList : public RecyclingGridDataSource, public CommentAction, public DynamicAction {
public:
    DataSourceDynamicDetailList(const bilibili::DynamicArticleResult& data, bilibili::DynamicArticleModuleState state,
                                brls::Event<size_t>* likeState, brls::Event<size_t>* likeNum, int mode,
                                std::function<void(void)> cb)
        : data(data),
          state(std::move(state)),
          likeState(likeState),
          likeNum(likeNum),
          commentMode(mode),
          switchModeCallback(std::move(cb)) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        if (index == 0) {
            DynamicArticleView* item = (DynamicArticleView*)recycler->dequeueReusableCell("Cell");
            item->setCard(data);
            // 为方便修改，使用 state 内的数据
            item->setForwardNum(state.forward.count);
            item->setReplyNum(state.comment.count);
            item->setLikeNum(state.like.count);
            item->setLiked(state.like.like_state);
            return item;
        } else if (index == 1) {
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
        } else if (index == 2) {
            VideoCommentReply* item = (VideoCommentReply*)recycler->dequeueReusableCell("Reply");
            item->setHeight(40);
            return item;
        }

        if (dataList[index - 3].rpid == 1) {  // 添加评论结束提示
            GridHintView* bottom = (GridHintView*)recycler->dequeueReusableCell("Hint");
            bottom->setJustifyContent(brls::JustifyContent::CENTER);
            bottom->hintLabel->setText("wiliwili/player/single_comment/end"_i18n);
            return bottom;
        }

        //从缓存列表中取出 或者 新生成一个表单项
        VideoComment* item = (VideoComment*)recycler->dequeueReusableCell("Comment");

        item->setData(this->dataList[index - 3]);
        return item;
    }

    size_t getItemCount() override { return dataList.size() + 3; }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        if (index == 0) {
            auto* item = dynamic_cast<DynamicArticleView*>(recycler->getGridItemByIndex(index));
            if (!item) return;

            auto* view = new DynamicCommentAction();
            view->setActionData(data, state, item->getY(), item->getWidth());
            auto container = new brls::AppletFrame(view);
            container->setHeaderVisibility(brls::Visibility::GONE);
            container->setFooterVisibility(brls::Visibility::GONE);
            container->setInFadeAnimation(true);
            brls::Application::pushActivity(new brls::Activity(container));

            view->likeClickEvent.subscribe([this, item]() {
                state.like.like_state = !state.like.like_state;
                state.like.count += state.like.like_state ? 1 : -1;
                item->setLiked(state.like.like_state);
                item->setLikeNum(state.like.count);
                // 将点赞情况传递给上一级列表
                likeState->fire(state.like.like_state);
                likeNum->fire(state.like.count);
                this->dynamicLike(data.id_str, state.like.like_state);
            });
            return;
        }
        if (index == 1) {
            if (switchModeCallback) switchModeCallback();
            return;
        }
        if (index == 2) {
            if (!DialogHelper::checkLogin()) return;
            // 回复评论
            brls::Application::getImeManager()->openForText(
                [this, recycler](const std::string& text) {
                    if (text.empty()) return;
                    this->commentReply(text, state.comment.comment_id, 0, 0, state.comment.comment_type,
                                       [this, recycler](const bilibili::VideoCommentAddResult& result) {
                                           this->dataList.insert(dataList.begin(), result.reply);
                                           recycler->reloadData();
                                           // 重新设置一下焦点到 recycler 的默认 cell （顶部）
                                           brls::Application::giveFocus(recycler);
                                       });
                },
                "", "", 500, "", 0);
            return;
        }

        auto* item = dynamic_cast<VideoComment*>(recycler->getGridItemByIndex(index));
        if (!item) return;

        auto* view = new PlayerSingleComment(CommentUiType::COMMENT_UI_TYPE_DYNAMIC);
        view->setCommentData(dataList[index - 3], item->getY(), state.comment.comment_type);
        auto container = new brls::AppletFrame(view);
        container->setHeaderVisibility(brls::Visibility::GONE);
        container->setFooterVisibility(brls::Visibility::GONE);
        container->setInFadeAnimation(true);
        brls::Application::pushActivity(new brls::Activity(container));

        view->likeStateEvent.subscribe([this, item, index](size_t value) {
            auto& itemData  = dataList[index - 3];
            itemData.action = value;
            item->setLiked(value);
        });
        view->likeNumEvent.subscribe([this, item, index](size_t value) {
            auto& itemData = dataList[index - 3];
            itemData.like  = value;
            item->setLikeNum(value);
        });
        view->replyNumEvent.subscribe([this, item, index](size_t value) {
            auto& itemData  = dataList[index - 3];
            itemData.rcount = value;
            item->setReplyNum(value);
        });
        view->deleteEvent.subscribe([this, recycler, index]() {
            dataList.erase(dataList.begin() + index - 3);
            recycler->reloadData();
            // 重新设置一下焦点到 recycler 的默认 cell （顶部）
            brls::Application::giveFocus(recycler);
        });
    }

    void appendData(const bilibili::VideoCommentListResult& result) {
        bool skip = false;
        for (auto& i : result) {
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
    const bilibili::DynamicArticleResult& data;  // 动态原始数据
    bilibili::DynamicArticleModuleState state;  // 动态赞评转数据，为方便修改，不使用 data 内的数据
    brls::Event<size_t>* likeState;
    brls::Event<size_t>* likeNum;
    bilibili::VideoCommentListResult dataList;
    int commentMode                              = 3;  // 2: 按时间；3: 按热度
    std::function<void(void)> switchModeCallback = nullptr;
};

void DynamicArticleDetail::initList(const bilibili::DynamicArticleResult& result,
                                const bilibili::DynamicArticleModuleState& moduleState) {
    data = result;
    state = moduleState;
    this->recyclingGrid->registerCell("Cell", []() { return DynamicArticleView::create(); });
    this->recyclingGrid->registerCell("Reply", []() { return VideoCommentReply::create(); });
    this->recyclingGrid->registerCell("Sort", []() { return VideoCommentSort::create(); });
    this->recyclingGrid->registerCell("Hint", []() { return GridHintView::create(); });
    this->recyclingGrid->registerCell("Comment", []() { return VideoComment::create(); });

    this->recyclingGrid->registerAction("wiliwili/home/common/switch"_i18n, brls::ControllerButton::BUTTON_X,
                                        [this](brls::View* view) -> bool {
                                            this->toggleCommentMode();
                                            return true;
                                        });
    this->recyclingGrid->setDataSource(new DataSourceDynamicDetailList(data, state, &likeStateEvent, &likeNumEvent,
                                                                       this->getVideoCommentMode(),
                                                                       [this]() { this->toggleCommentMode(); }));
    this->recyclingGrid->onNextPage([this]() {
        this->requestVideoComment(this->state.comment.comment_id, -1, -1, this->state.comment.comment_type);
    });
}

DynamicArticleDetail::DynamicArticleDetail(const std::string& id) {
    this->inflateFromXMLRes("xml/fragment/dynamic_detail.xml");
    requestDynamicArticle(id);
    this->buttonClose->setFocusable(true);
    this->buttonClose->registerClickAction([](...){
        brls::Application::popActivity(brls::TransitionAnimation::NONE);
        return true;
    });
    brls::Application::giveFocus(this->buttonClose);
}

DynamicArticleDetail::DynamicArticleDetail(const bilibili::DynamicArticleResult& data,
                                           const bilibili::DynamicArticleModuleState& state) {
    this->inflateFromXMLRes("xml/fragment/dynamic_detail.xml");
    initList(data, state);
}

void DynamicArticleDetail::onDynamicArticle(const bilibili::DynamicArticleResult& result) {
    for (auto& j : result.modules) {
        if ((bilibili::DynamicArticleModuleType)j.data.index() != bilibili::DynamicArticleModuleType::MODULE_TYPE_STAT)
            continue;
        auto* dataState = std::get_if<bilibili::DynamicArticleModuleState>(&j.data);
        if (!dataState) break;
        state = *dataState;
        break;
    }
    initList(result, state);
    this->buttonClose->setFocusable(false);
    if (brls::Application::getCurrentFocus() == this->buttonClose)
        brls::Application::giveFocus(this->recyclingGrid);
}

void DynamicArticleDetail::onCommentInfo(const bilibili::VideoCommentResultWrapper& result) {
    auto* datasource = dynamic_cast<DataSourceDynamicDetailList*>(recyclingGrid->getDataSource());
    if (!datasource) return;
    if (result.requestIndex == 0) {
        // 第一页评论
        //整合置顶评论
        std::vector<bilibili::VideoCommentResult> comments(result.top_replies);
        comments.insert(comments.end(), result.replies.begin(), result.replies.end());
        datasource->appendData(comments);
    } else {
        // 第N页评论
        if (!result.replies.empty()) {
            datasource->appendData(result.replies);
        }
    }
    if (result.cursor.is_end) {
        bilibili::VideoCommentResult bottom;
        bottom.rpid = 1;
        datasource->appendData({bottom});
    }
    recyclingGrid->notifyDataChanged();
}

void DynamicArticleDetail::toggleCommentMode() {
    // 更新请求模式
    int newMode = this->getVideoCommentMode() == 3 ? 2 : 3;
    this->setVideoCommentMode(newMode);
    // 清空评论列表
    this->recyclingGrid->setDataSource(new DataSourceDynamicDetailList(
        data, state, &likeStateEvent, &likeNumEvent, newMode, [this]() { this->toggleCommentMode(); }));
    // 焦点切换到动态
    brls::Application::giveFocus(this->recyclingGrid);
}

DynamicArticleView::DynamicArticleView() {
    this->inflateFromXMLRes("xml/views/dynamic_card.xml");
    if (brls::Application::ORIGINAL_WINDOW_HEIGHT == 544) {
        videoArea->setWidthPercentage(80);
        videoAreaForward->setWidthPercentage(80);
        textBox->setMaxRows(3);
        textBoxForward->setMaxRows(2);
    }
}

void DynamicArticleView::setCard(const bilibili::DynamicArticleResult& result) {
    this->articleData = result;
    // 清空内容
    this->contentArea->setVisibility(brls::Visibility::GONE);
    this->imageArea->setVisibility(brls::Visibility::GONE);
    this->videoArea->setVisibility(brls::Visibility::GONE);
    this->forwardArea->setVisibility(brls::Visibility::GONE);
    this->topicArea->setVisibility(brls::Visibility::GONE);
    this->disputeArea->setVisibility(brls::Visibility::GONE);
    this->state = bilibili::DynamicArticleModuleState();

    for (auto& j : result.modules) {
        switch ((bilibili::DynamicArticleModuleType)j.data.index()) {
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_AUTHOR: {
                auto* data = std::get_if<bilibili::DynamicArticleModuleAuthor>(&j.data);
                if (!data) break;
                // 作者
                this->author->setUserInfo(data->user.face + ImageHelper::face_ext, data->user.name, data->pub_text);
                // 设置作者颜色
                if (data->user.vip.nickname_color.empty()) {
                    this->author->setMainTextColor(brls::Application::getTheme().getColor("brls/text"));
                } else {
                    this->author->getLabelName()->applyXMLAttribute("textColor", data->user.vip.nickname_color);
                }
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_DESC: {
                auto* data = std::get_if<bilibili::DynamicArticleModuleDesc>(&j.data);
                if (!data) break;
                // 文本
                this->textBox->setText(data->text);
                this->contentArea->setVisibility(brls::Visibility::VISIBLE);
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_DATA: {
                auto* internal = std::get_if<bilibili::DynamicArticleModuleData>(&j.data);
                if (!internal) break;
                // 内嵌数据
                switch ((bilibili::DynamicArticleModuleDataType)internal->data.index()) {
                    case bilibili::DynamicArticleModuleDataType::MODULE_TYPE_VIDEO: {
                        auto* video = std::get_if<bilibili::DynamicArticleModuleArchive>(&internal->data);
                        if (!video) break;
                        // 视频
                        this->videoArea->setCard(video->cover + ImageHelper::h_ext, video->title, video->stat.play,
                                                 video->stat.danmaku, video->duration_text);
                        this->videoArea->setVisibility(brls::Visibility::VISIBLE);
                        break;
                    }
                    case bilibili::DynamicArticleModuleDataType::MODULE_TYPE_IMAGE: {
                        auto* image = std::get_if<bilibili::DynamicArticleModuleDraw>(&internal->data);
                        if (!image) break;
                        // 图片
                        RichTextData d;
                        float size = brls::Application::ORIGINAL_WINDOW_HEIGHT == 544 ? 18 : 36;
                        if (image->items.size() <= 3)
                            size *= 4;
                        else if (image->items.size() <= 6)
                            size *= 3;
                        else
                            size *= 2;
                        float margin = brls::Application::ORIGINAL_WINDOW_HEIGHT == 544 ? 4 : 8;
                        int i        = 0;
                        for (auto& p : image->items) {
                            auto item      = std::make_shared<RichTextImage>(p.src + ImageHelper::note_ext, size, size);
                            item->r_margin = margin;
                            item->t_margin = margin;
                            d.emplace_back(item);
                            // 每3张图片，自动换行
                            if (++i % 3 == 0) d.emplace_back(std::make_shared<RichTextBreak>());
                        }
                        this->imageBox->setRichText(d);
                        this->imageArea->setVisibility(brls::Visibility::VISIBLE);
                        break;
                    }
                    case bilibili::DynamicArticleModuleDataType::MODULE_TYPE_LIVE: {
                        auto* live = std::get_if<bilibili::DynamicArticleModuleLive>(&internal->data);
                        if (!live) break;
                        // 直播
                        this->videoArea->setCard(
                            live->card_info.live_play_info.cover + ImageHelper::h_ext,
                            live->card_info.live_play_info.title,
                            live->card_info.live_play_info.watched_show.text_large,
                            fmt::format("{} {}", live->card_info.live_play_info.parent_area_name,
                                        live->card_info.live_play_info.area_name),
                            fmt::format("{} 开始", wiliwili::sec2date(live->card_info.live_play_info.live_start_time)));
                        this->videoArea->setVisibility(brls::Visibility::VISIBLE);
                        break;
                    }
                    case bilibili::DynamicArticleModuleDataType::MODULE_TYPE_FORWARD: {
                        auto* forward = std::get_if<bilibili::DynamicArticleModuleForward>(&internal->data);
                        if (!forward) break;
                        // 转发
                        this->forwardArea->setVisibility(brls::Visibility::VISIBLE);
                        this->setForwardCard(forward->item);
                        break;
                    }
                    case bilibili::DynamicArticleModuleDataType::MODULE_TYPE_NONE:
                    default:
                        brls::Logger::error("\t unknown module data type: {}", internal->type);
                        break;
                }
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_STAT: {
                auto* data = std::get_if<bilibili::DynamicArticleModuleState>(&j.data);
                if (!data) break;
                state = *data;
                // 转发 回复 点赞
                // todo: is_forbidden: 是否禁止转发，评论和点赞是否也有类似情况?
                this->setForwardNum(data->forward.count);
                this->setReplyNum(data->comment.count);
                this->setLikeNum(data->like.count);
                this->setLiked(data->like.like_state);
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_TOPIC: {
                // 话题
                auto* data = std::get_if<bilibili::DynamicArticleModuleTopic>(&j.data);
                if (!data) break;
                this->labelTopic->setText(data->name);
                this->topicArea->setVisibility(brls::Visibility::VISIBLE);
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_DISPUTE: {
                // 警告
                auto* data = std::get_if<bilibili::DynamicArticleModuleDispute>(&j.data);
                if (!data) break;
                this->labelDispute->setText(data->title);
                this->disputeArea->setVisibility(brls::Visibility::VISIBLE);
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_NULL: {
                // 转发的动态已失效
                auto* data = std::get_if<bilibili::DynamicArticleModuleNull>(&j.data);
                if (!data) break;
                this->forwardArea->setVisibility(brls::Visibility::VISIBLE);
                this->contentAreaForward->setVisibility(brls::Visibility::GONE);
                this->imageAreaForward->setVisibility(brls::Visibility::GONE);
                this->videoAreaForward->setVisibility(brls::Visibility::GONE);
                this->topicAreaForward->setVisibility(brls::Visibility::GONE);
                this->disputeAreaForward->setVisibility(brls::Visibility::GONE);
                this->authorForward->setText(data->text);
                this->authorForward->setTextColor(brls::Application::getTheme().getColor("font/grey"));
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_NONE:
            default:
                brls::Logger::error("\t unknown module type: {}", j.module_type);
                break;
        }
    }
}

void DynamicArticleView::setForwardCard(const bilibili::dynamic_forward::DynamicArticleResult& result) {
    // 清空内容
    this->contentAreaForward->setVisibility(brls::Visibility::GONE);
    this->imageAreaForward->setVisibility(brls::Visibility::GONE);
    this->videoAreaForward->setVisibility(brls::Visibility::GONE);
    this->topicAreaForward->setVisibility(brls::Visibility::GONE);
    this->disputeAreaForward->setVisibility(brls::Visibility::GONE);

    for (auto& j : result.modules) {
        switch ((bilibili::DynamicArticleModuleType)j.data.index()) {
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_AUTHOR: {
                auto* data = std::get_if<bilibili::DynamicArticleModuleAuthor>(&j.data);
                if (!data) break;
                // 作者
                this->authorForward->setText("@" + data->user.name);
                if (data->user.vip.nickname_color.empty()) {
                    this->authorForward->setTextColor(brls::Application::getTheme().getColor("color/link"));
                } else {
                    this->authorForward->applyXMLAttribute("textColor", data->user.vip.nickname_color);
                }
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_DESC: {
                auto* data = std::get_if<bilibili::DynamicArticleModuleDesc>(&j.data);
                if (!data) break;
                // 文本
                this->textBoxForward->setText(data->text);
                this->contentAreaForward->setVisibility(brls::Visibility::VISIBLE);
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_DATA: {
                auto* internal = std::get_if<bilibili::dynamic_forward::DynamicArticleModuleData>(&j.data);
                if (!internal) break;
                // 内嵌数据
                switch ((bilibili::DynamicArticleModuleDataType)internal->data.index()) {
                    case bilibili::DynamicArticleModuleDataType::MODULE_TYPE_VIDEO: {
                        auto* video = std::get_if<bilibili::DynamicArticleModuleArchive>(&internal->data);
                        if (!video) break;
                        // 视频
                        this->videoAreaForward->setCard(video->cover + ImageHelper::h_ext, video->title,
                                                        video->stat.play, video->stat.danmaku, video->duration_text);
                        this->videoAreaForward->setVisibility(brls::Visibility::VISIBLE);
                        break;
                    }
                    case bilibili::DynamicArticleModuleDataType::MODULE_TYPE_IMAGE: {
                        auto* image = std::get_if<bilibili::DynamicArticleModuleDraw>(&internal->data);
                        if (!image) break;
                        // 图片
                        RichTextData d;
                        float size = brls::Application::ORIGINAL_WINDOW_HEIGHT == 544 ? 18 : 36;
                        size *= 4 - ((image->items.size() - 1) / 3);
                        float margin = brls::Application::ORIGINAL_WINDOW_HEIGHT == 544 ? 4 : 8;
                        // 显示为正方形缩略图
                        int i = 0;
                        for (auto& p : image->items) {
                            auto item      = std::make_shared<RichTextImage>(p.src + ImageHelper::note_ext, size, size);
                            item->r_margin = margin;
                            item->t_margin = margin;
                            d.emplace_back(item);
                            if (++i % 3 == 0) d.emplace_back(std::make_shared<RichTextBreak>());
                        }
                        this->imageBoxForward->setRichText(d);
                        this->imageAreaForward->setVisibility(brls::Visibility::VISIBLE);
                        break;
                    }
                    case bilibili::DynamicArticleModuleDataType::MODULE_TYPE_LIVE: {
                        auto* live = std::get_if<bilibili::DynamicArticleModuleLive>(&internal->data);
                        if (!live) break;
                        // 直播
                        this->videoAreaForward->setCard(
                            live->card_info.live_play_info.cover + ImageHelper::h_ext,
                            live->card_info.live_play_info.title,
                            live->card_info.live_play_info.watched_show.text_large,
                            fmt::format("{} {}", live->card_info.live_play_info.parent_area_name,
                                        live->card_info.live_play_info.area_name),
                            fmt::format("{} 开始", wiliwili::sec2date(live->card_info.live_play_info.live_start_time)));
                        this->videoAreaForward->setVisibility(brls::Visibility::VISIBLE);
                        break;
                    }
                    case bilibili::DynamicArticleModuleDataType::MODULE_TYPE_NONE:
                    default:
                        brls::Logger::error("\t unknown module data type: {}", internal->type);
                        break;
                }
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_TOPIC: {
                // 话题
                auto* data = std::get_if<bilibili::DynamicArticleModuleTopic>(&j.data);
                if (!data) break;
                this->labelTopicForward->setText(data->name);
                this->topicAreaForward->setVisibility(brls::Visibility::VISIBLE);
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_DISPUTE: {
                // 警告
                auto* data = std::get_if<bilibili::DynamicArticleModuleDispute>(&j.data);
                if (!data) break;
                this->labelDisputeForward->setText(data->title);
                this->disputeAreaForward->setVisibility(brls::Visibility::VISIBLE);
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_NONE:
            default:
                brls::Logger::error("\t unknown module type: {}", j.module_type);
                break;
        }
    }
}

void DynamicArticleView::openDetail() {
    auto* detail    = new DynamicArticleDetail(articleData, state);
    auto* container = new brls::AppletFrame(detail);
    container->setHeaderVisibility(brls::Visibility::GONE);
    container->setFooterVisibility(brls::AppletFrame::HIDE_BOTTOM_BAR ? brls::Visibility::GONE
                                                                      : brls::Visibility::VISIBLE);
    brls::Application::pushActivity(new brls::Activity(container), brls::TransitionAnimation::NONE);

    detail->likeNumEvent.subscribe([this](size_t num) {
        this->setLikeNum(num);
        this->state.like.count = num;
    });

    detail->likeStateEvent.subscribe([this](size_t value) {
        this->setLiked(value);
        this->state.like.like_state = value;
    });
}

void DynamicArticleView::prepareForReuse() {}

void DynamicArticleView::cacheForReuse() {
    ImageHelper::clear(this->author->getAvatar());
    if (this->videoArea->getVisibility() == brls::Visibility::VISIBLE) {
        ImageHelper::clear(this->videoArea->picture);
    }
    if (this->videoAreaForward->getVisibility() == brls::Visibility::VISIBLE) {
        ImageHelper::clear(this->videoAreaForward->picture);
    }
}

void DynamicArticleView::setGrow() {
    this->textBox->setMaxRows(SIZE_T_MAX);
    this->textBoxForward->setMaxRows(SIZE_T_MAX);
}

void DynamicArticleView::setLiked(bool liked) {
    if (liked) {
        this->svgLike->setImageFromSVGRes("svg/comment-agree-active.svg");
    } else {
        this->svgLike->setImageFromSVGRes("svg/comment-agree-grey.svg");
    }
}

void DynamicArticleView::setLikeNum(size_t num) { this->labelLike->setText(num == 0 ? "点赞" : wiliwili::num2w(num)); }

void DynamicArticleView::setReplyNum(size_t num) {
    this->labelReply->setText(num == 0 ? "评论" : wiliwili::num2w(num));
}

void DynamicArticleView::setForwardNum(size_t num) {
    this->labelFroward->setText(num == 0 ? "转发" : wiliwili::num2w(num));
}

RecyclingGridItem* DynamicArticleView::create() { return new DynamicArticleView(); }