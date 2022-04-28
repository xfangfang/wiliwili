#pragma once

#include <borealis.hpp>
#include <thread>
#include <chrono>

#include "grid_list.hpp"

template<class DataType>
class GridFrame : public brls::AbsoluteLayout
{
    public:
        GridFrame(int columns, int itemWidth, int itemHeight, int spacing, std::function<GridListItem*(DataType)> createItem, bool more = false){

            this->pn = 0;
            this->ps = 19;
            // Create views
            list = new GridList<DataType>(columns, itemWidth, itemHeight, spacing, createItem);
            
            if(more) {
                brls::Logger::error("add more action");
                list->addMoreAction([this](){
                    this->getNextData();
                });
            }

            this->addView(list);
        }

        brls::View* getDefaultFocus() override {
            return this->list->getDefaultFocus();
        }

        void layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash){
            int x = this->getX();
            int y = this->getY();
            int w = this->getWidth();
            int h = this->getHeight();
            list->setBoundaries(x, y, w, h);
        }

        void loadData(std::vector<DataType> dataList){
            if (dataList.size() == 0){ // Fix: when network error dataList.size() is also 0
                brls::Application::notify("no more");
                this->pn--;
            } else {
                this->list->addListData(dataList);
            }
        }

        void setRequestCallback(std::function<void(int, int)> requestCallback){
            this->requestCallback = requestCallback;
        }

        void setNumPerPage(size_t num){
            this->ps = num;
        }

        void setCount(size_t count){
            this->count = count;
        }

        void getNextData(){
            this->pn++;
            if(this->requestCallback) 
                this->requestCallback(pn, ps);
        }

        brls::Event<DataType, GridListItem*>* getClickEvent(){
            return this->list->getClickEvent();
        }


    private:
        std::function<void(int, int)> requestCallback;
        GridList<DataType>* list;
        std::vector<DataType> data;
        size_t pn;
        size_t ps;
        size_t count;
        bool have_more;
};