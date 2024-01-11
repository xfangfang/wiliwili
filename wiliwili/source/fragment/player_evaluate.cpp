//
// Created by fang on 2023/1/16.
//

#include <borealis/views/label.hpp>
#include <borealis/views/image.hpp>

#include "fragment/player_evaluate.hpp"

PlayerEvaluate::PlayerEvaluate() { this->inflateFromXMLRes("xml/fragment/player_evaluate.xml"); }

void PlayerEvaluate::setContent(const std::string& value) {
    if (value.empty()) {
        empty->setVisibility(brls::Visibility::VISIBLE);
    } else {
        this->label->setText(value);
    }
}

PlayerEvaluate::~PlayerEvaluate() { brls::Logger::debug("Fragment PlayerEvaluate: delete"); }

brls::View* PlayerEvaluate::create() { return new PlayerEvaluate(); }