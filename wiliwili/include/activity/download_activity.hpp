//
// Created by copilot on 2026/4/6.
//

#pragma once

#include <borealis/core/activity.hpp>
#include <borealis/core/bind.hpp>
#include <borealis/core/box.hpp>
#include <borealis/views/label.hpp>
#include <borealis/views/image.hpp>
#include <borealis/views/cells/cell_radio.hpp>

#include "view/auto_tab_frame.hpp"
#include "view/recycling_grid.hpp"
#include "utils/download_manager.hpp"

class DownloadCardCell : public RecyclingGridItem {
public:
    DownloadCardCell();

    void setData(const DownloadTask& task,
                 std::function<void(const std::string&)> btn1Action,
                 std::function<void(const std::string&)> btn2Action);

    static RecyclingGridItem* create() { return new DownloadCardCell(); }

    BRLS_BIND(brls::Image, cover, "download/cell/cover");
    BRLS_BIND(brls::Label, titleLabel, "download/cell/title");
    BRLS_BIND(brls::Label, qualityLabel, "download/cell/quality");
    BRLS_BIND(brls::Label, progressLabel, "download/cell/progress");
    BRLS_BIND(brls::RadioCell, btn1, "download/cell/btn1");
    BRLS_BIND(brls::RadioCell, btn2, "download/cell/btn2");

private:
    std::string taskId;
};

class DownloadActivity : public brls::Activity {
public:
    CONTENT_FROM_XML_RES("activity/download_activity.xml");

    DownloadActivity();
    ~DownloadActivity() override;

    void onContentAvailable() override;

private:
    void refreshLists();

    BRLS_BIND(AutoTabFrame, tabFrame, "download/tabFrame");
    BRLS_BIND(RecyclingGrid, activeList, "download/active/list");
    BRLS_BIND(RecyclingGrid, completedList, "download/completed/list");

    brls::Event<std::string>::Subscription progressSub;
    brls::Event<std::string>::Subscription statusSub;
};
