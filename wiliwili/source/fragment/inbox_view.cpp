#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/core/thread.hpp>

#include "fragment/inbox_view.hpp"
#include "fragment/inbox_chat.hpp"
#include "view/button_close.hpp"
#include "view/recycling_grid.hpp"
#include "view/auto_tab_frame.hpp"
#include "view/user_info.hpp"
#include "utils/image_helper.hpp"
#include "utils/number_helper.hpp"

using namespace brls::literals;

class ChatUserCard : public RecyclingGridItem {
public:
    ChatUserCard() { this->inflateFromXMLRes("xml/views/inbox_user.xml"); }

    void setCard(const bilibili::InboxChatResult& r) {
        std::string misc;
        if (r.last_msg.msg_type == 1) {
            misc = r.last_msg.content.at("content");
        } else if (r.last_msg.msg_type == 2) {
            misc = "[图片]";
        } else if (r.last_msg.msg_type == 7) {
            misc = "[分享] ";
            misc.append(r.last_msg.content.at("title"));
        } else if (r.last_msg.msg_type == 14) {
            misc = "[分享] ";
            misc.append(r.last_msg.content.at("desc"));
        } else if (r.last_msg.content.contains("title")) {
            misc = r.last_msg.content.at("title");
        }
        this->talker->setUserInfo(r.account_info.pic_url + ImageHelper::face_ext, r.account_info.name, misc);
        if (r.last_msg.timestamp > 0) {
            this->time->setText(wiliwili::sec2date(r.last_msg.timestamp));
        }
        if (r.unread_count > 0) {
            this->badge->setVisibility(brls::Visibility::VISIBLE);
        } else {
            this->badge->setVisibility(brls::Visibility::INVISIBLE);
        }
    }

    void prepareForReuse() override { this->talker->getAvatar()->setImageFromRes("pictures/default_avatar.png"); }

    void cacheForReuse() override { ImageHelper::clear(this->talker->getAvatar()); }

private:
    BRLS_BIND(UserInfoView, talker, "chat/talker");
    BRLS_BIND(brls::Label, time, "inbox/lastTime");
    BRLS_BIND(brls::Rectangle, badge, "badge");
};

class DataSourceChatList : public RecyclingGridDataSource {
public:
    explicit DataSourceChatList(bilibili::InboxChatListResult result) : list(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        auto* item = (ChatUserCard*)recycler->dequeueReusableCell("Cell");
        auto& r    = this->list[index];
        item->setCard(r);
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        auto& r    = this->list[index];
        auto* view = new InboxChat(r, [recycler]() {
            recycler->refresh();
        });
        recycler->present(view);
        view->setWidth(600);
    }

    void appendData(const bilibili::InboxChatListResult& data) {}

    void clearData() override { this->list.clear(); }

private:
    bilibili::InboxChatListResult list;
};

InboxView::InboxView() {
    this->inflateFromXMLRes("xml/fragment/inbox_view.xml");
    brls::Logger::debug("Fragment InboxView: create");

    this->registerAction("hints/cancel"_i18n, brls::BUTTON_B, [](...) {
        brls::Application::popActivity();
        return true;
    });

    this->cancel->registerClickAction([](...) {
        brls::Application::popActivity();
        return true;
    });
    this->cancel->addGestureRecognizer(new brls::TapGestureRecognizer(this->cancel));

    closebtn->registerClickAction([](...) {
        brls::Application::popActivity();
        return true;
    });

    this->registerAction(
        "上一项", brls::ControllerButton::BUTTON_LT,
        [this](brls::View* view) -> bool {
            tabFrame->focus2LastTab();
            return true;
        },
        true);
    this->registerAction(
        "上一项", brls::ControllerButton::BUTTON_LB,
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
    this->registerAction(
        "下一项", brls::ControllerButton::BUTTON_RB,
        [this](brls::View* view) -> bool {
            tabFrame->focus2NextTab();
            return true;
        },
        true);

    recyclingGrid->registerCell("Cell", []() { return new ChatUserCard(); });
    recyclingGrid->setRefreshAction([this]() {
        brls::Application::giveFocus(tabFrame->getSidebar());
        this->recyclingGrid->showSkeleton();
        this->requestData(true);
    });
    recyclingGrid->registerAction("wiliwili/home/common/refresh"_i18n, brls::BUTTON_X, [this](brls::View* view) {
        this->recyclingGrid->refresh();
        return true;
    });

    this->requestData(true);
}

InboxView::~InboxView() { brls::Logger::debug("Fragment InboxView: delete"); }

bool InboxView::isTranslucent() { return true; }

brls::View* InboxView::getDefaultFocus() { return this->tabFrame->getDefaultFocus(); }

void InboxView::onChatList(const bilibili::InboxChatListResult& result, bool refresh) {
    auto* datasource = dynamic_cast<DataSourceChatList*>(recyclingGrid->getDataSource());
    if (datasource && !refresh) {
        if (!result.empty()) {
            datasource->appendData(result);
            recyclingGrid->notifyDataChanged();
        }
    } else {
        auto dataSource = new DataSourceChatList(result);
        recyclingGrid->setDataSource(dataSource);
    }
}

void InboxView::onError(const std::string& error) { this->recyclingGrid->setError(error); }