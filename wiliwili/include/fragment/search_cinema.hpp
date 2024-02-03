//
// Created by fang on 2022/8/3.
//

// register this fragment in main.cpp
//#include "fragment/search_cinema.hpp"
//    brls::Application::registerXMLView("SearchCinema", SearchCinema::create);
// <brls:View xml=@res/xml/fragment/search_cinema.xml

#pragma once

#include <atomic>
#include <memory>

#include "fragment/search_interface.hpp"

class RecyclingGrid;

class SearchCinema : public SearchAttachedView {
public:
    SearchCinema();

    ~SearchCinema() override;

    static brls::View* create();

    void requestSearch(const std::string& key) override;

    void _requestSearch(const std::string& key);

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "search/cinema/recyclingGrid");

    unsigned int requestIndex = 1;
};