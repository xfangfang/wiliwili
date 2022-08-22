//
// Created by fang on 2022/6/9.
//

#include "fragment/dynamic_tab.hpp"
#include "fragment/dynamic_video.hpp"
#include "view/auto_tab_frame.hpp"
#include "view/recycling_grid.hpp"
#include "view/svg_image.hpp"
#include "utils/image_helper.hpp"


class DynamicUserInfoView : public RecyclingGridItem {

public:
    DynamicUserInfoView(std::string xml){
        this->inflateFromXMLRes(xml);
    }

    void setUserInfo(std::string avatar, std::string username, bool isUpdate = false){
        this->labelUsername->setText(username);
        ImageHelper::with(this)->load(avatar)->into(this->avatarView);
    }

    brls::Image* getAvatar(){
        return this->avatarView;
    }

    void prepareForReuse(){

    }

    void cacheForReuse(){
        if(!dynamic_cast<SVGImage*>(this->avatarView.getView()))
            ImageHelper::clear(this->avatarView);
    }

    static RecyclingGridItem* create(std::string xml = "xml/views/user_info_dynamic.xml"){
        return new DynamicUserInfoView(xml);
    }

private:
    BRLS_BIND(brls::Image, avatarView, "avatar");
    BRLS_BIND(brls::Label, labelUsername, "username");
};

class DataSourceUpList
        : public RecyclingGridDataSource
{
public:
    DataSourceUpList(bilibili::DynamicUpListResult result):list(result){

    }
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index){
        if(index == 0){
            DynamicUserInfoView* item = (DynamicUserInfoView*)recycler->dequeueReusableCell("CellAll");
            return item;
        }

        //从缓存列表中取出 或者 新生成一个表单项
        DynamicUserInfoView* item = (DynamicUserInfoView*)recycler->dequeueReusableCell("Cell");

        auto& r = this->list[index - 1];
        item->setUserInfo(r.user_profile.info.face+"@96w_96h_1c.jpg", r.user_profile.info.uname);
        return item;
    }

    size_t getItemCount() {
        return list.size() + 1;
    }

    void onItemSelected(RecyclingGrid* recycler, size_t index) {
        if(index == 0){
            // 选择全部
            userSelectedEvent.fire(0);
        } else {
            // 选择具体的某个up主
            userSelectedEvent.fire(list[index - 1].user_profile.info.uid);
        }

    }

    void appendData(const bilibili::DynamicUpListResult& data){
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

    UserSelectedEvent* getSelectedEvent(){
        return &this->userSelectedEvent;
    }

private:
    bilibili::DynamicUpListResult list;
    UserSelectedEvent userSelectedEvent;
};

DynamicTab::DynamicTab() {
    this->inflateFromXMLRes("xml/fragment/dynamic_tab.xml");
    brls::Logger::debug("Fragment DynamicTab: create");

    recyclingGrid->registerCell("Cell", []() { return DynamicUserInfoView::create(); });
    recyclingGrid->registerCell("CellAll", []() { return DynamicUserInfoView::create("xml/views/user_info_dynamic_all.xml"); });
    recyclingGrid->setDataSource(new DataSourceUpList(bilibili::DynamicUpListResult()));
    this->requestData();
}

DynamicTab::~DynamicTab() {
    brls::Logger::debug("Fragment DynamicTabActivity: delete");
}

brls::View* DynamicTab::create() {
    return new DynamicTab();
}

void DynamicTab::onUpList(const bilibili::DynamicUpListResultWrapper &result){
    brls::Threading::sync([this, result]() {
        auto dataSource = new DataSourceUpList(result.items);
        recyclingGrid->setDataSource(dataSource);
        dataSource->getSelectedEvent()->subscribe([this](uint mid){
            dynamicVideoTab->changeUser(mid);
        });
    });
}

void DynamicTab::onError(const string& error){
    brls::Logger::error("DynamicTab::onError {}", error);
}

void DynamicTab::onCreate(){
    this->registerTabAction("刷新列表", brls::ControllerButton::BUTTON_X, [this](brls::View* view)-> bool {
        this->requestData();
        dynamicVideoTab->changeUser(0);
        return true;
    });
}
