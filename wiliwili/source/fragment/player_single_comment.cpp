//
// Created by fang on 2023/1/6.
//

#include <utility>
#include <borealis/views/applet_frame.hpp>
#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/core/thread.hpp>

#include "fragment/player_single_comment.hpp"
#include "view/video_comment.hpp"
#include "view/recycling_grid.hpp"
#include "view/button_close.hpp"
#include "view/svg_image.hpp"
#include "utils/image_helper.hpp"
#include "utils/dialog_helper.hpp"
#include "utils/config_helper.hpp"
#include "utils/number_helper.hpp"
#include "utils/string_helper.hpp"
#include "utils/activity_helper.hpp"
#include "utils/gesture_helper.hpp"
#include "presenter/comment_related.hpp"
#include "bilibili.h"

using namespace brls::literals;

/// GridRelatedView
class GridRelatedView : public RecyclingGridItem {
public:
    GridRelatedView() {
        this->setFocusable(false);
        this->setMarginBottom(20);
        hintLabel = new brls::Label();
        hintLabel->setFontSize(16);
        hintLabel->setMarginLeft(8);
        hintLabel->setTextColor(brls::Application::getTheme().getColor("font/grey"));
        this->addView(hintLabel);
    }

    void setNum(size_t num) const {
        if (num <= 0) {
            this->hintLabel->setText("");
        } else {
            this->hintLabel->setText(
                wiliwili::format("wiliwili/player/single_comment/related"_i18n, wiliwili::num2w(num)));
        }
    }

    static RecyclingGridItem* create() { return new GridRelatedView(); }

    brls::Label* hintLabel;
};

/// DataSourceCommentList

class DataSourceSingleCommentList : public RecyclingGridDataSource, public CommentAction {
public:
    DataSourceSingleCommentList(bilibili::VideoCommentListResult result, int type, CommentUiType uiType,
                                brls::Event<size_t>* likeState, brls::Event<size_t>* likeNum,
                                brls::Event<size_t>* replyNum, brls::Event<>* deleteReply)
        : dataList(std::move(result)),
          commentType(type),
          uiType(uiType),
          likeState(likeState),
          likeNum(likeNum),
          replyNum(replyNum),
          deleteReply(deleteReply) {
        user_mid = ProgramConfig::instance().getUserID();
    }
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        if (dataList[index].rpid == 0) {  // 添加子回复数量提示
            GridRelatedView* related = (GridRelatedView*)recycler->dequeueReusableCell("Related");
            related->setNum(dataList[index].rcount);
            return related;
        } else if (dataList[index].rpid == 1) {  // 添加评论结束提示
            GridHintView* bottom = (GridHintView*)recycler->dequeueReusableCell("Hint");
            bottom->setJustifyContent(brls::JustifyContent::CENTER);
            bottom->hintLabel->setText("wiliwili/player/single_comment/end"_i18n);
            return bottom;
        }

        //从缓存列表中取出 或者 新生成一个表单项
        VideoComment* item = (VideoComment*)recycler->dequeueReusableCell("Cell");

        item->setData(this->dataList[index]);
        if (index == 0) {
            item->setLineColor(nvgRGBA(180, 180, 180, 60));
            item->setLineBottom(1);
            item->hideReplyIcon(false);
        } else {
            item->setLineBottom(0);
            item->hideReplyIcon(true);
        }
        return item;
    }

    size_t getItemCount() override { return dataList.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        if (dataList[index].rpid == 0 || dataList[index].rpid == 1) {
            return;
        }
        if (!DialogHelper::checkLogin()) return;

        auto* item = dynamic_cast<VideoComment*>(recycler->getGridItemByIndex(index));
        if (!item) return;

        auto* view = new PlayerCommentAction(uiType);
        view->setActionData(dataList[index], item->getY());
        if (user_mid == dataList[index].member.mid) view->showDelete();
        auto container = new brls::AppletFrame(view);
        container->setHeaderVisibility(brls::Visibility::GONE);
        container->setFooterVisibility(brls::Visibility::GONE);
        container->setInFadeAnimation(true);
        brls::Application::pushActivity(new brls::Activity(container));

        view->likeClickEvent.subscribe([this, item, index](size_t action) {
            auto& itemData  = dataList[index];
            // 点赞/取消点赞 或 点踩/取消点踩
            bool isLike = action == 1 || (action == 0 & itemData.action == 1);
            if (action == 1) {
                // 最新状态变为点赞，点赞数 +1
                itemData.like++;
            } else if (itemData.action == 1) {
                // 最新状态由点赞转为 取消或点踩，点赞数 -1
                itemData.like--;
            }

            itemData.action = action;
            item->setLiked(itemData.action);
            item->setLikeNum(itemData.like);

            // 只将对层主的点赞情况传递给上一级列表
            if (index == 0) {
                likeState->fire(itemData.action);
                likeNum->fire(itemData.like);
            }

            if (isLike) {
                this->commentLike(std::to_string(itemData.oid), itemData.rpid, itemData.action, commentType);
            } else {
                this->commentDislike(std::to_string(itemData.oid), itemData.rpid, itemData.action, commentType);
            }
        });

        view->replyClickEvent.subscribe([this, index, recycler]() {
            brls::Application::getImeManager()->openForText(
                [this, index, recycler](std::string text) {
                    // 更新显示的评论数量
                    this->updateCommentLabelNum(recycler, dataList[0].rcount + 1);

                    auto& itemData = dataList[index];

                    // 若回复指定人而不是回复层主，需要@被回复人
                    if (itemData.rpid != itemData.root) {
                        text = fmt::format("回复 @{} :{}", itemData.member.uname, text);
                    }

                    this->commentReply(text, std::to_string(itemData.oid), itemData.rpid, itemData.root, commentType,
                                       [this, recycler](const bilibili::VideoCommentAddResult& result) {
                                           this->dataList.insert(dataList.begin() + 2, result.reply);
                                           recycler->reloadData();
                                           // 重新设置一下焦点到 recycler 的默认 cell （顶部）
                                           brls::Application::giveFocus(recycler);
                                       });
                },
                "", "", 500, "", 0);
        });

        view->deleteClickEvent.subscribe([this, recycler, index]() {
            DialogHelper::showCancelableDialog("wiliwili/player/single_comment/delete"_i18n, [this, recycler, index]() {
                auto& itemData = dataList[index];
                this->commentDelete(std::to_string(itemData.oid), itemData.rpid, commentType);

                if (index == 0) {
                    // 删除一整层
                    deleteReply->fire();
                } else {
                    // 删除单条回复
                    dataList.erase(dataList.begin() + index);
                    recycler->reloadData();
                    // 重新设置一下焦点到 recycler 的默认 cell （顶部）
                    brls::Application::giveFocus(recycler);
                    // 更新评论数量
                    this->updateCommentLabelNum(recycler, dataList[0].rcount - 1);
                }
            });
        });
    }

    void updateCommentLabelNum(RecyclingGrid* recycler, size_t wholeNum) {
        dataList[0].rcount = wholeNum;
        dataList[1].rcount = wholeNum;

        // 设置上级数量
        replyNum->fire(wholeNum);

        // 更新层主显示的评论数量
        auto* topItem = dynamic_cast<VideoComment*>(recycler->getGridItemByIndex(0));
        if (topItem) {
            topItem->setReplyNum(wholeNum);
        }

        // 相关回复
        auto* relatedItem = dynamic_cast<GridRelatedView*>(recycler->getGridItemByIndex(1));
        if (relatedItem) relatedItem->setNum(wholeNum);
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
    int commentType;
    CommentUiType uiType;
    brls::Event<size_t>* likeState;
    brls::Event<size_t>* likeNum;
    brls::Event<size_t>* replyNum;
    brls::Event<>* deleteReply;
    std::string user_mid;
};

/// PlayerSingleComment

PlayerSingleComment::PlayerSingleComment(CommentUiType type) : uiType(type) {
    if (type == COMMENT_UI_TYPE_DYNAMIC) {
        this->inflateFromXMLRes("xml/fragment/player_single_comment_dynamic.xml");
    } else {
        this->inflateFromXMLRes("xml/fragment/player_single_comment.xml");
    }
    brls::Logger::debug("Fragment PlayerSingleComment: create");

    this->recyclingGrid->registerCell("Cell", []() { return VideoComment::create(); });

    this->recyclingGrid->registerCell("Hint", []() { return GridHintView::create(); });

    this->recyclingGrid->registerCell("Related", []() { return GridRelatedView::create(); });

    this->recyclingGrid->onNextPage([this]() { this->requestData(); });

    this->closeBtn->registerClickAction([this](...) {
        this->dismiss();
        return true;
    });

    this->cancel->registerClickAction([this](...) {
        this->dismiss();
        return true;
    });
    this->cancel->addGestureRecognizer(new brls::TapGestureRecognizer(this->cancel));

    this->deleteEvent.subscribe([]() {
        // 直接移除，避免关闭动画导致焦点错乱
        brls::Application::popActivity(brls::TransitionAnimation::NONE);
    });

    this->registerAction("wiliwili/home/common/refresh"_i18n, brls::ControllerButton::BUTTON_X,
                         [this](brls::View* view) {
                             brls::Application::giveFocus(this->closeBtn);
                             this->recyclingGrid->forceRequestNextPage();
                             this->setCommentData(this->root, NAN, this->commentType);
                             return true;
                         });
}

void PlayerSingleComment::setCommentData(const bilibili::VideoCommentResult& result, float y, int type) {
    GA("single_comment", {{"id", std::to_string(result.rpid)}})
    GA("single_comment", {{"comment_id", std::to_string(result.rpid)}})

    this->root        = result;
    this->commentType = type;
    // 将楼主的root id设置为评论id，方便点击时一视同仁地判断
    this->root.root     = root.rpid;
    this->cursor.next   = 0;
    this->cursor.is_end = false;

    // 添加楼主评论，添加后，会因为列表中只有一个元素填不满画面而自动调用加载下一页
    bilibili::VideoCommentResult r;
    r.rpid   = 0;
    r.rcount = root.rcount;
    bilibili::VideoCommentListResult defaultData{root, r};
    recyclingGrid->setDataSource(new DataSourceSingleCommentList(defaultData, commentType, uiType, &likeStateEvent,
                                                                 &likeNumEvent, &replyNumEvent, &deleteEvent));

    this->showStartAnimation(y);
}

void PlayerSingleComment::showStartAnimation(float y) {
    if (isnan(y)) return;
    commentOriginalPosition = y;

    brls::Application::blockInputs();
    this->position.stop();
    this->position.reset(commentOriginalPosition);
    this->position.addStep(60, 300, brls::EasingFunction::quadraticOut);
    this->position.setTickCallback([this] {
        this->backgroundBox->setPositionTop(this->position - 60);
        float alpha = 1 - fabs((this->position - 60) / commentOriginalPosition);
        this->backgroundBox->setAlpha(alpha);
        this->setAlpha(alpha);
    });
    this->position.setEndCallback([](bool finished) { brls::Application::unblockInputs(); });
    this->position.start();
}

void PlayerSingleComment::showDismissAnimation() {
    auto* item            = dynamic_cast<VideoComment*>(recyclingGrid->getGridItemByIndex(0));
    float animationLength = 720.0f;
    if (brls::Application::getCurrentFocus() == item) {
        // 当焦点在第一个元素时，下滑到原本的评论的位置而不是屏幕底部，这样观感更好
        animationLength = commentOriginalPosition - 60;
    }

    brls::Application::blockInputs();
    this->position.stop();
    this->position.reset(0);
    this->position.addStep(animationLength, 300, brls::EasingFunction::quadraticIn);
    this->position.setTickCallback([this, animationLength] {
        this->backgroundBox->setPositionTop(this->position);
        float alpha = 1 - fabs(this->position / animationLength);
        this->backgroundBox->setAlpha(alpha);
        this->setAlpha(alpha);
    });
    this->position.setEndCallback([](bool finished) {
        brls::Application::unblockInputs();
        brls::Application::popActivity(brls::TransitionAnimation::NONE);
    });
    this->position.start();
}

void PlayerSingleComment::dismiss(std::function<void(void)> cb) { this->showDismissAnimation(); }

void PlayerSingleComment::requestData() {
    if (cursor.is_end) return;
    brls::Logger::debug("请求评论: oid: {} rpid:{} type:{} page:{}", root.oid, root.rpid, cursor.next, commentType);
    // request comments
    ASYNC_RETAIN
    BILI::get_comment_detail(
        ProgramConfig::instance().getCSRF(), std::to_string(root.oid), root.rpid, cursor.next, commentType,
        [ASYNC_TOKEN](bilibili::VideoSingleCommentDetail result) {
            brls::sync([ASYNC_TOKEN, result]() mutable {
                ASYNC_RELEASE

                brls::Logger::debug("请求评论结束: root:{} page:{} is_end:{}", root.rpid, result.cursor.next,
                                    result.cursor.is_end);
                cursor.next   = result.cursor.next;
                cursor.is_end = result.cursor.is_end;

                std::string uploader_mid = std::to_string(result.upper);

                auto* ds = dynamic_cast<DataSourceSingleCommentList*>(recyclingGrid->getDataSource());
                if (!ds) return;

                // 更新层主评论状态
                if (result.cursor.is_begin) {
                    ds->updateCommentLabelNum(recyclingGrid, result.root.rcount);
                    this->root = result.root;
                    likeNumEvent.fire(result.root.like);
                    likeStateEvent.fire(result.root.action);
                    replyNumEvent.fire(result.root.rcount);
                }

                // 设置是否为up主
                for (auto& i : result.root.replies) i.member.is_uploader = i.member.mid == uploader_mid;

                // 根据元素的数量来检查是否加载结束，2为楼主与回复数提示
                if (ds->getItemCount() - 2 + result.root.replies.size() >= result.root.rcount) cursor.is_end = true;

                if (cursor.is_end) {
                    bilibili::VideoCommentResult bottom;
                    bottom.rpid = 1;
                    result.root.replies.emplace_back(bottom);
                }

                // 非首页评论
                if (!result.root.replies.empty()) {
                    ds->appendData(result.root.replies);
                    recyclingGrid->notifyDataChanged();
                }
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::Logger::error("{}", error);
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                DialogHelper::showDialog(error);
            });
        });
}

PlayerSingleComment::~PlayerSingleComment() { brls::Logger::debug("Fragment PlayerSingleComment: delete"); }

brls::View* PlayerSingleComment::getDefaultFocus() { return this->recyclingGrid; }

/// PlayerCommentAction

PlayerCommentAction::PlayerCommentAction(CommentUiType type) {
    if (type == COMMENT_UI_TYPE_DYNAMIC) {
        if (brls::Application::ORIGINAL_WINDOW_HEIGHT < 720) {
            this->inflateFromXMLRes("xml/fragment/player_comment_action_dynamic_sm.xml");
        } else {
            this->inflateFromXMLRes("xml/fragment/player_comment_action_dynamic.xml");
        }
    } else {
        this->inflateFromXMLRes("xml/fragment/player_comment_action.xml");
    }

    this->comment->setMaxRows(SIZE_T_MAX);

    this->svgLike->registerClickAction([this](...) {
        this->dismiss();
        this->likeClickEvent.fire(this->comment->getData().action == 1 ? 0 : 1);
        return true;
    });
    this->svgDisLike->registerClickAction([this](...) {
        this->dismiss();
        this->likeClickEvent.fire(this->comment->getData().action == 2 ? 0 : 2);
        return true;
    });
    this->svgReply->registerClickAction([this](...) {
        // 直接退出不调用动画
        // 相关issue: https://github.com/xfangfang/wiliwili/issues/108
        brls::Application::popActivity(brls::TransitionAnimation::NONE);
        this->replyClickEvent.fire();
        return true;
    });
    this->svgDelete->registerClickAction([this](...) {
        // Same as above
        brls::Application::popActivity(brls::TransitionAnimation::NONE);
        this->deleteClickEvent.fire();
        return true;
    });
    this->svgGallery->registerClickAction([this](...) {
        std::vector<std::string> data;
#ifdef __PSV__
        const std::string note_raw_ext = "@300h.jpg";
#else
        const std::string note_raw_ext = "@!web-comment-note.jpg";
#endif
        for (auto& i : this->comment->getData().content.pictures) {
            std::string raw_ext = ImageHelper::note_raw_ext;
            if (i.img_src.size() > 4 && i.img_src.substr(i.img_src.size() - 4, 4) == ".gif") {
                // gif 图片暂时按照 jpg 来解析
                raw_ext = note_raw_ext;
            }
            data.emplace_back(i.img_src + raw_ext);
        }
        Intent::openGallery(data);
        return true;
    });

    this->svgLike->addGestureRecognizer(new brls::TapGestureRecognizer(this->svgLike));
    this->svgDisLike->addGestureRecognizer(new brls::TapGestureRecognizer(this->svgDisLike));
    this->svgReply->addGestureRecognizer(new brls::TapGestureRecognizer(this->svgReply));
    this->svgDelete->addGestureRecognizer(new brls::TapGestureRecognizer(this->svgDelete));
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
        this->comment->getParent()->setPositionTop(this->position - 60);
        float alpha = 1 - fabs((this->position - 60) / commentOriginalPosition);
        this->backgroundBox->setAlpha(alpha);
        this->actionBox->setAlpha(alpha);
    });
}

void PlayerCommentAction::setActionData(const bilibili::VideoCommentResult& data, float y) {
    this->comment->setData(data);
    if (!data.content.pictures.empty()) this->svgGallery->setVisibility(brls::Visibility::VISIBLE);
    if (data.rpid != data.root) this->comment->hideReplyIcon(true);
    commentOriginalPosition = y;
    this->showStartAnimation();
}

void PlayerCommentAction::showStartAnimation() {
    if (commentOriginalPosition == 60) return;

    brls::Application::blockInputs();
    this->position.stop();
    this->position.reset(commentOriginalPosition);
    this->position.addStep(60, 200, brls::EasingFunction::quadraticOut);
    this->position.setEndCallback([](bool finished) { brls::Application::unblockInputs(); });
    this->position.start();
}

void PlayerCommentAction::showDismissAnimation() {
    if (commentOriginalPosition == 60) {
        brls::Application::popActivity(brls::TransitionAnimation::NONE);
        return;
    }

    brls::Application::blockInputs();
    this->position.stop();
    this->position.reset(60);
    this->position.addStep(commentOriginalPosition, 200, brls::EasingFunction::quadraticIn);
    this->position.setEndCallback([](bool finished) {
        brls::Application::unblockInputs();
        brls::Application::popActivity(brls::TransitionAnimation::NONE);
    });
    this->position.start();
}

void PlayerCommentAction::showDelete() { this->svgDelete->setVisibility(brls::Visibility::VISIBLE); }

brls::View* PlayerCommentAction::getDefaultFocus() {
    if (this->svgGallery->getVisibility() == brls::Visibility::VISIBLE) {
        return this->svgGallery;
    }
    return this->svgLike;
}

void PlayerCommentAction::dismiss(std::function<void(void)> cb) { this->showDismissAnimation(); }
