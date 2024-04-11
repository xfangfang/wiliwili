#include <borealis/core/thread.hpp>
#include <algorithm>

#include "fragment/inbox_chat.hpp"
#include "view/recycling_grid.hpp"

#include "view/text_box.hpp"

using namespace brls::literals;

class ChatMsgCard : public RecyclingGridItem {
public:
    ChatMsgCard() { this->inflateFromXMLRes("xml/views/inbox_msg.xml"); }

    void setCard(const bilibili::InboxMessageResult& r) {
        try {
            auto j = nlohmann::json::parse(r.content);
            switch (r.msg_type) {
                case 1:
                    this->textBox->setText(j.at("content").get<std::string>());
                    this->msgBox->setVisibility(brls::Visibility::VISIBLE);
                    this->picBox->setVisibility(brls::Visibility::GONE);
                    break;
                case 2: {
                    std::string pic = j.at("url").get<std::string>();
                    float width     = j.at("width").get<float>();
                    float height    = j.at("height").get<float>();

                    if (width > 400.f) {
                        height = height * 400.f / width;
                        width  = 400.f;
                    }
                    this->picBox->setWidth(width);
                    this->picBox->setHeight(height);
                    ImageHelper::with(this->msgPic)->load(pic);

                    this->picBox->setVisibility(brls::Visibility::VISIBLE);
                    this->msgBox->setVisibility(brls::Visibility::GONE);
                    break;
                }
            }

        } catch (const std::exception& ex) {
        }
    }

    void setTime(const std::string& time_str) {
        if (time_str.empty()) {
            this->labelTime->setVisibility(brls::Visibility::GONE);
        } else {
            this->labelTime->setText(time_str);
            this->labelTime->setVisibility(brls::Visibility::VISIBLE);
        }
    }

    void setTalker(bool talker) {
        auto theme = brls::Application::getTheme();
        if (talker) {
            this->talker->setVisibility(brls::Visibility::VISIBLE);
            this->mine->setVisibility(brls::Visibility::INVISIBLE);
            this->msgBox->setBackgroundColor(theme.getColor("color/grey_2"));
        } else {
            this->talker->setVisibility(brls::Visibility::INVISIBLE);
            this->mine->setVisibility(brls::Visibility::VISIBLE);
            this->msgBox->setBackgroundColor(theme.getColor("color/bilibili"));
        }
    }

private:
    BRLS_BIND(TextBox, textBox, "msg/content");
    BRLS_BIND(brls::Box, msgBox, "msg/content_box");
    BRLS_BIND(brls::Box, picBox, "msg/picture_box");
    BRLS_BIND(brls::Image, msgPic, "msg/picture");
    BRLS_BIND(brls::Image, talker, "avatar/talker");
    BRLS_BIND(brls::Image, mine, "avatar/mine");
    BRLS_BIND(brls::Label, labelTime, "msg/time");
};

class DataSourceMsgList : public RecyclingGridDataSource {
public:
    explicit DataSourceMsgList(const bilibili::InboxMessageListResult& result, uint64_t mid)
        : list(std::move(result)), talkerId(mid) {
        std::reverse(this->list.begin(), this->list.end());
    }
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        auto* item = (ChatMsgCard*)recycler->dequeueReusableCell("Cell");
        auto& r    = this->list[index];

        std::string t = wiliwili::sec2date(r.timestamp);
        item->setTalker(r.sender_uid == this->talkerId);
        item->setCard(r);
        if (this->lastTime == t) {
            item->setTime("");
        } else {
            item->setTime(t);
            this->lastTime = t;
        }
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
    uint64_t talkerId;
    std::string lastTime;
};

InboxChat::InboxChat(uint64_t talker_id, int session_type) {
    this->inflateFromXMLRes("xml/fragment/inbox_chat.xml");
    brls::Logger::debug("Fragment InboxChat: create");

    this->registerAction("hints/cancel"_i18n, brls::BUTTON_B, [](...) {
        brls::Application::popActivity();
        return true;
    });

    this->setTalkerId(talker_id);

    recyclingGrid->registerCell("Cell", []() { return new ChatMsgCard(); });
    recyclingGrid->onNextPage([this, session_type]() { this->requestData(false, session_type); });

    this->requestData(true, session_type);
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
            auto dataSource = new DataSourceMsgList(result.messages, this->talkerId);
            recyclingGrid->setDataSource(dataSource);
        }
    });
}