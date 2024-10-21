//
// Created by fang on 2022/7/14.
//

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
#include "fragment/mine_later.hpp"
#include "fragment/mine_collection.hpp"
#include "fragment/mine_collection_video_list.hpp"
#include "fragment/mine_bangumi.hpp"
#include "fragment/inbox_feed.hpp"
#include "fragment/search_tab.hpp"
#include "fragment/search_order.hpp"
#include "fragment/search_video.hpp"
#include "fragment/search_cinema.hpp"
#include "fragment/search_bangumi.hpp"
#include "fragment/search_hots.hpp"
#include "fragment/search_history.hpp"
#include "fragment/share_dialog.hpp"

#include "utils/config_helper.hpp"

#include "view/auto_tab_frame.hpp"
#include "view/video_view.hpp"
#include "view/user_info.hpp"
#include "view/text_box.hpp"
#include "view/qr_image.hpp"
#include "view/svg_image.hpp"
#include "view/up_user_small.hpp"
#include "view/recycling_grid.hpp"
#include "view/grid_dropdown.hpp"
#include "view/video_comment.hpp"
#include "view/video_progress_slider.hpp"
#include "view/gallery_view.hpp"
#include "view/custom_button.hpp"
#include "view/animation_image.hpp"
#include "view/button_close.hpp"
#include "view/check_box.hpp"
#include "view/video_profile.hpp"
#include "view/selector_cell.hpp"
#include "view/hint_label.hpp"
#include "view/mpv_core.hpp"
#include "view/dynamic_video_card.hpp"
#include "view/dynamic_article.hpp"

void Register::initCustomView() {
    // Register extended views
    brls::Application::registerXMLView("AutoTabFrame", AutoTabFrame::create);
    brls::Application::registerXMLView("RecyclingGrid", RecyclingGrid::create);
    brls::Application::registerXMLView("VideoView", VideoView::create);
    brls::Application::registerXMLView("VideoProfile", VideoProfile::create);
    brls::Application::registerXMLView("QRImage", QRImage::create);
    brls::Application::registerXMLView("SVGImage", SVGImage::create);
    brls::Application::registerXMLView("TextBox", TextBox::create);
    brls::Application::registerXMLView("VideoProgressSlider", VideoProgressSlider::create);
    brls::Application::registerXMLView("GalleryView", GalleryView::create);
    brls::Application::registerXMLView("CustomButton", CustomButton::create);
    brls::Application::registerXMLView("HintLabel", HintLabel::create);

    //     Register custom views
    brls::Application::registerXMLView("UserInfoView", UserInfoView::create);
    brls::Application::registerXMLView("UpUserSmall", UpUserSmall::create);
    brls::Application::registerXMLView("VideoComment", VideoComment::create);
    brls::Application::registerXMLView("ButtonClose", ButtonClose::create);
    brls::Application::registerXMLView("CheckBox", BiliCheckBox::create);
    brls::Application::registerXMLView("SelectorCell", BiliSelectorCell::create);
    brls::Application::registerXMLView("AnimationImage", AnimationImage::create);
    brls::Application::registerXMLView("ShareBox", ShareBox::create);
    brls::Application::registerXMLView("DynamicVideoCardView", DynamicVideoCardView::create);
    brls::Application::registerXMLView("DynamicArticleView", DynamicArticleView::create);

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
    brls::Application::registerXMLView("MineLater", MineLater::create);
    brls::Application::registerXMLView("MineCollection", MineCollection::create);
    brls::Application::registerXMLView("MineCollectionVideoList", MineCollectionVideoList::create);
    brls::Application::registerXMLView("MineBangumi", MineBangumi::create);
    brls::Application::registerXMLView("InboxFeed", InboxFeed::create);
    brls::Application::registerXMLView("SearchTab", SearchTab::create);
    brls::Application::registerXMLView("SearchOrder", SearchOrder::create);
    brls::Application::registerXMLView("SearchVideo", SearchVideo::create);
    brls::Application::registerXMLView("SearchCinema", SearchCinema::create);
    brls::Application::registerXMLView("SearchBangumi", SearchBangumi::create);
    brls::Application::registerXMLView("SearchHots", SearchHots::create);
    brls::Application::registerXMLView("SearchHistory", SearchHistory::create);
}

void Register::initCustomTheme() {
    // Add custom values to the theme
    // 用于左侧边栏背景
    brls::Theme::getLightTheme().addColor("color/grey_1", nvgRGB(245, 246, 247));
    brls::Theme::getDarkTheme().addColor("color/grey_1", nvgRGB(51, 52, 53));

    // 用于二级切换分栏的背景色（例：直播切换分区、每周必看切换周）
    brls::Theme::getLightTheme().addColor("color/grey_2", nvgRGB(245, 245, 245));
    brls::Theme::getDarkTheme().addColor("color/grey_2", nvgRGB(51, 53, 55));

    // 用于骨架屏背景色
    brls::Theme::getLightTheme().addColor("color/grey_3", nvgRGBA(200, 200, 200, 16));
    brls::Theme::getDarkTheme().addColor("color/grey_3", nvgRGBA(160, 160, 160, 160));

    // 用于历史记录右上角Badge的半透明背景
    brls::Theme::getLightTheme().addColor("color/grey_4", nvgRGBA(48, 48, 48, 160));
    brls::Theme::getDarkTheme().addColor("color/grey_4", nvgRGBA(48, 48, 48, 160));

    // 深浅配色通用的灰色字体颜色
    brls::Theme::getLightTheme().addColor("font/grey", nvgRGB(148, 153, 160));
    brls::Theme::getDarkTheme().addColor("font/grey", nvgRGB(148, 153, 160));

    // 入站必刷推荐原因背景色
    brls::Theme::getLightTheme().addColor("color/yellow_1", nvgRGB(253, 246, 230));
    brls::Theme::getDarkTheme().addColor("color/yellow_1", nvgRGB(46, 33, 17));

    // 入站必刷推荐原因字体颜色
    brls::Theme::getLightTheme().addColor("font/yellow_1", nvgRGB(217, 118, 7));
    brls::Theme::getDarkTheme().addColor("font/yellow_1", nvgRGB(217, 118, 7));

    // 粉色文字，bilibili主题色
    brls::Theme::getLightTheme().addColor("color/bilibili", nvgRGB(255, 102, 153));
    brls::Theme::getDarkTheme().addColor("color/bilibili", nvgRGB(255, 102, 153));

    // 蓝色文字，用于链接文字颜色
    brls::Theme::getLightTheme().addColor("color/link", nvgRGB(102, 147, 182));
    brls::Theme::getDarkTheme().addColor("color/link", nvgRGB(102, 147, 182));

    // 分割线颜色
    brls::Theme::getLightTheme().addColor("color/line", nvgRGB(208, 208, 208));
    brls::Theme::getDarkTheme().addColor("color/line", nvgRGB(100, 100, 100));

    // 粉色背景，用于扁平TabBar背景色
    brls::Theme::getLightTheme().addColor("color/pink_1", nvgRGB(252, 237, 241));
    brls::Theme::getDarkTheme().addColor("color/pink_1", nvgRGB(44, 27, 34));

    // 红色，用于提示小红点
    brls::Theme::getLightTheme().addColor("color/tip/red", nvgRGB(250, 88, 87));
    brls::Theme::getDarkTheme().addColor("color/tip/red", nvgRGB(211, 63, 64));

    brls::Theme::getLightTheme().addColor("color/white", nvgRGB(255, 255, 255));
    brls::Theme::getDarkTheme().addColor("color/white", nvgRGBA(255, 255, 255, 180));

    brls::Theme::getLightTheme().addColor("captioned_image/caption", nvgRGB(2, 176, 183));
    brls::Theme::getDarkTheme().addColor("captioned_image/caption", nvgRGB(51, 186, 227));
}

void Register::initCustomStyle() {
    // Add custom values to the style
    brls::getStyle().addMetric("brls/animations/highlight", 200);

    if (brls::Application::ORIGINAL_WINDOW_HEIGHT == 544) {
        brls::getStyle().addMetric("wiliwili/grid/span/5", 4);
        brls::getStyle().addMetric("wiliwili/grid/span/4", 3);
        brls::getStyle().addMetric("wiliwili/grid/span/3", 2);
        brls::getStyle().addMetric("wiliwili/grid/span/2", 2);
        brls::getStyle().addMetric("wiliwili/player/width", 600);
        brls::getStyle().addMetric("wiliwili/player/bottom/font", 10);
        brls::getStyle().addMetric("wiliwili/comment/level/x", 30);
        brls::getStyle().addMetric("wiliwili/margin/20", 10);
        brls::getStyle().addMetric("wiliwili/about/qr", 150);
        brls::getStyle().addMetric("wiliwili/about/speech/width", 420);
        brls::getStyle().addMetric("wiliwili/about/speech/header", 495);
        brls::getStyle().addMetric("wiliwili/tab_frame/content_padding_top_bottom", 20);
        brls::getStyle().addMetric("wiliwili/mine/num", 18);
        brls::getStyle().addMetric("wiliwili/mine/type", 12);
        brls::getStyle().addMetric("wiliwili/setting/about/bottom", 0);
        brls::getStyle().addMetric("wiliwili/dynamic/video/card/padding", 10);
        brls::getStyle().addMetric("brls/tab_frame/content_padding_sides", 30);
    } else {
        switch (brls::Application::ORIGINAL_WINDOW_HEIGHT) {
            case 1080:
                brls::getStyle().addMetric("wiliwili/grid/span/5", 7);
                brls::getStyle().addMetric("wiliwili/grid/span/4", 6);
                brls::getStyle().addMetric("wiliwili/grid/span/3", 5);
                brls::getStyle().addMetric("wiliwili/grid/span/2", 4);
                break;
            case 900:
                brls::getStyle().addMetric("wiliwili/grid/span/5", 6);
                brls::getStyle().addMetric("wiliwili/grid/span/4", 5);
                brls::getStyle().addMetric("wiliwili/grid/span/3", 4);
                brls::getStyle().addMetric("wiliwili/grid/span/2", 3);
                break;
            case 720:
            default:
                brls::getStyle().addMetric("wiliwili/grid/span/5", 5);
                brls::getStyle().addMetric("wiliwili/grid/span/4", 4);
                brls::getStyle().addMetric("wiliwili/grid/span/3", 3);
                brls::getStyle().addMetric("wiliwili/grid/span/2", 2);
        }
        brls::getStyle().addMetric("wiliwili/player/width", brls::Application::ORIGINAL_WINDOW_WIDTH - 480);
        brls::getStyle().addMetric("wiliwili/player/bottom/font", 13);
        brls::getStyle().addMetric("wiliwili/comment/level/x", 45);
        brls::getStyle().addMetric("wiliwili/margin/20", 20);
        brls::getStyle().addMetric("wiliwili/about/qr", 200);
        brls::getStyle().addMetric("wiliwili/about/speech/width", 660);
        brls::getStyle().addMetric("wiliwili/about/speech/header", 760);
        brls::getStyle().addMetric("wiliwili/tab_frame/content_padding_top_bottom", 42);
        brls::getStyle().addMetric("wiliwili/mine/num", 24);
        brls::getStyle().addMetric("wiliwili/mine/type", 16);
        brls::getStyle().addMetric("wiliwili/setting/about/bottom", 50);
        brls::getStyle().addMetric("wiliwili/dynamic/video/card/padding", 20);
    }
}
