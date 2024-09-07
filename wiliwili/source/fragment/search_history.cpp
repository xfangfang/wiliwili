//
// Created by fang on 2023/4/25.
//

#include "fragment/search_history.hpp"

#include <utility>
#include <borealis/views/dialog.hpp>
#include <borealis/core/thread.hpp>
#include "view/recycling_grid.hpp"
#include "view/hots_card.hpp"
#include "utils/config_helper.hpp"

using namespace brls::literals;

SearchHistory::SearchHistory() {
    this->inflateFromXMLRes("xml/fragment/search_history.xml");
    brls::Logger::debug("Fragment SearchHistory: create");
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemHotsCard::create(); });

    this->registerFloatXMLAttribute("spanCount", [this](float value) { this->recyclingGrid->spanCount = (int)value; });
}

brls::View *SearchHistory::create() { return new SearchHistory(); }

SearchHistory::~SearchHistory() {
    brls::Logger::debug("Fragment SearchHistory: delete");
    this->recyclingGrid->clearData();
}

class HistoryDataSource : public RecyclingGridDataSource {
public:
    HistoryDataSource(std::vector<std::string> result, UpdateSearchEvent *u, brls::Event<> *c)
        : list(std::move(result)), updateSearchEvent(u), clearSearchEvent(c) {}

    RecyclingGridItem *cellForRow(RecyclingGrid *recycler, size_t index) override {
        if (index == list.size()) {
            auto *item = (RecyclingGridItemHotsCard *)recycler->dequeueReusableCell("Cell");
            item->setCard("🤐", "wiliwili/search/history/clear"_i18n, "");
            return item;
        }
        auto *item = (RecyclingGridItemHotsCard *)recycler->dequeueReusableCell("Cell");
        item->setCard(std::to_string(index + 1), this->list[index], "");
        return item;
    }

    void onItemSelected(RecyclingGrid *recycler, size_t index) override {
        if (index == list.size() && this->clearSearchEvent) {
            this->clearSearchEvent->fire();
            return;
        }
        if (this->updateSearchEvent) {
            this->updateSearchEvent->fire(list[index]);
        }
    }

    size_t getItemCount() override {
        if (list.empty()) return 0;
        return list.size() + 1;
    }

    void clearData() override { this->list.clear(); }

private:
    std::vector<std::string> list;
    UpdateSearchEvent *updateSearchEvent = nullptr;
    brls::Event<> *clearSearchEvent      = nullptr;
};

void SearchHistory::requestHistory() {
    recyclingGrid->setDataSource(new HistoryDataSource(ProgramConfig::instance().getHistoryList(),
                                                       this->updateSearchEvent, &this->clearSearchEvent));
}

void SearchHistory::setSearchCallback(UpdateSearchEvent *UpdateEvent) {
    this->updateSearchEvent = UpdateEvent;

    this->clearSearchEvent.subscribe([this]() {
        auto dialog = new brls::Dialog("wiliwili/search/history/clear_hint"_i18n);
        dialog->addButton("hints/cancel"_i18n, []() {});
        dialog->addButton("hints/ok"_i18n, [this]() {
            brls::sync([this]() {
                brls::Application::giveFocus(this->getTabBar());
                ProgramConfig::instance().setHistory({});
                this->requestHistory();
            });
        });
        dialog->open();
    });
}

RecyclingGrid *SearchHistory::getRecyclingGrid() { return this->recyclingGrid; }
