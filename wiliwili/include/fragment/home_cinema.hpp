//
// Created by fang on 2022/7/14.
//

// register this fragment in main.cpp
//#include "fragment/home_cinema.hpp"
//    brls::Application::registerXMLView("HomeCinema", HomeCinema::create);
// <brls:View xml=@res/xml/fragment/home_cinema.xml

#pragma once

#include "presenter/home_pgc.hpp"
#include "view/auto_tab_frame.hpp"

class HomeCinema : public AttachedView, public HomeCinemaRequest {
public:
    HomeCinema();

    void onCreate() override;

    void onCinemaList(const bilibili::PGCResultWrapper &result) override;

    ~HomeCinema();

    static View *create();

    void onError(const std::string &error) override;

private:
    BRLS_BIND(AutoTabFrame, tabFrame, "homeCinema/tabFrame");
};