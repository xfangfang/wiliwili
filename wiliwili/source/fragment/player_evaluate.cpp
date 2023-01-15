//
// Created by fang on 2023/1/16.
//

#include "fragment/player_evaluate.hpp"

PlayerEvaluate::PlayerEvaluate() {
    this->inflateFromXMLRes("xml/fragment/player_evaluate.xml");
}

void PlayerEvaluate::setContent(const std::string& value) {
    this->label->setText(value);
}

PlayerEvaluate::~PlayerEvaluate() {
    brls::Logger::debug("Fragment PlayerEvaluate: delete");
}

brls::View* PlayerEvaluate::create() { return new PlayerEvaluate(); }