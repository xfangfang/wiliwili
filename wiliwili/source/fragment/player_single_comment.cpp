//
// Created by fang on 2023/1/6.
//

#include "fragment/player_single_comment.hpp"
#include "view/video_comment.hpp"
#include "view/recycling_grid.hpp"
#include "view/button_close.hpp"
#include "utils/dialog_helper.hpp"
#include "utils/config_helper.hpp"
#include "presenter/comment_related.hpp"
#include "bilibili.h"
#include <borealis/platforms/switch/swkbd.hpp>

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
        hintLabel->setTextColor(
            brls::Application::getTheme().getColor("font/grey"));
        this->addView(hintLabel);
    }

    void setNum(size_t num) {
        if (num <= 0) {
            this->hintLabel->setText("");
        } else {
            this->hintLabel->setText(
                fmt::format("相关回复共{}条", wiliwili::num2w(num)));
        }
    }

    static RecyclingGridItem* create() { return new GridRelatedView(); }

    brls::Label* hintLabel;
};

/// DataSourceCommentList

class DataSourceSingleCommentList : public RecyclingGridDataSource,
                                    public CommentRequest {
public:
    DataSourceSingleCommentList(bilibili::VideoCommentListResult result,
                                brls::Event<bool>* likeState,
                                brls::Event<size_t>* likeNum,
                                brls::Event<size_t>* replyNum,
                                brls::Event<>* deleteReply)
        : dataList(result),
          likeState(likeState),
          likeNum(likeNum),
          replyNum(replyNum),
          deleteReply(deleteReply) {
        user_mid = ProgramConfig::instance().getUserID();
    }
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler,
                                  size_t index) override {
        if (dataList[index].rpid == 0) {  // 添加子回复数量提示
            GridRelatedView* related =
                (GridRelatedView*)recycler->dequeueReusableCell("Related");
            related->setNum(dataList[index].rcount);
            return related;
        } else if (dataList[index].rpid == 1) {  // 添加评论结束提示
            GridHintView* bottom =
                (GridHintView*)recycler->dequeueReusableCell("Hint");
            bottom->setJustifyContent(brls::JustifyContent::CENTER);
            bottom->hintLabel->setText("没有更多评论");
            return bottom;
        }

        //从缓存列表中取出 或者 新生成一个表单项
        VideoComment* item =
            (VideoComment*)recycler->dequeueReusableCell("Cell");

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

        VideoComment* item =
            dynamic_cast<VideoComment*>(recycler->getGridItemByIndex(index));
        if (!item) return;

        PlayerCommentAction* view = new PlayerCommentAction();
        view->setActionData(dataList[index], item->getY());
        if (user_mid == dataList[index].member.mid) view->showDelete();
        auto container = new brls::AppletFrame(view);
        container->setHeaderVisibility(brls::Visibility::GONE);
        container->setFooterVisibility(brls::Visibility::GONE);
        container->setInFadeAnimation(true);
        brls::Application::pushActivity(new brls::Activity(container));

        view->likeClickEvent.subscribe([this, item, index]() {
            auto& itemData  = dataList[index];
            itemData.action = !itemData.action;
            itemData.like += itemData.action ? 1 : -1;
            item->setData(itemData);

            // 只将对层主的点赞情况传递给上一级列表
            if (index == 0) {
                likeState->fire(itemData.action);
                likeNum->fire(itemData.like);
            }

            this->commentLike(itemData.oid, itemData.rpid, itemData.action);
        });

        view->replyClickEvent.subscribe([this, index, recycler]() {
            brls::Swkbd::openForText(
                [this, index, recycler](std::string text) {
                    // 更新显示的评论数量
                    this->updateCommentLabelNum(recycler,
                                                dataList[0].rcount + 1);

                    auto& itemData = dataList[index];

                    // 若回复指定人而不是回复层主，需要@被回复人
                    if (itemData.rpid != itemData.root) {
                        text = fmt::format("回复 @{} : {}",
                                           itemData.member.uname, text);
                    }

                    this->commentReply(
                        text, itemData.oid, itemData.rpid, itemData.root,
                        [this, recycler](
                            const bilibili::VideoCommentAddResult& result) {
                            this->dataList.insert(dataList.begin() + 2,
                                                  result.reply);
                            recycler->reloadData();
                        });
                },
                "", "", 500, "", 0);
        });

        view->deleteClickEvent.subscribe([this, recycler, index]() {
            DialogHelper::showCancelableDialog(
                "删除评论后，评论下所有回复都会被删除\n是否继续?",
                [this, recycler, index]() {
                    auto& itemData = dataList[index];
                    this->commentDelete(itemData.oid, itemData.rpid);

                    brls::Logger::error("current reply: {}",
                                        dataList[0].rcount - 1);
                    if (index == 0) {
                        // 删除一整层
                        deleteReply->fire();
                    } else {
                        // 删除单条回复
                        dataList.erase(dataList.begin() + index);
                        recycler->reloadData();
                        // 更新评论数量
                        this->updateCommentLabelNum(recycler,
                                                    dataList[0].rcount - 1);
                    }
                });
        });
    }

    void updateCommentLabelNum(RecyclingGrid* recycler, size_t wholeNum) {
        dataList[0].rcount = wholeNum;
        dataList[1].rcount = wholeNum;

        // 设置上级数量
        replyNum->fire(wholeNum);

        // 层主显示的数量
        VideoComment* topItem =
            dynamic_cast<VideoComment*>(recycler->getGridItemByIndex(0));
        if (topItem) topItem->setData(dataList[0]);

        // 相关回复
        GridRelatedView* relatedItem =
            dynamic_cast<GridRelatedView*>(recycler->getGridItemByIndex(1));
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
    brls::Event<bool>* likeState;
    brls::Event<size_t>* likeNum;
    brls::Event<size_t>* replyNum;
    brls::Event<>* deleteReply;
    std::string user_mid;
};

/// PlayerSingleComment

PlayerSingleComment::PlayerSingleComment() {
    this->inflateFromXMLRes("xml/fragment/player_single_comment.xml");
    brls::Logger::debug("Fragment PlayerSingleComment: create");

    this->recyclingGrid->registerCell("Cell",
                                      []() { return VideoComment::create(); });

    this->recyclingGrid->registerCell("Hint",
                                      []() { return GridHintView::create(); });

    this->recyclingGrid->registerCell(
        "Related", []() { return GridRelatedView::create(); });

    this->recyclingGrid->onNextPage([this]() { this->requestData(); });

    this->closeBtn->registerClickAction([this](...) {
        this->dismiss();
        return true;
    });

    this->deleteEvent.subscribe([this]() { this->dismiss(); });

    this->registerAction("wiliwili/home/common/refresh"_i18n,
                         brls::ControllerButton::BUTTON_X,
                         [this](brls::View* view) {
                             brls::Application::giveFocus(this->closeBtn);
                             this->recyclingGrid->forceRequestNextPage();
                             this->setCommentData(this->root);
                             return true;
                         });
}

void PlayerSingleComment::setCommentData(
    const bilibili::VideoCommentResult& result) {
    this->root = result;
    // 将楼主的root id设置为评论id，方便点击时一视同仁地判断
    this->root.root     = root.rpid;
    this->cursor.next   = 0;
    this->cursor.is_end = false;

    // 添加楼主评论，添加后，会因为列表中只有一个元素填不满画面而自动调用加载下一页
    bilibili::VideoCommentResult r;
    r.rpid   = 0;
    r.rcount = root.rcount;
    bilibili::VideoCommentListResult defaultData{root, r};
    recyclingGrid->setDataSource(new DataSourceSingleCommentList(
        defaultData, &likeStateEvent, &likeNumEvent, &replyNumEvent,
        &deleteEvent));
}

void PlayerSingleComment::dismiss(std::function<void(void)> cb) {
    brls::Application::popActivity(brls::TransitionAnimation::NONE);
}

void PlayerSingleComment::requestData() {
    if (cursor.is_end) return;
    brls::Logger::debug("请求评论: root:{} page:{}", root.rpid, cursor.next);
    // request comments
    ASYNC_RETAIN
    BILI::get_comment_detail(
        ProgramConfig::instance().getCSRF(), root.oid, root.rpid, cursor.next,
        [ASYNC_TOKEN](bilibili::VideoSingleCommentDetail result) {
            brls::sync([ASYNC_TOKEN, result]() mutable {
                ASYNC_RELEASE

                brls::Logger::debug("请求评论结束: root:{} page:{} is_end:{}",
                                    root.rpid, result.cursor.next,
                                    result.cursor.is_end);
                cursor.next   = result.cursor.next;
                cursor.is_end = result.cursor.is_end;

                std::string uploader_mid = std::to_string(result.upper);

                DataSourceSingleCommentList* ds =
                    dynamic_cast<DataSourceSingleCommentList*>(
                        recyclingGrid->getDataSource());
                if (!ds) return;

                // 更新层主评论状态
                if (result.cursor.is_begin) {
                    ds->updateCommentLabelNum(recyclingGrid,
                                              result.root.rcount);
                    this->root = result.root;
                    likeNumEvent.fire(result.root.like);
                    likeStateEvent.fire(result.root.action);
                    replyNumEvent.fire(result.root.rcount);
                }

                // 设置是否为up主
                for (auto& i : result.root.replies)
                    i.member.is_uploader = i.member.mid == uploader_mid;

                // 根据元素的数量来检查是否加载结束，2为楼主与回复数提示
                if (ds->getItemCount() - 2 + result.root.replies.size() >=
                    result.root.rcount)
                    cursor.is_end = true;

                if (cursor.is_end) {
                    bilibili::VideoCommentResult bottom;
                    bottom.rpid = 1;
                    result.root.replies.emplace_back(bottom);
                }

                // 非首页评论
                ds->appendData(result.root.replies);
                recyclingGrid->notifyDataChanged();
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::Logger::error(error);
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                DialogHelper::showDialog(error);
            });
        });
}

PlayerSingleComment::~PlayerSingleComment() {
    brls::Logger::debug("Fragment PlayerSingleComment: delete");
}
brls::View* PlayerSingleComment::getDefaultFocus() {
    return this->recyclingGrid;
}

/// PlayerCommentAction

PlayerCommentAction::PlayerCommentAction() {
    this->inflateFromXMLRes("xml/fragment/player_comment_action.xml");

    this->svgLike->registerAction(
        "", brls::ControllerButton::BUTTON_NAV_RIGHT,
        [this](...) {
            this->dismiss();
            return true;
        },
        true);

    this->svgLike->registerClickAction([this](...) {
        this->dismiss();
        this->likeClickEvent.fire();
        return true;
    });
    this->svgReply->registerClickAction([this](...) {
        this->dismiss();
        this->replyClickEvent.fire();
        return true;
    });
    this->svgDelete->registerClickAction([this](...) {
        this->dismiss();
        this->deleteClickEvent.fire();
        return true;
    });

    this->svgLike->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->svgLike));
    this->svgReply->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->svgReply));
    this->svgDelete->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->svgDelete));

    this->closeBtn->registerClickAction([this](...) {
        this->dismiss();
        return true;
    });
}

void PlayerCommentAction::setActionData(
    const bilibili::VideoCommentResult& data, float y) {
    this->comment->setPositionTop(y - 8);
    this->comment->setData(data);

    if (data.rpid != data.root) this->comment->hideReplyIcon(true);

    if (y < 20) y = 20;
    if (y > 620) y = 620;
    this->actionBox->setPositionTop(y);
}

void PlayerCommentAction::showDelete() {
    this->svgDelete->setVisibility(brls::Visibility::VISIBLE);
}

brls::View* PlayerCommentAction::getDefaultFocus() { return this->svgLike; }

void PlayerCommentAction::dismiss(std::function<void(void)> cb) {
    brls::Application::popActivity(brls::TransitionAnimation::NONE);
}
