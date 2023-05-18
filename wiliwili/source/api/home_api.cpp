#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "bilibili.h"
#include "bilibili/util/md5.hpp"
#include "curl/curl.h"
#include "bilibili/util/http.hpp"
#include "bilibili/result/home_pgc_result.h"
#include "bilibili/result/home_live_result.h"
#include "bilibili/result/home_hots_all_result.h"
#include "bilibili/result/home_hots_weekly_result.h"
#include "bilibili/result/home_hots_history_result.h"
#include "bilibili/result/home_hots_rank.h"

namespace bilibili {

/// 主页 推荐
void BilibiliClient::get_recommend(
    int index, int num,
    const std::function<void(RecommendVideoListResultWrapper)>& callback,
    const ErrorCallback& error) {
    //        BilibiliClient::pool.enqueue([=]{
    HTTP::getResultAsync<RecommendVideoListResultWrapper>(
        Api::Recommend,
        {
            {"fresh_idx", std::to_string(index)},
            {"ps", std::to_string(num)},
            {"feed_version", "V1"},
            {"fresh_type", "4"},
            {"plat", "1"},
        },
        [callback, index](RecommendVideoListResultWrapper wrapper) {
            wrapper.requestIndex = index;
            callback(wrapper);
        },
        error);
    //        });
}

/// 主页 热门 热门综合
void BilibiliClient::get_hots_all(
    int index, int num,
    const std::function<void(HotsAllVideoListResult, bool)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<HotsAllVideoListResultWrapper>(
        Api::HotsAll,
        {{"pn", std::to_string(index)}, {"ps", std::to_string(num)}},
        [callback](const HotsAllVideoListResultWrapper& wrapper) {
            callback(wrapper.list, wrapper.no_more);
        },
        error);
}

/// 主页 热门 每周推荐列表
void BilibiliClient::get_hots_weekly_list(
    const std::function<void(HotsWeeklyListResult)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<HotsWeeklyResultWrapper>(
        Api::HotsWeeklyList, {},
        [callback](const HotsWeeklyResultWrapper& wrapper) {
            callback(wrapper.list);
        },
        error);
}

/// 主页 热门 每周推荐
void BilibiliClient::get_hots_weekly(
    int number,
    const std::function<void(HotsWeeklyVideoListResult, std::string,
                             std::string)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<HotsWeeklyVideoListResultWrapper>(
        Api::HotsWeekly, {{"number", std::to_string(number)}},
        [callback](const HotsWeeklyVideoListResultWrapper& wrapper) {
            callback(wrapper.list, wrapper.config.label, wrapper.reminder);
        },
        error);
}

/// 主页 热门 入站必刷
void BilibiliClient::get_hots_history(
    const std::function<void(HotsHistoryVideoListResult, std::string)>&
        callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<HotsHistoryVideoListResultWrapper>(
        Api::HotsHistory, {},
        [callback](const HotsHistoryVideoListResultWrapper& wrapper) {
            callback(wrapper.list, wrapper.explain);
        },
        error);
}

/// 主页 热门 排行榜 投稿视频
void BilibiliClient::get_hots_rank(
    int rid, const std::string& type,
    const std::function<void(HotsRankVideoListResult, std::string)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<HotsRankVideoListResultWrapper>(
        Api::HotsRank, {{"rid", std::to_string(rid)}, {"type", type}},
        [callback](auto wrapper) { callback(wrapper.list, wrapper.note); },
        error);
}

/// 主页 热门 排行榜 官方
void BilibiliClient::get_hots_rank_pgc(
    int season_type, int day,
    const std::function<void(HotsRankPGCVideoListResult, std::string)>&
        callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<HotsRankPGCVideoListResultWrapper>(
        Api::HotsRankPGC,
        {{"season_type", std::to_string(season_type)},
         {"day", std::to_string(day)}},
        [callback](auto wrapper) { callback(wrapper.list, wrapper.note); },
        error);
}

/// 主页 直播推荐
void BilibiliClient::get_live_recommend(
    int parent_area_id, int area_id, int page, const std::string& source,
    const std::function<void(LiveResultWrapper)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<LiveResultWrapper>(
        Api::LiveFeed,
        {
            {"parent_area_id", std::to_string(parent_area_id)},
            {"area_id", std::to_string(area_id)},
            {"device", "switch"},
            {"page", std::to_string(page)},
            {"platform", "web"},
            {"scale", "xxhdpi"},
            {"source_name", source},
            {"mobi_app", "pc_electron"},
        },
        [callback](auto wrapper) { callback(wrapper); }, error, true);
}

/// 主页 追番列表
void BilibiliClient::get_bangumi(
    int is_refresh, const std::string& cursor,
    const std::function<void(PGCResultWrapper)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<PGCResultWrapper>(
        Api::Bangumi,
        {
            {"is_refresh", std::to_string(is_refresh)},
            {"cursor", cursor},
        },
        [callback](auto wrapper) { callback(wrapper); }, error);
}

/// 主页 影视列表
void BilibiliClient::get_cinema(
    int is_refresh, const std::string& cursor,
    const std::function<void(PGCResultWrapper)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<PGCResultWrapper>(
        Api::Cinema,
        {
            {"is_refresh", std::to_string(is_refresh)},
            {"cursor", cursor},
        },
        [callback](auto wrapper) { callback(wrapper); }, error);
}

/// 主页 追番/影视 分类检索
void BilibiliClient::get_pgc_index(
    const std::string& param, int page,
    const std::function<void(PGCIndexResultWrapper)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<PGCIndexResultWrapper>(
        Api::PGCIndex + "?" + param + "&page=" + std::to_string(page), {},
        [callback](auto wrapper) { callback(wrapper); }, error);
}

/// 主页 追番/影视 获取分类
void BilibiliClient::get_pgc_filter(
    const std::string& index_type,
    const std::function<void(PGCIndexFilterWrapper)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<PGCIndexFilterWrapper>(
        Api::PGCIndexFilter,
        {
            {"type", "2"},
            {"index_type", index_type},
        },
        [callback](auto wrapper) { callback(wrapper); }, error);
}

/// 主页 追番/影视 获取全部分类
void BilibiliClient::get_pgc_all_filter(
    const std::function<void(PGCIndexFilters)>& callback,
    const ErrorCallback& error) {
    PGCIndexFilters res;
    BilibiliClient::get_pgc_filter(
        "1",
        [callback, error, res](auto wrapper) mutable {
            wrapper.index_name = "追番";
            res["1"]           = wrapper;
            BilibiliClient::get_pgc_filter(
                "2",
                [callback, error, res](auto wrapper) mutable {
                    wrapper.index_name = "电影";
                    res["2"]           = wrapper;
                    BilibiliClient::get_pgc_filter(
                        "5",
                        [callback, error, res](auto wrapper) mutable {
                            wrapper.index_name = "电视剧";
                            res["5"]           = wrapper;
                            BilibiliClient::get_pgc_filter(
                                "3",
                                [callback, error, res](auto wrapper) mutable {
                                    wrapper.index_name = "纪录片";
                                    res["3"]           = wrapper;
                                    BilibiliClient::get_pgc_filter(
                                        "7",
                                        [callback, error,
                                         res](auto wrapper) mutable {
                                            wrapper.index_name = "综艺";
                                            res["7"]           = wrapper;
                                            BilibiliClient::get_pgc_filter(
                                                "102",
                                                [callback,
                                                 res](auto wrapper) mutable {
                                                    wrapper.index_name =
                                                        "影视综合";
                                                    res["102"] = wrapper;
                                                    callback(res);
                                                },
                                                error);
                                        },
                                        error);
                                },
                                error);
                        },
                        error);
                },
                error);
        },
        error);
}
}  // namespace bilibili