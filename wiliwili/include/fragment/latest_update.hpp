//
// Created by fang on 2022/11/19.
//

// register this fragment in main.cpp
//#include "fragment/latest_update.hpp"
//    brls::Application::registerXMLView("LatestUpdate", LatestUpdate::create);
// <brls:View xml=@res/xml/fragment/latest_update.xml

#pragma once

#include <nlohmann/json.hpp>
#include <borealis/core/box.hpp>

#include "view/user_info.hpp"

class ReleaseAuthor {
public:
    std::string login;
    std::string avatar_url;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReleaseAuthor, login, avatar_url);

class ReleaseReaction {
public:
    int plus_one;
    int laugh;
    int hooray;
    int heart;
    int rocket;
    int eyes;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, ReleaseReaction& nlohmann_json_t) {
    if (nlohmann_json_j.contains("+1")) {
        nlohmann_json_j.at("+1").get_to(nlohmann_json_t.plus_one);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, laugh, hooray, heart, rocket, eyes));
}

class ReleaseNote {
public:
    std::string tag_name;
    std::string name;
    std::string body;
    std::string published_at;
    ReleaseAuthor author{};
    ReleaseReaction reactions{};
};
inline void from_json(const nlohmann::json& nlohmann_json_j, ReleaseNote& nlohmann_json_t) {
    if (nlohmann_json_j.contains("reactions")) {
        nlohmann_json_j.at("reactions").get_to(nlohmann_json_t.reactions);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, tag_name, name, body, published_at, author));
}

class LatestUpdate : public brls::Box {
public:
    explicit LatestUpdate(const ReleaseNote& info);

    ~LatestUpdate() override;

private:
    BRLS_BIND(brls::Label, subtitle, "latest/subtitle");
    BRLS_BIND(brls::Box, textbox, "latest/textbox");
    BRLS_BIND(brls::Box, topBox, "latest/top");
    BRLS_BIND(brls::Label, header, "latest/header");
    BRLS_BIND(UserInfoView, author, "latest/author");
};