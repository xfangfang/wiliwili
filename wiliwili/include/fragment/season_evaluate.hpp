//
// Created by fang on 2023/1/16.
//

// register this fragment in main.cpp
//#include "fragment/season_evaluate.hpp"
//    brls::Application::registerXMLView("SeasonEvaluate", SeasonEvaluate::create);
// <brls:View xml=@res/xml/fragment/season_evaluate.xml

#pragma once

#include <borealis.hpp>

class SeasonEvaluate : public brls::Box {
public:
    SeasonEvaluate();

    ~SeasonEvaluate() override;

    void setKeyword(const std::string& value);

    void setContent(const std::string& value);

    static View* create();

private:
    BRLS_BIND(brls::Label, label, "evaluate/content");
    BRLS_BIND(brls::Button, btnDouban, "evaluate/douban");
    BRLS_BIND(brls::Button, btnZhihu, "evaluate/zhihu");
    BRLS_BIND(brls::Button, btnBaidu, "evaluate/baidu");
    BRLS_BIND(brls::Button, btnBing, "evaluate/bing");

    std::string keyword;
};