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
#include "fragment/mine_collection_video_list.hpp"
#include "utils/activity_helper.hpp"

#include "presenter/video_detail.hpp"

void Intent::openBV(const std::string& bvid, int cid, int progress) {
    brls::Application::pushActivity(new PlayerActivity(bvid, cid, progress),
                                    brls::TransitionAnimation::NONE);
}

void Intent::openSeasonBySeasonId(int seasonId, int progress) {
    brls::Application::pushActivity(
        new PlayerSeasonActivity(seasonId, PGC_ID_TYPE::SEASON_ID, progress),
        brls::TransitionAnimation::NONE);
}

void Intent::openSeasonByEpId(int epId, int progress) {
    brls::Application::pushActivity(
        new PlayerSeasonActivity(epId, PGC_ID_TYPE::EP_ID, progress),
        brls::TransitionAnimation::NONE);
}

void Intent::openLive(int live, const std::string& name,
                      const std::string& views) {
    brls::Application::pushActivity(new LiveActivity(live, name, views),
                                    brls::TransitionAnimation::NONE);
}

void Intent::openCollection(const std::string& mid, const std::string& type) {
    auto collection = new MineCollectionVideoList();
    collection->applyXMLAttribute("type", type);
    collection->applyXMLAttribute("collection", mid);
    brls::Application::pushActivity(new brls::Activity(collection),
                                    brls::TransitionAnimation::NONE);
}

void Intent::openSearch(const std::string& key) {
    brls::Application::pushActivity(new SearchActivity(key),
                                    brls::TransitionAnimation::NONE);
}

void Intent::openTVSearch() {
    brls::Application::pushActivity(new TVSearchActivity(),
                                    brls::TransitionAnimation::NONE);
}

void Intent::openPgcFilter(const std::string& filter) {
    brls::Application::pushActivity(new PGCIndexActivity(filter),
                                    brls::TransitionAnimation::NONE);
}

void Intent::openSetting() {
    brls::Application::pushActivity(new SettingActivity());
}

void Intent::openHint() { brls::Application::pushActivity(new HintActivity()); }

void Intent::openMain() { brls::Application::pushActivity(new MainActivity()); }

void Intent::openGallery(const std::vector<std::string>& data) {
    brls::Application::pushActivity(new GalleryActivity(data),
                                    brls::TransitionAnimation::NONE);
}

void Intent::openDLNA() {
    brls::Application::pushActivity(new DLNAActivity(),
                                    brls::TransitionAnimation::NONE);
}