#include <borealis/core/thread.hpp>
#include <pystring.h>

#include "fragment/inbox_feed.hpp"
#include "view/recycling_grid.hpp"
#include "view/text_box.hpp"
#include "utils/image_helper.hpp"
#include "utils/activity_helper.hpp"
#include "utils/string_helper.hpp"

using namespace brls::literals;

class FeedCard : public RecyclingGridItem {
public:
    FeedCard() { this->inflateFromXMLRes("xml/views/feed_card.xml"); }

    void setItem(const bilibili::MsgFeedItem& item) {
        this->textBox->setText(item.source_content);
        ImageHelper::with(this->picture)->load(item.image + ImageHelper::v_ext);
    }

    void setAuther(const bilibili::FeedLikeResult& r) {
        std::vector<std::string> users;
        std::string t = brls::getStr("wiliwili/inbox/type/" + r.item.type);
        for (auto& s : r.users) users.push_back(s.nickname);
        ImageHelper::with(this->avatar)->load(r.users.front().avatar + ImageHelper::face_ext);
        this->labelAuthor->setText(pystring::join(",", users));
        this->labelTime->setText(wiliwili::sec2date(r.like_time));
        this->labelMisc->setText(wiliwili::format("wiliwili/inbox/like"_i18n, t));
    }

    void setAuther(const bilibili::FeedAtResult& r) {
        std::string t = brls::getStr("wiliwili/inbox/type/" + r.item.type);
        ImageHelper::with(this->avatar)->load(r.user.avatar + ImageHelper::face_ext);
        this->labelAuthor->setText(r.user.nickname);
        this->labelTime->setText(wiliwili::sec2date(r.at_time));
        this->labelMisc->setText(wiliwili::format("wiliwili/inbox/at"_i18n, t));
    }

    void setAuther(const bilibili::FeedReplyResult& r) {
        std::string t = brls::getStr("wiliwili/inbox/type/" + r.item.type);
        ImageHelper::with(this->avatar)->load(r.user.avatar + ImageHelper::face_ext);
        this->labelAuthor->setText(r.user.nickname);
        this->labelTime->setText(wiliwili::sec2date(r.reply_time));
        this->labelMisc->setText(wiliwili::format("wiliwili/inbox/reply"_i18n, t));
    }

    void prepareForReuse() override {
        this->avatar->setImageFromRes("pictures/default_avatar.png");
        this->picture->setImageFromRes("pictures/video-card-bg.png");
    }

    void cacheForReuse() override {
        ImageHelper::clear(this->avatar);
        ImageHelper::clear(this->picture);
    }

private:
    BRLS_BIND(brls::Image, avatar, "feed/avatar");
    BRLS_BIND(brls::Label, labelAuthor, "feed/label/author");
    BRLS_BIND(brls::Label, labelMisc, "feed/label/misc");
    BRLS_BIND(brls::Label, labelTime, "feed/time");
    BRLS_BIND(TextBox, textBox, "feed/content");
    BRLS_BIND(brls::Image, picture, "feed/card/picture");
};

template <typename T>
class DataSourceFeedList : public RecyclingGridDataSource {
public:
    explicit DataSourceFeedList(const std::vector<T>& result) : list(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        auto* item = (FeedCard*)recycler->dequeueReusableCell("Cell");
        auto& r    = this->list[index];
        item->setItem(r.item);
        item->setAuther(r);
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        auto& r = this->list[index];
        if (r.item.type == "video" || r.item.type == "reply") {
            // 解析BV号
            size_t pos = r.item.uri.find_last_of('/');
            if (pos > 0) Intent::openBV(r.item.uri.substr(pos + 1));
        }
    }

    void clearData() override { this->list.clear(); }

private:
    std::vector<T> list;
};

InboxFeed::InboxFeed() {
    this->inflateFromXMLRes("xml/fragment/inbox_feed.xml");
    brls::Logger::debug("Fragment InboxFeed: create");

    BRLS_REGISTER_ENUM_XML_ATTRIBUTE("mode", MsgFeedMode, this->setMode,
                                     {
                                         {"reply", MsgFeedMode::REPLY},
                                         {"at", MsgFeedMode::AT},
                                         {"like", MsgFeedMode::LIKE},
                                     });

    // 消息列表
    recyclingGrid->registerCell("Cell", []() { return new FeedCard(); });
}

InboxFeed::~InboxFeed() { brls::Logger::debug("Fragment InboxFeed: delete"); }

void InboxFeed::onCreate() { this->requestData(feedMode, true); }

void InboxFeed::setMode(MsgFeedMode mode) { this->feedMode = mode; }

void InboxFeed::onFeedReplyList(const bilibili::FeedReplyResultWrapper& result) {
    brls::Threading::sync([this, result]() {
        auto dataSource = new DataSourceFeedList(result.items);
        recyclingGrid->setDataSource(dataSource);
    });
}

void InboxFeed::onFeedAtList(const bilibili::FeedAtResultWrapper& result) {
    brls::Threading::sync([this, result]() {
        auto dataSource = new DataSourceFeedList(result.items);
        recyclingGrid->setDataSource(dataSource);
    });
}

void InboxFeed::onFeedLikeList(const bilibili::FeedLikeResultWrapper& result) {
    brls::Threading::sync([this, result]() {
        auto dataSource = new DataSourceFeedList(result.total.items);
        recyclingGrid->setDataSource(dataSource);
    });
}

void InboxFeed::onError(const std::string& error) {
    brls::Threading::sync([this, error]() { this->recyclingGrid->setError(error); });
}

brls::View* InboxFeed::create() { return new InboxFeed(); }