#include <borealis/core/thread.hpp>
#include <algorithm>

#include "fragment/inbox_chat.hpp"
#include "view/inbox_msg_card.hpp"
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
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {}

    void appendData(const bilibili::InboxMessageListResult& data) {
        bool skip = false;
        for (const auto& i : data) {
            skip = false;
            for (const auto& j : this->list) {
                if (j.msg_seqno == i.msg_seqno) {
                    skip = true;
                    break;
                }
            }
            if (!skip) {
                this->list.push_back(i);
            }
        }
    }

    void clearData() override { this->list.clear(); }

private:
    bilibili::InboxMessageListResult list;
    IEMap emotes;
    uint64_t talkerId;
};

InboxChat::InboxChat(const bilibili::InboxChatResult& r) {
    this->inflateFromXMLRes("xml/fragment/inbox_chat.xml");
    brls::Logger::debug("Fragment InboxChat: create");

    this->registerAction("hints/cancel"_i18n, brls::BUTTON_B, [](...) {
        brls::Application::popActivity();
        return true;
    });

    this->setTalkerId(r.talker_id);

    recyclingGrid->registerCell("Cell", [r]() {
        auto* card = new InboxMsgCard();
        card->setAvatar(r.account_info.pic_url);
        return card;
    });
    recyclingGrid->registerCell("Notice", []() { return new InboxNoticeCard(); });
    recyclingGrid->onNextPage([this, r]() { this->requestData(false, r.session_type); });

    labelTalker->setText(r.account_info.name);

    this->requestData(true, r.session_type);
}

InboxChat::~InboxChat() { brls::Logger::debug("Fragment InboxChat: delete"); }

void InboxChat::onError(const std::string& error) {
    brls::Threading::sync([this, error]() { this->recyclingGrid->setError(error); });
}

void InboxChat::onMsgList(const bilibili::InboxMessageResultWrapper& result, bool refresh) {
    brls::Threading::sync([this, result, refresh]() {
        auto* datasource = dynamic_cast<DataSourceMsgList*>(recyclingGrid->getDataSource());
        if (datasource && !refresh) {
            if (!result.messages.empty()) {
                datasource->appendData(result.messages);
                recyclingGrid->notifyDataChanged();
            }
        } else {
            auto dataSource = new DataSourceMsgList(result, this->talkerId);
            recyclingGrid->setDefaultCellFocus(dataSource->getItemCount() - 1);
            recyclingGrid->setDataSource(dataSource);
        }
    });
}