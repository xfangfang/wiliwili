//
// Created by fang on 2022/8/3.
//

// register this fragment in main.cpp
//#include "fragment/search_bangumi.hpp"
//    brls::Application::registerXMLView("SearchBangumi", SearchBangumi::create);
// <brls:View xml=@res/xml/fragment/search_bangumi.xml

#pragma once

#include <atomic>
#include <memory>

#include "view/recycling_grid.hpp"

class SearchBangumi : public brls::Box {
public:
    SearchBangumi();

    ~SearchBangumi() override;

    static View* create();

    void requestSearch(const std::string& key);

    void _requestSearch(const std::string& key);

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "search/bangumi/recyclingGrid");

    unsigned int requestIndex = 1;
};