//
// Created by fang on 2023/4/17.
//

#include <borealis/core/application.hpp>
#include "activity/player_activity.hpp"
#include "activity/live_player_activity.hpp"
#include "activity/hint_activity.hpp"
#include "activity/setting_activity.hpp"
#include "activity/search_activity.hpp"
#include "activity/search_activity_tv.hpp"
#include "activity/pgc_index_activity.hpp"
#include "activity/main_activity.hpp"
#include "activity/gallery_activity.hpp"
#include "activity/dlna_activity.hpp"
#include "activity/dynamic_activity.hpp"
#include "fragment/mine_collection_video_list.hpp"
#include "fragment/inbox_view.hpp"
#include "utils/activity_helper.hpp"
#include "utils/config_helper.hpp"

#include "presenter/video_detail.hpp"

void Intent::openBV(const std::string& bvid, uint64_t cid, int progress) {
    auto activity = new PlayerActivity(bvid, cid, progress);
    brls::Application::pushActivity(activity, brls::TransitionAnimation::NONE);
    registerFullscreen(activity);
}

void Intent::openSeasonBySeasonId(uint64_t seasonId, int progress) {
    auto activity = new PlayerSeasonActivity(seasonId, PGC_ID_TYPE::SEASON_ID, progress);
    brls::Application::pushActivity(activity, brls::TransitionAnimation::NONE);
    registerFullscreen(activity);
}

void Intent::openSeasonByEpId(uint64_t epId, int progress) {
    auto activity = new PlayerSeasonActivity(epId, PGC_ID_TYPE::EP_ID, progress);
    brls::Application::pushActivity(activity, brls::TransitionAnimation::NONE);
    registerFullscreen(activity);
}

void Intent::openLive(int live, const std::string& name, const std::string& views) {
    auto activity = new LiveActivity(live, name, views);
    brls::Application::pushActivity(activity, brls::TransitionAnimation::NONE);
    registerFullscreen(activity);
}

void Intent::openCollection(const std::string& mid, const std::string& type) {
    auto collection = new MineCollectionVideoList();
    collection->applyXMLAttribute("type", type);
    collection->applyXMLAttribute("collection", mid);
    auto activity = new brls::Activity(collection);
    brls::Application::pushActivity(activity, brls::TransitionAnimation::NONE);
    registerFullscreen(activity);
}

void Intent::openSearch(const std::string& key) {
    if (key.empty()) return;
    auto activity = new SearchActivity(key);
    brls::Application::pushActivity(activity, brls::TransitionAnimation::NONE);
    registerFullscreen(activity);
}

void Intent::openTVSearch() {
    auto activity = new TVSearchActivity();
    brls::Application::pushActivity(activity, brls::TransitionAnimation::NONE);
    registerFullscreen(activity);
}

void Intent::openPgcFilter(const std::string& filter) {
    auto activity = new PGCIndexActivity(filter);
    brls::Application::pushActivity(activity, brls::TransitionAnimation::NONE);
    registerFullscreen(activity);
}

void Intent::openSetting() {
    auto activity = new SettingActivity();
    brls::Application::pushActivity(activity);
    registerFullscreen(activity);
}

void Intent::openInbox() {
    auto inbox = new InboxView();
    brls::Application::pushActivity(new brls::Activity(inbox));
}

void Intent::openHint() { brls::Application::pushActivity(new HintActivity()); }

void Intent::openMain() {
    auto activity = new MainActivity();
    brls::Application::pushActivity(activity);
    registerFullscreen(activity);
}

void Intent::openGallery(const std::vector<std::string>& data) {
    auto activity = new GalleryActivity(data);
    brls::Application::pushActivity(activity, brls::TransitionAnimation::NONE);
    registerFullscreen(activity);
}

void Intent::openDLNA() {
    auto activity = new DLNAActivity();
    brls::Application::pushActivity(activity, brls::TransitionAnimation::NONE);
    registerFullscreen(activity);
}

void Intent::openActivity(const std::string& id) {
    auto activity = new DynamicActivity(id);
    brls::Application::pushActivity(activity, brls::TransitionAnimation::NONE);
    registerFullscreen(activity);
}

void Intent::_registerFullscreen(brls::Activity* activity) {
    (void)activity;
}