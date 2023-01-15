//
// Created by fang on 2023/1/16.
//

// register this fragment in main.cpp
//#include "fragment/player_evaluate.hpp"
//    brls::Application::registerXMLView("PlayerEvaluate", PlayerEvaluate::create);
// <brls:View xml=@res/xml/fragment/player_evaluate.xml

#pragma once

#include <borealis.hpp>

class PlayerEvaluate : public brls::Box {
public:
    PlayerEvaluate();

    ~PlayerEvaluate() override;

    void setContent(const std::string& value);

    static View* create();

private:
    BRLS_BIND(brls::Label, label, "evaluate/content");
};