//
// Created by fang on 2022/7/28.
//

#include <utility>
#include <borealis/views/dialog.hpp>
#include <borealis/core/thread.hpp>

#include "fragment/mine_history.hpp"
#include "view/video_card.hpp"
#include "utils/number_helper.hpp"
#include "utils/activity_helper.hpp"
#include "utils/image_helper.hpp"

using namespace brls::literals;

class DataSourceMineHistoryVideoList : public RecyclingGridDataSource {
public:
    explicit DataSourceMineHistoryVideoList(bilibili::HistoryVideoListResult result) : list(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemHistoryVideoCard* item =
            (RecyclingGridItemHistoryVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::HistoryVideoResult& r = this->list[index];
        auto badge                      = r.badge;
        if (badge.empty() && r.progress == -1) {
            badge = "wiliwili/mine/done"_i18n;
        }
        auto author     = r.author_name;
        bool showUpName = true;
        if (author.empty()) {
            showUpName = false;
            author     = r.show_title;
        } else if (!r.show_title.empty()) {
            author += " - " + r.show_title;
        }
        auto time = wiliwili::sec2TimeDate(r.view_at);

        std::string duration;
        float progress = -1;
        if (r.duration >= 0 && r.progress >= 0 && (r.history.business == "pgc" || r.history.business == "archive")) {
            duration = wiliwili::sec2Time(r.progress) + "/" + wiliwili::sec2Time(r.duration);
            progress = r.progress * 1.0 / r.duration;
        } else if (r.progress < 0) {
            progress = 1.0;
        }

        item->setCard(r.cover + ImageHelper::h_ext, r.title, author, time, duration, badge, r.history.dt, progress,
                      showUpName);

        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        auto& data = list[index].history;

        std::string& bvid     = data.bvid;
        std::string& business = data.business;
        if (business == "archive") {
            Intent::openBV(bvid, data.cid, -1);
        } else if (business == "pgc") {
            Intent::openSeasonByEpId(data.epid, -1);
        } else if (business == "live") {
            if (list[index].live_status) {
                Intent::openLive(data.oid, list[index].title, "-");
            }
        } else if (business == "article" || business == "article-list") {
            auto cvid = data.oid;
            if (data.cid != 0) cvid = data.cid;
            auto url = fmt::format("https://www.bilibili.com/read/cv{}", cvid);

            auto container = new brls::Box(brls::Axis::COLUMN);
            container->setJustifyContent(brls::JustifyContent::CENTER);
            container->setAlignItems(brls::AlignItems::CENTER);
            auto l1 = new brls::Label();
            l1->setFontSize(24);
            l1->setText("wiliwili/mine/article"_i18n);
            auto l2 = new brls::Label();
            l2->setMarginTop(10);
            l2->setTextColor(brls::Application::getTheme().getColor("color/link"));
            l2->setText(url);
            container->setHeight(200);
            container->addView(l1);
            container->addView(l2);
            auto dialog = new brls::Dialog(container);
            dialog->addButton("hints/ok"_i18n, [url]() { brls::Application::getPlatform()->openBrowser(url); });
            dialog->open();
        }
    }

    void appendData(const bilibili::HistoryVideoListResult& data) {
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

    void clearData() override { this->list.clear(); }

private:
    bilibili::HistoryVideoListResult list;
};

MineHistory::MineHistory() {
    this->inflateFromXMLRes("xml/fragment/mine_history.xml");
    brls::Logger::debug("Fragment MineHistory: create");

    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemHistoryVideoCard::create(); });
    recyclingGrid->onNextPage([this]() { this->requestData(); });
    recyclingGrid->setRefreshAction([this]() {
        AutoTabFrame::focus2Sidebar(this);
        this->recyclingGrid->showSkeleton();
        this->requestData(true);
    });
    this->requestData();
}

MineHistory::~MineHistory() { brls::Logger::debug("Fragment MineHistoryActivity: delete"); }

brls::View* MineHistory::create() { return new MineHistory(); }

void MineHistory::onCreate() {
    this->registerTabAction("wiliwili/mine/refresh_history"_i18n, brls::ControllerButton::BUTTON_X,
                            [this](brls::View* view) -> bool {
                                this->recyclingGrid->refresh();
                                return true;
                            });
}

void MineHistory::onHistoryList(const bilibili::HistoryVideoResultWrapper& result) {
    for (auto i : result.list) {
        brls::Logger::verbose("history: {}: {}", i.title, i.progress);
    }

    int view_at = this->cursor.view_at;
    brls::Threading::sync([this, result, view_at]() {
        auto* datasource = dynamic_cast<DataSourceMineHistoryVideoList*>(recyclingGrid->getDataSource());
        if (datasource && view_at != 0) {
            if (!result.list.empty()) {
                datasource->appendData(result.list);
                recyclingGrid->notifyDataChanged();
            }
        } else {
            recyclingGrid->setDataSource(new DataSourceMineHistoryVideoList(result.list));
        }
    });
}

void MineHistory::onError(const std::string& error) {
    brls::Logger::error("MineHistory::onError: {}", error);
    brls::sync([this, error]() { this->recyclingGrid->setError(error); });
}