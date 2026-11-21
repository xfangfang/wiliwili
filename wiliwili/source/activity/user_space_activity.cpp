//
// Created by fang on 2024/12/21.
//

#include <borealis/core/thread.hpp>
#include <borealis/core/application.hpp>
#include <borealis/views/dialog.hpp>

#include "activity/user_space_activity.hpp"
#include "utils/activity_helper.hpp"
#include "utils/dialog_helper.hpp"
#include "utils/image_helper.hpp"
#include "utils/number_helper.hpp"
#include "view/video_card.hpp"
#include "view/svg_image.hpp"

using namespace brls::literals;

class DataSourceUserSpaceVideoList : public RecyclingGridDataSource {
public:
    DataSourceUserSpaceVideoList(bilibili::UserUploadedVideoListResult result, uint64_t mid)
        : list(std::move(result)), userMid(mid) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        RecyclingGridItemVideoCard* item = (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::UserUploadedVideoResult& r = this->list[index];
        item->setCard(r.pic + ImageHelper::h_ext, r.title, r.author + " · " + wiliwili::sec2TimeDate(r.created),
                      r.play == -1 ? "-" : wiliwili::num2w(r.play), wiliwili::num2w(r.video_review), r.length);
        item->setCharging(r.is_charging_arc);
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        Intent::openBV(list[index].bvid);
    }

    void appendData(const bilibili::UserUploadedVideoListResult& data) {
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

    void clearData() override { this->list.clear(); }

private:
    bilibili::UserUploadedVideoListResult list;
    uint64_t userMid;
};

UserSpaceActivity::UserSpaceActivity(uint64_t mid) : user_mid(mid) {}

void UserSpaceActivity::onContentAvailable() {
    brls::Logger::debug("UserSpaceActivity::onContentAvailable mid: {}", user_mid);

    // Bind UI elements
    this->appletFrame = (brls::AppletFrame*)this->getView("user_space/applet_frame");
    this->headerBox = (brls::Box*)this->getView("user_space/header");
    this->userAvatar = (brls::Image*)this->getView("user_space/avatar");
    this->userName = (brls::Label*)this->getView("user_space/name");
    this->userSign = (brls::Label*)this->getView("user_space/sign");
    this->userFollowing = (brls::Label*)this->getView("user_space/following");
    this->userFollower = (brls::Label*)this->getView("user_space/follower");
    this->btnFollowBox = (brls::Box*)this->getView("user_space/follow_box");
    this->btnFollowLabel = (brls::Label*)this->getView("user_space/follow_label");
    this->btnFollowIcon = (SVGImage*)this->getView("user_space/follow_icon");
    this->tabFrame = (AutoTabFrame*)this->getView("user_space/tab_frame");
    this->recyclingGrid = (RecyclingGrid*)this->getView("user_space/recycling_grid");

    // Request user info
    this->requestUpInfo(user_mid);

    // Set up recycling grid
    this->recyclingGrid->estimatedRowHeight = 200;
    this->recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemVideoCard::create(); });

    // Set up pagination
    this->recyclingGrid->onNextPage([this]() {
        if (this->hasMoreUploads) {
            this->uploadPage++;
            this->requestUploadedVideos(this->user_mid, this->uploadPage);
        }
    });

    // Request uploaded videos
    this->requestUploadedVideos(this->user_mid, 1);

    // Setup follow button
    this->btnFollowBox->registerClickAction([this](...) {
        if (!DialogHelper::checkLogin()) return true;

        if (this->isFollowing) {
            auto dialog = new brls::Dialog("wiliwili/player/not_follow"_i18n);
            dialog->addButton("hints/cancel"_i18n, []() {});
            dialog->addButton("hints/ok"_i18n, [this]() {
                this->followUp(std::to_string(this->user_mid), false);
            });
            dialog->open();
        } else {
            this->followUp(std::to_string(this->user_mid), true);
        }
        return true;
    });
}

void UserSpaceActivity::onUpInfo(const bilibili::UserDetailResultWrapper& result) {
    brls::Logger::debug("UserSpaceActivity::onUpInfo name: {}", result.card.name);

    brls::sync([this, result]() {
        // Set user avatar
        ImageHelper::with(this->userAvatar)->load(result.card.face + ImageHelper::face_ext);

        // Set user name
        this->userName->setText(result.card.name);

        // Set user signature
        this->userSign->setText(result.card.sign.empty() ? "wiliwili/user_space/no_sign"_i18n : result.card.sign);

        // Set follower and following counts
        this->userFollowing->setText(wiliwili::num2w(result.following));
        this->userFollower->setText(wiliwili::num2w(result.follower));

        // Set follow button state
        this->isFollowing = result.following;
        auto theme = brls::Application::getTheme();
        if (this->isFollowing) {
            this->btnFollowBox->setBackgroundColor(theme.getColor("color/grey_1"));
            this->btnFollowLabel->setText("wiliwili/user_space/following"_i18n);
            this->btnFollowLabel->setTextColor(theme.getColor("font/grey"));
            this->btnFollowIcon->setImageFromSVGRes("svg/bpx-svg-sprite-sort.svg");
        } else {
            this->btnFollowBox->setBackgroundColor(theme.getColor("color/bilibili"));
            this->btnFollowLabel->setText("wiliwili/user_space/follow"_i18n);
            this->btnFollowLabel->setTextColor(theme.getColor("color/white"));
            this->btnFollowIcon->setImageFromSVGRes("svg/bpx-svg-sprite-add.svg");
        }
    });
}

void UserSpaceActivity::onUploadedVideos(const bilibili::UserUploadedVideoResultWrapper& result) {
    brls::Logger::debug("UserSpaceActivity::onUploadedVideos count: {} page: {}", result.list.vlist.size(),
                        result.page.pn);

    brls::sync([this, result]() {
        auto* datasource = dynamic_cast<DataSourceUserSpaceVideoList*>(this->recyclingGrid->getDataSource());

        if (datasource && result.page.pn > 1) {
            // Append data for pagination
            datasource->appendData(result.list.vlist);
            this->recyclingGrid->notifyDataChanged();
        } else {
            // First page - set new data source
            this->recyclingGrid->setDataSource(new DataSourceUserSpaceVideoList(result.list.vlist, this->user_mid));
        }

        // Check if there are more pages
        this->hasMoreUploads = result.page.pn * result.page.ps < result.page.count;
    });
}

void UserSpaceActivity::onError(const std::string& error) {
    brls::Logger::error("UserSpaceActivity::onError: {}", error);
    brls::sync([this, error]() {
        this->recyclingGrid->setError(error);
    });
}

brls::View* UserSpaceActivity::createContentView() {
    return brls::View::createFromXMLResource("xml/activity/user_space_activity.xml");
}
