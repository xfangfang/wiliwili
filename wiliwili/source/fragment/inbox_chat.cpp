#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/core/thread.hpp>
#include <algorithm>

#include "fragment/inbox_chat.hpp"
#include "view/inbox_msg_card.hpp"
#include "view/custom_button.hpp"
#include "utils/number_helper.hpp"

using namespace brls::literals;

class InboxNoticeCard : public RecyclingGridItem {
public:
    InboxNoticeCard() {
        auto theme = brls::Application::getTheme();
        this->setFocusable(false);
        this->setJustifyContent(brls::JustifyContent::CENTER);
        noticeLabel = new brls::Label();
        this->noticeLabel->setFontSize(16);
        this->noticeLabel->setTextColor(theme.getColor("font/grey"));
        this->addView(noticeLabel);
    }

    void setCard(const bilibili::InboxMessageResult& r) {
        std::string msg = r.content.at("content");
        auto result     = nlohmann::json::parse(msg);
        this->noticeLabel->setText(result[0]["text"]);
    }

private:
    brls::Label* noticeLabel;
};

class DataSourceMsgList : public RecyclingGridDataSource {
public:
    explicit DataSourceMsgList(const bilibili::InboxMessageResultWrapper& result, uint64_t mid)
        : list(std::move(result.messages)), talkerId(mid) {
        std::sort(this->list.begin(), this->list.end(),
                  [](const bilibili::InboxMessageResult& x, const bilibili::InboxMessageResult& y) {
                      return x.msg_seqno < y.msg_seqno;
                  });

        for (auto& e : result.e_infos) {
            this->emotes.insert({e.text, std::make_shared<bilibili::InboxEmote>(e)});
        }
    }
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        auto& r = this->list[index];

        if (r.msg_type == 18) {
            auto* item = (InboxNoticeCard*)recycler->dequeueReusableCell("Notice");
            item->setCard(r);
            return item;
        }

        auto* item = (InboxMsgCard*)recycler->dequeueReusableCell("Cell");
        item->setCard(r, this->emotes, this->talkerId);

        item->setTimeVisible(index == 0 || this->list[index - 1].timestamp + 300 < r.timestamp);
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {}

    bool appendData(const bilibili::InboxMessageResultWrapper& result) {
        bool skip_all = true;
        for (const auto& i : result.messages) {
            bool skip = false;
            for (const auto& j : this->list) {
                if (j.msg_seqno == i.msg_seqno) {
                    skip = true;
                    break;
                }
            }
            if (!skip) {
                this->list.push_back(i);
                skip_all = false;
            }
        }

        for (auto& e : result.e_infos) {
            this->emotes[e.text] = std::make_shared<bilibili::InboxEmote>(e);
        }
        return skip_all;
    }

    void clearData() override { this->list.clear(); }

private:
    bilibili::InboxMessageListResult list;
    IEMap emotes;
    uint64_t talkerId;
};

InboxChat::InboxChat(const bilibili::InboxChatResult& r, std::function<void()> cb) {
    this->inflateFromXMLRes("xml/fragment/inbox_chat.xml");
    brls::Logger::debug("Fragment InboxChat: create");

    this->registerAction("hints/cancel"_i18n, brls::BUTTON_B, [this, cb](...) {
        this->dismiss(cb);
        return true;
    });

    this->setTalkerId(r.talker_id);
    this->setMsgSeq(r.ack_seqno);

    recyclingGrid->registerCell("Cell", [r]() {
        auto* card = new InboxMsgCard();
        card->setAvatar(r.account_info.pic_url);
        return card;
    });
    recyclingGrid->registerCell("Notice", []() { return new InboxNoticeCard(); });
    recyclingGrid->onNextPage([this, r]() { this->requestData(false, r.session_type); });
    recyclingGrid->setRefreshAction([this]() { this->recyclingGrid->forceRequestNextPage(); });
    // 避免聊天框和底部刷新按钮重叠
    recyclingGrid->setPaddingBottom(80);

    this->registerAction("wiliwili/home/common/refresh"_i18n, brls::BUTTON_X, [this](brls::View* view) {
        this->recyclingGrid->refresh();
        return true;
    });

    labelTalker->setText(r.account_info.name);

    if (r.system_msg_type > 0) {
        inputReply->setVisibility(brls::Visibility::GONE);
    } else {
        inputReply->registerClickAction([this](...) { return this->toggleSend(); });
        inputReply->addGestureRecognizer(new brls::TapGestureRecognizer(this->inputReply));

        this->registerAction("", brls::BUTTON_Y, [this](brls::View* view) { return this->toggleSend(); }, true);
    }

    this->requestData(true, r.session_type);
}

InboxChat::~InboxChat() { brls::Logger::debug("Fragment InboxChat: delete"); }

bool InboxChat::toggleSend() {
    return brls::Application::getImeManager()->openForText(
        [this](const std::string& text) {
            if (text.empty()) return;
            this->sendMsg(text);
        },
        "wiliwili/inbox/chat/hint"_i18n, "", 500, "", 0);
}

void InboxChat::onError(const std::string& error) {
    this->recyclingGrid->setError(error);
}

void InboxChat::onMsgList(const bilibili::InboxMessageResultWrapper& result, bool refresh) {
    brls::Threading::sync([this, result, refresh]() {
        auto* datasource = dynamic_cast<DataSourceMsgList*>(recyclingGrid->getDataSource());
        if (datasource && !refresh) {
            if (!result.messages.empty()) {
                if(!datasource->appendData(result)){
                    // 只在有内容更新时才调整位置
                    recyclingGrid->notifyDataChanged();
                    recyclingGrid->selectRowAt(datasource->getItemCount() - 1, true);
                }
            }
        } else {
            datasource = new DataSourceMsgList(result, this->talkerId);
            recyclingGrid->setDefaultCellFocus(datasource->getItemCount() - 1);
            recyclingGrid->setDataSource(datasource);
            recyclingGrid->selectRowAt(recyclingGrid->getDefaultCellFocus(), true);
        }
    });
}

void InboxChat::onSendMsg(const bilibili::InboxSendResult& result) {
    this->recyclingGrid->forceRequestNextPage();
}