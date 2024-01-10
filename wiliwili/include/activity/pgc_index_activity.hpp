//
// Created by fang on 2022/8/24.
//
// 本页面提供的功能：番剧、影视 按分类进行检索

#pragma once

#include <borealis/core/activity.hpp>
#include <borealis/core/bind.hpp>

#include "presenter/pgc_index.hpp"

typedef brls::Event<UserRequestData> IndexChangeEvent;

class RecyclingGrid;

class PGCIndexActivity : public brls::Activity, PGCIndexRequest {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/pgc_index_activity.xml");

    PGCIndexActivity(const std::string& url);

    void onContentAvailable() override;

    ~PGCIndexActivity();

    void onPGCIndex(const bilibili::PGCIndexResultWrapper& result) override;

    void onPGCFilter(const bilibili::PGCIndexFilters& result) override;

    void onError(const std::string& error) override;

    void parseParam(const std::string& url);

    // 输入键值对与检索列表，返回对应的检索项列表
    // [{order,1}, {area, 2}] -> [最近更新 日本]
    std::vector<std::string> parseData(const UserRequestData& query);

    void updateTitleBox();

    void openIndexActivity();

    void startAnimation(float alpha);

private:
    std::string originParam;
    UserRequestData requestParam;

    BRLS_BIND(brls::Box, titleBox, "pgc/title");
    BRLS_BIND(RecyclingGrid, recyclingGrid, "pgc/recyclingGrid");
    brls::Animatable alpha = 1.0f;
};