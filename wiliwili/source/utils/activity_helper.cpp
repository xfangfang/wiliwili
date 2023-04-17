//
// Created by fang on 2023/4/17.
//

#include <borealis/core/application.hpp>
#include "activity/player_activity.hpp"
#include "activity/live_player_activity.hpp"
#include "activity/hint_activity.hpp"
#include "activity/setting_activity.hpp"
#include "activity/search_activity.hpp"
#include "activity/pgc_index_activity.hpp"
#include "activity/main_activity.hpp"
#include "utils/activity_helper.hpp"

#include "presenter/video_detail.hpp"

void Intent::openBV(const std::string& bvid, int cid, int progress) {
    brls::Application::pushActivity(new PlayerActivity(bvid, cid, progress));
}

void Intent::openSeasonBySeasonId(int seasonId, int progress) {
    brls::Application::pushActivity(
        new PlayerSeasonActivity(seasonId, PGC_ID_TYPE::SEASON_ID, progress));
}

void Intent::openSeasonByEpId(int epId, int progress) {
    brls::Application::pushActivity(
        new PlayerSeasonActivity(epId, PGC_ID_TYPE::EP_ID, progress));
}

void Intent::openLive(int live, const std::string& name,
                      const std::string& views) {
    brls::Application::pushActivity(new LiveActivity(live, name, views));
}

void Intent::openSearch(const std::string& key) {
    brls::Application::pushActivity(new SearchActivity(key));
}

void Intent::openPgcFilter(const std::string& filter) {
    brls::Application::pushActivity(new PGCIndexActivity(filter));
}

void Intent::openSetting() {
    brls::Application::pushActivity(new SettingActivity());
}

void Intent::openHint() { brls::Application::pushActivity(new HintActivity()); }

void Intent::openMain() { brls::Application::pushActivity(new MainActivity()); }