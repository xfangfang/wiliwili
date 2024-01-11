//
// Created by 贾海峰 on 2022/8/20.
//

#include <utility>
#include <borealis/core/thread.hpp>

#include "fragment/search_hots.hpp"
#include "view/recycling_grid.hpp"
#include "view/hots_card.hpp"
#include "bilibili.h"
#include "bilibili/result/search_result.h"

SearchHots::SearchHots() {
    this->inflateFromXMLRes("xml/fragment/search_hots.xml");
    brls::Logger::debug("Fragment SearchHots: create");
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemHotsCard::create(); });

    this->registerFloatXMLAttribute("spanCount", [this](float value) { this->recyclingGrid->spanCount = (int)value; });

    try {
        this->requestSearch();
    } catch (brls::ViewNotFoundException const &exception) {
        brls::Logger::error("ViewNotFoundException: {}", exception.what());
    }
}

brls::View *SearchHots::create() { return new SearchHots(); }

SearchHots::~SearchHots() {
    brls::Logger::debug("Fragment Hots: delete");
    this->recyclingGrid->clearData();
}

class HotsDataSource : public RecyclingGridDataSource {
public:
    HotsDataSource(bilibili::SearchHotsListResult result, UpdateSearchEvent *u)
        : list(std::move(result)), updateSearchEvent(u) {}

    RecyclingGridItem *cellForRow(RecyclingGrid *recycler, size_t index) override {
        RecyclingGridItemHotsCard *item = (RecyclingGridItemHotsCard *)recycler->dequeueReusableCell("Cell");
        bilibili::SearchHotsResult &r   = this->list[index];
        item->setCard(std::to_string(index + 1), r.show_name, r.icon);
        return item;
    }

    void onItemSelected(RecyclingGrid *recycler, size_t index) override {
        if (this->updateSearchEvent) {
            this->updateSearchEvent->fire(list[index].keyword);
        }
    }

    size_t getItemCount() override { return list.size(); }

    void clearData() override { this->list.clear(); }

private:
    bilibili::SearchHotsListResult list;
    UpdateSearchEvent *updateSearchEvent = nullptr;
};

void SearchHots::requestSearch() {
    ASYNC_RETAIN
    BILI::get_search_hots(
        50,
        [ASYNC_TOKEN](const bilibili::SearchHotsResultWrapper &result) {
            brls::Threading::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                auto ds = new HotsDataSource(result.list, this->updateSearchEvent);
                recyclingGrid->setDataSource(ds);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::Logger::error("SearchHots: {}", error);
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                this->recyclingGrid->setError(error);
            });
        });
}

void SearchHots::setSearchCallback(UpdateSearchEvent *event) { this->updateSearchEvent = event; }

RecyclingGrid *SearchHots::getRecyclingGrid() { return this->recyclingGrid; }
