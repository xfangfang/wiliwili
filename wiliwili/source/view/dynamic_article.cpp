//
// Created by fang on 2024/3/22.
//

#include <borealis/views/applet_frame.hpp>
#include <borealis/core/thread.hpp>

#include "view/dynamic_article.hpp"
#include "view/text_box.hpp"
#include "view/dynamic_video_card.hpp"
#include "view/video_comment.hpp"
#include "utils/image_helper.hpp"
#include "utils/number_helper.hpp"
#include "utils/string_helper.hpp"
#include "fragment/player_single_comment.hpp"
#include "utils/dialog_helper.hpp"

using namespace brls::literals;

class DataSourceDynamicDetailList : public RecyclingGridDataSource, public CommentAction {
public:
    DataSourceDynamicDetailList(const bilibili::DynamicArticleResult& data,
                                const bilibili::DynamicArticleModuleState& state, int mode,
                                std::function<void(void)> cb)
        : data(data), state(state), commentMode(mode), switchModeCallback(cb) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        if (index == 0) {
            DynamicArticleView* item = (DynamicArticleView*)recycler->dequeueReusableCell("Cell");
            item->setCard(data);
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
            // todo: 展开详情
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

        view->likeStateEvent.subscribe([this, item, index](bool value) {
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
    const bilibili::DynamicArticleResult& data;
    const bilibili::DynamicArticleModuleState& state;

    bilibili::VideoCommentListResult dataList;
    int commentMode                              = 3;  // 2: 按时间；3: 按热度
    std::function<void(void)> switchModeCallback = nullptr;
};

class DynamicArticleDetail : public brls::Box, public CommentRequest {
public:
    explicit DynamicArticleDetail(const bilibili::DynamicArticleResult& data,
                                  const bilibili::DynamicArticleModuleState& state)
        : data(data), state(state) {
        this->inflateFromXMLRes("xml/fragment/dynamic_detail.xml");

        this->recyclingGrid->registerCell("Cell", []() { return DynamicArticleView::create(); });

        this->recyclingGrid->registerCell("Reply", []() { return VideoCommentReply::create(); });

        this->recyclingGrid->registerCell("Sort", []() { return VideoCommentSort::create(); });

        this->recyclingGrid->registerCell("Hint", []() { return GridHintView::create(); });

        this->recyclingGrid->registerCell("Comment", []() { return VideoComment::create(); });

        this->recyclingGrid->setDataSource(new DataSourceDynamicDetailList(data, state, this->getVideoCommentMode(),
                                                                           [this]() { this->toggleCommentMode(); }));

        this->recyclingGrid->registerAction("wiliwili/home/common/switch"_i18n, brls::ControllerButton::BUTTON_X,
                                            [this](brls::View* view) -> bool {
                                                this->toggleCommentMode();
                                                return true;
                                            });

        this->recyclingGrid->onNextPage([this]() {
            this->requestVideoComment(this->state.comment.comment_id, -1, -1, this->state.comment.comment_type);
        });
    }

    void onCommentInfo(const bilibili::VideoCommentResultWrapper& result) override {
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

    void toggleCommentMode() {
        // 更新请求模式
        int newMode = this->getVideoCommentMode() == 3 ? 2 : 3;
        this->setVideoCommentMode(newMode);
        // 清空评论列表
        this->recyclingGrid->setDataSource(
            new DataSourceDynamicDetailList(data, state, newMode, [this]() { this->toggleCommentMode(); }));
        // 焦点切换到动态
        brls::Application::giveFocus(this->recyclingGrid);
    }

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "dynamic/detail/grid");

    bilibili::DynamicArticleResult data;
    bilibili::DynamicArticleModuleState state;
};

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
    this->state = bilibili::DynamicArticleModuleState();

    for (auto& j : result.modules) {
        switch ((bilibili::DynamicArticleModuleType)j.data.index()) {
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_AUTHOR: {
                auto* data = std::get_if<bilibili::DynamicArticleModuleAuthor>(&j.data);
                if (!data) break;
                // 作者
                this->author->setUserInfo(data->user.face + ImageHelper::face_ext, data->user.name, data->pub_text);
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
                // todo: 自己是否已经点过赞
                this->labelFroward->setText(data->forward.count == 0 ? "转发" : wiliwili::num2w(data->forward.count));
                this->labelReply->setText(data->comment.count == 0 ? "评论" : wiliwili::num2w(data->comment.count));
                this->labelLike->setText(data->like.count == 0 ? "点赞" : wiliwili::num2w(data->like.count));
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
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_NULL: {
                // 转发的动态已失效
                auto* data = std::get_if<bilibili::DynamicArticleModuleNull>(&j.data);
                if (!data) break;
                this->forwardArea->setVisibility(brls::Visibility::VISIBLE);
                this->contentAreaForward->setVisibility(brls::Visibility::GONE);
                this->imageAreaForward->setVisibility(brls::Visibility::GONE);
                this->videoAreaForward->setVisibility(brls::Visibility::GONE);
                this->topicAreaForward->setVisibility(brls::Visibility::GONE);
                this->authorForward->setText(data->text);
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

    for (auto& j : result.modules) {
        switch ((bilibili::DynamicArticleModuleType)j.data.index()) {
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_AUTHOR: {
                auto* data = std::get_if<bilibili::DynamicArticleModuleAuthor>(&j.data);
                if (!data) break;
                // 作者
                this->authorForward->setText("@" + data->user.name);
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
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_NONE:
            default:
                brls::Logger::error("\t unknown module type: {}", j.module_type);
                break;
        }
    }
}

void DynamicArticleView::openDetail() {
    auto container = new brls::AppletFrame(new DynamicArticleDetail(articleData, state));
    container->setHeaderVisibility(brls::Visibility::GONE);
    container->setFooterVisibility(brls::AppletFrame::HIDE_BOTTOM_BAR ? brls::Visibility::GONE
                                                                      : brls::Visibility::VISIBLE);
    brls::Application::pushActivity(new brls::Activity(container), brls::TransitionAnimation::NONE);
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

RecyclingGridItem* DynamicArticleView::create() { return new DynamicArticleView(); }