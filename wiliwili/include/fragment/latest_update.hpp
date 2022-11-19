//
// Created by fang on 2022/11/19.
//

// register this fragment in main.cpp
//#include "fragment/latest_update.hpp"
//    brls::Application::registerXMLView("LatestUpdate", LatestUpdate::create);
// <brls:View xml=@res/xml/fragment/latest_update.xml

#pragma once

#include <borealis.hpp>
#include "view/user_info.hpp"

class LatestUpdate : public brls::Box {
public:
    LatestUpdate(std::string name, std::string content, std::string author_name,
                 std::string author_avatar, std::string publish_date);

    ~LatestUpdate();

private:
    BRLS_BIND(brls::Label, label, "latest/content/label");
    BRLS_BIND(brls::Label, header, "latest/header");
    BRLS_BIND(UserInfoView, author, "latest/author");
};