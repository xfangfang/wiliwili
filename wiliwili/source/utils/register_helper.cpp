//
// Created by fang on 2022/7/14.
//

#include "view/auto_tab_frame.hpp"
#include "view/video_view.hpp"
#include "view/user_info.hpp"
#include "view/text_box.hpp"
#include "view/qr_image.hpp"
#include "view/svg_image.hpp"
#include "view/up_user_small.hpp"
#include "view/recycling_grid.hpp"
#include "view/grid_dropdown.hpp"
//#include "view/raw_video_view.hpp"
#include "view/video_comment.hpp"
#include "view/mpv_core.hpp"
#include "view/video_progress_slider.hpp"

#include "fragment/home_tab.hpp"
#include "fragment/dynamic_tab.hpp"
#include "fragment/mine_tab.hpp"
#include "fragment/home_recommends.hpp"
#include "fragment/home_hots_all.hpp"
#include "fragment/home_hots_history.hpp"
#include "fragment/home_hots_weekly.hpp"
#include "fragment/home_hots_rank.hpp"
#include "fragment/home_hots.hpp"
#include "fragment/home_live.hpp"
#include "fragment/home_bangumi.hpp"
#include "fragment/home_cinema.hpp"
#include "fragment/mine_history.hpp"
#include "fragment/mine_collection.hpp"
#include "fragment/search_tab.hpp"
#include "fragment/search_video.hpp"
#include "fragment/search_cinema.hpp"
#include "fragment/search_bangumi.hpp"

#include "utils/config_helper.hpp"

void Register::initCustomView(){
    // Register extended views
    brls::Application::registerXMLView("AutoTabFrame", AutoTabFrame::create);
    brls::Application::registerXMLView("RecyclingGrid", RecyclingGrid::create);
    brls::Application::registerXMLView("VideoView", VideoView::create);
    brls::Application::registerXMLView("QRImage", QRImage::create);
    brls::Application::registerXMLView("SVGImage", SVGImage::create);
    brls::Application::registerXMLView("TextBox", TextBox::create);
    brls::Application::registerXMLView("VideoProgressSlider", VideoProgressSlider::create);

//     Register custom views
    brls::Application::registerXMLView("UserInfoView", UserInfoView::create);
    brls::Application::registerXMLView("UpUserSmall", UpUserSmall::create);
    brls::Application::registerXMLView("VideoComment", VideoComment::create);

//     Register fragments
    brls::Application::registerXMLView("HomeTab", HomeTab::create);
    brls::Application::registerXMLView("DynamicTab", DynamicTab::create);
    brls::Application::registerXMLView("MineTab", MineTab::create);
    brls::Application::registerXMLView("HomeRecommends", HomeRecommends::create);
    brls::Application::registerXMLView("HomeHotsAll", HomeHotsAll::create);
    brls::Application::registerXMLView("HomeHotsHistory", HomeHotsHistory::create);
    brls::Application::registerXMLView("HomeHotsWeekly", HomeHotsWeekly::create);
    brls::Application::registerXMLView("HomeHotsRank", HomeHotsRank::create);
    brls::Application::registerXMLView("HomeHots", HomeHots::create);
    brls::Application::registerXMLView("HomeLive", HomeLive::create);
    brls::Application::registerXMLView("HomeBangumi", HomeBangumi::create);
    brls::Application::registerXMLView("HomeCinema", HomeCinema::create);
    brls::Application::registerXMLView("MineHistory", MineHistory::create);
    brls::Application::registerXMLView("MineCollection", MineCollection::create);
    brls::Application::registerXMLView("SearchTab", SearchTab::create);
    brls::Application::registerXMLView("SearchVideo", SearchVideo::create);
    brls::Application::registerXMLView("SearchCinema", SearchCinema::create);
    brls::Application::registerXMLView("SearchBangumi", SearchBangumi::create);

//    brls::Application::registerXMLView("RawVideoView", RawVideoView::create);
//    MPVCore::instance();
}

void Register::initCustomTheme(){
    // Add custom values to the theme
    // 用于左侧边栏背景
    brls::getLightTheme().addColor("color/grey_1", nvgRGBA(48, 48, 48, 16));
    brls::getDarkTheme().addColor("color/grey_1", nvgRGBA(48, 48, 48, 160));

    // 用于二级切换分栏的背景色（例：直播切换分区、每周必看切换周）
    brls::getLightTheme().addColor("color/grey_2", nvgRGB(245, 245, 245));
    brls::getDarkTheme().addColor("color/grey_2", nvgRGB(51,53,55));

    // 粉色文字，bilibili主题色
    brls::getLightTheme().addColor("color/bilibili", nvgRGB(255, 102, 153));
    brls::getDarkTheme().addColor("color/bilibili", nvgRGB(255, 102, 153));

    // 粉色背景，用于扁平TabBar背景色
    brls::getLightTheme().addColor("color/pink_1", nvgRGB(252, 237, 241));
    brls::getDarkTheme().addColor("color/pink_1", nvgRGB(44,27,34));

    brls::getLightTheme().addColor("captioned_image/caption", nvgRGB(2, 176, 183));
    brls::getDarkTheme().addColor("captioned_image/caption", nvgRGB(51, 186, 227));
}

void Register::initCustomStyle(){
    // Add custom values to the style
    brls::getStyle().addMetric("brls/animations/highlight", 200);
}
