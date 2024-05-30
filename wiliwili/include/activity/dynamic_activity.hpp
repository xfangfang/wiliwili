//
// Created by fang on 2024/5/30.
//

#pragma once

#include <borealis/core/activity.hpp>
#include <borealis/views/applet_frame.hpp>
#include <string>
#include <utility>

#include "view/dynamic_article.hpp"

class DynamicActivity : public brls::Activity {
public:
    explicit DynamicActivity(std::string id) : id(std::move(id)) {}

    brls::View* createContentView() override {
        auto* detail    = new DynamicArticleDetail(id);
        auto* container = new brls::AppletFrame(detail);
        container->setHeaderVisibility(brls::Visibility::GONE);
        container->setFooterVisibility(brls::AppletFrame::HIDE_BOTTOM_BAR ? brls::Visibility::GONE
                                                                          : brls::Visibility::VISIBLE);
        return container;
    }

private:
    std::string id;
};