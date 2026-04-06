//
// Created by copilot on 2026/4/6.
//

#pragma once

#include <string>
#include <borealis/core/activity.hpp>
#include <borealis/core/bind.hpp>
#include <borealis/views/label.hpp>

#include "view/video_view.hpp"

class LocalPlayerActivity : public brls::Activity {
public:
    CONTENT_FROM_XML_RES("activity/local_player_activity.xml");

    explicit LocalPlayerActivity(const std::string& taskId);
    ~LocalPlayerActivity() override;

    void onContentAvailable() override;

private:
    std::string taskId;

    BRLS_BIND(VideoView, video, "video");
    BRLS_BIND(brls::Label, titleLabel, "local/title");
    BRLS_BIND(brls::Label, qualityLabel, "local/quality");
};
