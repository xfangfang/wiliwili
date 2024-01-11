//
// Created by fang on 2023/4/25.
//

#include "fragment/search_history.hpp"

#include <utility>
#include "view/recycling_grid.hpp"
#include "view/hots_card.hpp"
#include "utils/config_helper.hpp"

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
    HistoryDataSource(std::vector<std::string> result, UpdateSearchEvent **u)
        : list(std::move(result)), updateSearchEvent(u) {}

    RecyclingGridItem *cellForRow(RecyclingGrid *recycler, size_t index) override {
        auto *item = (RecyclingGridItemHotsCard *)recycler->dequeueReusableCell("Cell");
        item->setCard(std::to_string(index + 1), this->list[index], "");
        return item;
    }

    void onItemSelected(RecyclingGrid *recycler, size_t index) override {
        if (this->updateSearchEvent && *this->updateSearchEvent) {
            (*this->updateSearchEvent)->fire(list[index]);
        }
    }

    size_t getItemCount() override { return list.size(); }

    void clearData() override { this->list.clear(); }

private:
    std::vector<std::string> list;
    UpdateSearchEvent **updateSearchEvent = nullptr;
};

void SearchHistory::requestHistory() {
    recyclingGrid->setDataSource(
        new HistoryDataSource(ProgramConfig::instance().getHistoryList(), &this->updateSearchEvent));
}

void SearchHistory::setSearchCallback(UpdateSearchEvent *event) { this->updateSearchEvent = event; }

RecyclingGrid *SearchHistory::getRecyclingGrid() { return this->recyclingGrid; }
