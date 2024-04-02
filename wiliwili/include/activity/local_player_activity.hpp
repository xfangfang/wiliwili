#pragma once

#include <borealis/core/activity.hpp>
#include <borealis/core/bind.hpp>
#include <utility>

#include "utils/event_helper.hpp"
#include "presenter/live_data.hpp"

#include <cpr/filesystem.h>

class VideoView;

class LocalPlayerActivity : public brls::Activity {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/video_activity.xml");

    explicit LocalPlayerActivity(cpr::fs::path fp) : filepath(std::move(fp)) {}

    void onContentAvailable() override;

    ~LocalPlayerActivity() override;
private:
    const cpr::fs::path filepath;

    BRLS_BIND(VideoView, video, "video");
};