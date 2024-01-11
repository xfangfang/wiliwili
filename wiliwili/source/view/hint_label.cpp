//
// Created by fang on 2023/8/6.
//

#include <unordered_map>
#include <borealis/core/application.hpp>

#include "view/hint_label.hpp"

static std::unordered_map<std::string, std::string> key_map_origin{
    {"a", "\uE0E0"},
    {"b", "\uE0E1"},
    {"x", "\uE0E2"},
    {"y", "\uE0E3"},
};
static std::unordered_map<std::string, std::string> key_map_swap{
    {"a", "\uE0E1"},
    {"b", "\uE0E0"},
    {"x", "\uE0E3"},
    {"y", "\uE0E2"},
};

HintLabel::HintLabel() {
    brls::Logger::debug("View HintLabel: create");

    this->registerStringXMLAttribute("text", [this](const std::string& type) {
        if (key_map_origin.count(type) == 0) brls::fatal("unknown hint type for HintLabel: " + type);

        this->setText(brls::Application::isSwapInputKeys() ? key_map_swap[type] : key_map_origin[type]);
    });
}

HintLabel::~HintLabel() { brls::Logger::debug("View HintLabel: delete"); }

brls::View* HintLabel::create() { return new HintLabel(); }