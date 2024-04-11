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
            auto j = nlohmann::json::parse(r.last_msg.content);
            misc   = j.at("content").get<std::string>();
        } else if (r.last_msg.msg_type == 2) {
            misc = "Image";
        }
        this->talker->setUserInfo(r.account_info.pic_url + ImageHelper::face_ext, r.account_info.name, misc);
        if (r.last_msg.timestamp > 0) {
            this->time->setText(wiliwili::sec2date(r.last_msg.timestamp));
        }
    }

private:
    BRLS_BIND(UserInfoView, talker, "chat/talker");
    BRLS_BIND(brls::Label, time, "inbox/lastTime");
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
        auto* view = new InboxChat(r.talker_id, r.session_type);
        recycler->present(view);
        brls::sync([view]() { brls::Application::giveFocus(view); });
    }

    void appendData(const bilibili::InboxChatListResult& data) {}

    void clearData() override { this->list.clear(); }

private:
    bilibili::InboxChatListResult list;
};

InboxView::InboxView() {
    this->inflateFromXMLRes("xml/fragment/inbox_view.xml");
    brls::Logger::debug("Fragment InboxView: create");

    this->registerAction("hints/cancel"_i18n, brls::BUTTON_X, [](...) {
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

    recyclingGrid->registerCell("Cell", []() { return new ChatUserCard(); });
    recyclingGrid->onNextPage([this]() { this->requestData(false); });

    this->requestData(true);
}

InboxView::~InboxView() { brls::Logger::debug("Fragment InboxView: delete"); }

bool InboxView::isTranslucent() { return true; }

brls::View* InboxView::getDefaultFocus() { return this->inboxFrame->getDefaultFocus(); }

void InboxView::onChatList(const bilibili::InboxChatListResult& result, bool refresh) {
    brls::Threading::sync([this, result, refresh]() {
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
    });
}

void InboxView::onError(const std::string& error) {
    brls::Threading::sync([this, error]() { this->recyclingGrid->setError(error); });
}