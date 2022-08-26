//
// Created by fang on 2022/7/28.
//

#include "fragment/mine_history.hpp"
#include "view/video_card.hpp"
#include "activity/player_activity.hpp"
#include "utils/number_helper.hpp"


class DataSourceMineHistoryVideoList
        : public RecyclingGridDataSource
{
public:
    DataSourceMineHistoryVideoList(bilibili::HistoryVideoListResult result):list(result){

    }
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index){
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemHistoryVideoCard* item = (RecyclingGridItemHistoryVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::HistoryVideoResult& r = this->list[index];
        auto badge = r.badge;
        if(badge.empty() && r.progress == -1){
            badge = "已看完";
        }
        auto author = r.author_name;
        if(author.empty()){
            author = r.show_title;
        }
        auto time = wiliwili::sec2date(r.view_at);
        auto duration = wiliwili::sec2Time(r.duration);
        if(r.progress > 0){
            duration = wiliwili::sec2Time(r.progress) + " / " + duration;
        }
        item->setCard(r.cover+"@672w_378h_1c.jpg", r.title, author, time, duration, badge);

        return item;
    }

    size_t getItemCount() {
        return list.size();
    }

    void onItemSelected(RecyclingGrid* recycler, size_t index) {
        std::string& bvid = list[index].history.bvid;
        if(!bvid.empty()){
            brls::Application::pushActivity(new PlayerActivity(bvid));
        } else if(list[index].history.epid != 0) {
            // todo 通过epid获取番剧信息
//            brls::Application::pushActivity(new PlayerSeasonActivity(list[index].history.oid));
        }
    }

    void appendData(const bilibili::HistoryVideoListResult& data){
        brls::Logger::error("DataSourceMineHistoryVideoList: append data");
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

private:
    bilibili::HistoryVideoListResult list;
};


MineHistory::MineHistory() {
    this->inflateFromXMLRes("xml/fragment/mine_history.xml");
    brls::Logger::debug("Fragment MineHistory: create");

    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemHistoryVideoCard::create(); });
    recyclingGrid->onNextPage([this](){
        this->requestData();
    });
    this->requestData();
}

MineHistory::~MineHistory() {
    brls::Logger::debug("Fragment MineHistoryActivity: delete");
}

brls::View *MineHistory::create() {
    return new MineHistory();
}

void MineHistory::onCreate() {
    this->registerTabAction("刷新历史记录", brls::ControllerButton::BUTTON_X, [this](brls::View* view)-> bool {
        AutoTabFrame::focus2Sidebar(this);
        this->recyclingGrid->showSkeleton();
        this->requestData(true);
        return true;
    });
}

void MineHistory::onHistoryList(const bilibili::HistoryVideoResultWrapper &result){
    for(auto i: result.list){
        brls::Logger::debug("history: {}: {}", i.title, i.progress);
    }

    int view_at = this->cursor.view_at;
    brls::Threading::sync([this, result, view_at]() {
        DataSourceMineHistoryVideoList* datasource = (DataSourceMineHistoryVideoList *)recyclingGrid->getDataSource();
        if(datasource && view_at != 0){
            datasource->appendData(result.list);
            recyclingGrid->notifyDataChanged();
        } else{
            recyclingGrid->setDataSource(new DataSourceMineHistoryVideoList(result.list));
        }
    });

}

void MineHistory::onError(const std::string& error) {
    brls::Logger::error("MineHistory::onError: {}", error);
    brls::sync([this, error](){
       this->recyclingGrid->setError(error);
    });
}