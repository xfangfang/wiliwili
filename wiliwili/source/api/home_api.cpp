#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "bilibili.h"
#include "bilibili/api.h"
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
void BilibiliClient::get_recommend(int index, int num, int fresh_type, std::string feed_version, int x_num, int y_num,
                                   const std::function<void(RecommendVideoListResultWrapper)>& callback,
                                   const ErrorCallback& error) {
    cpr::Parameters parameters = {
        {"fresh_idx", std::to_string(index)},  // 手动刷新后重新计数，从1开始
        {"ps", std::to_string(num)},           // 手动刷新 30， 自动加载 15， 初始加载 10， 精选 10
        {"feed_version", feed_version},        // 首页是 V1，精选是 CLIENT_SELECTED
        {"fresh_type", std::to_string(fresh_type)},  // 手动刷新是 3， 自动加载是 4，初始加载是 0
        {"plat", "1"},
    };
    if (feed_version == "V1") {
        parameters.Add({
            {"x_num", std::to_string(x_num)},  // 只在首页存在，固定为 3
            {"y_num", std::to_string(y_num)},  // 只在首页存在，根据同屏卡片列数设置数字，默认为 4
        });
    }

    HTTP::getResultAsync<RecommendVideoListResultWrapper>(
        Api::Recommend, parameters,
        [callback, index](RecommendVideoListResultWrapper wrapper) {
            wrapper.requestIndex = index;
            callback(wrapper);
        },
        error);
}

/// 主页 热门 热门综合
void BilibiliClient::get_hots_all(int index, int num, const std::function<void(HotsAllVideoListResult, bool)>& callback,
                                  const ErrorCallback& error) {
    HTTP::getResultAsync<HotsAllVideoListResultWrapper>(
        Api::HotsAll, {{"pn", std::to_string(index)}, {"ps", std::to_string(num)}},
        [callback](const HotsAllVideoListResultWrapper& wrapper) { callback(wrapper.list, wrapper.no_more); }, error);
}

/// 主页 热门 每周推荐列表
void BilibiliClient::get_hots_weekly_list(const std::function<void(HotsWeeklyListResult)>& callback,
                                          const ErrorCallback& error) {
    HTTP::getResultAsync<HotsWeeklyResultWrapper>(
        Api::HotsWeeklyList, {}, [callback](const HotsWeeklyResultWrapper& wrapper) { callback(wrapper.list); }, error);
}

/// 主页 热门 每周推荐
void BilibiliClient::get_hots_weekly(
    int number, const std::function<void(HotsWeeklyVideoListResult, std::string, std::string)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<HotsWeeklyVideoListResultWrapper>(
        Api::HotsWeekly, {{"number", std::to_string(number)}},
        [callback](const HotsWeeklyVideoListResultWrapper& wrapper) {
            callback(wrapper.list, wrapper.config.label, wrapper.reminder);
        },
        error);
}

/// 主页 热门 入站必刷
void BilibiliClient::get_hots_history(const std::function<void(HotsHistoryVideoListResult, std::string)>& callback,
                                      const ErrorCallback& error) {
    HTTP::getResultAsync<HotsHistoryVideoListResultWrapper>(
        Api::HotsHistory, {},
        [callback](const HotsHistoryVideoListResultWrapper& wrapper) { callback(wrapper.list, wrapper.explain); },
        error);
}

/// 主页 热门 排行榜 投稿视频
void BilibiliClient::get_hots_rank(int rid, const std::string& type,
                                   const std::function<void(HotsRankVideoListResult, std::string)>& callback,
                                   const ErrorCallback& error) {
    HTTP::getResultAsync<HotsRankVideoListResultWrapper>(
        Api::HotsRank, {{"rid", std::to_string(rid)}, {"type", type}},
        [callback](auto wrapper) { callback(wrapper.list, wrapper.note); }, error);
}

/// 主页 热门 排行榜 官方
void BilibiliClient::get_hots_rank_pgc(int season_type, int day,
                                       const std::function<void(HotsRankPGCVideoListResult, std::string)>& callback,
                                       const ErrorCallback& error) {
    HTTP::getResultAsync<HotsRankPGCVideoListResultWrapper>(
        Api::HotsRankPGC, {{"season_type", std::to_string(season_type)}, {"day", std::to_string(day)}},
        [callback](auto wrapper) { callback(wrapper.list, wrapper.note); }, error);
}

/// 主页 直播推荐
void BilibiliClient::get_live_recommend(int parent_area_id, int area_id, int page, const std::string& source,
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

/// 主页 二级分区直播推荐，不包含关注
void BilibiliClient::get_live_recommend_second(int parent_area_id, int area_id, int page,
                                               const std::function<void(LiveSecondResultWrapper)>& callback,
                                               const ErrorCallback& error) {
    HTTP::getResultAsync<LiveSecondResultWrapper>(
        Api::LiveFeedSecond,
        {
            {"parent_area_id", std::to_string(parent_area_id)},
            {"area_id", std::to_string(area_id)},
            {"page", std::to_string(page)},
            {"page_size", "20"},
            {"platform", "web"},
            {"device", "switch"},
        },
        [callback](auto wrapper) { callback(wrapper); }, error, true);
}

/// 主页 直播分区列表
void BilibiliClient::get_live_area_list(const std::function<void(LiveFullAreaResultWrapper)>& callback,
                                        const ErrorCallback& error) {
    HTTP::getResultAsync<LiveFullAreaResultWrapper>(
        Api::LiveAreaList,
        {
            {"platform", "web"},
        },
        [callback](auto wrapper) { callback(wrapper); }, error, true);
}

/// 主页 追番列表
void BilibiliClient::get_bangumi(int is_refresh, const std::string& cursor,
                                 const std::function<void(PGCResultWrapper)>& callback, const ErrorCallback& error) {
    HTTP::getResultAsync<PGCResultWrapper>(
        Api::Bangumi,
        {
            {"is_refresh", std::to_string(is_refresh)},
            {"cursor", cursor},
        },
        [callback](auto wrapper) { callback(wrapper); }, error);
}

/// 主页 影视列表
void BilibiliClient::get_cinema(int is_refresh, const std::string& cursor,
                                const std::function<void(PGCResultWrapper)>& callback, const ErrorCallback& error) {
    HTTP::getResultAsync<PGCResultWrapper>(
        Api::Cinema,
        {
            {"is_refresh", std::to_string(is_refresh)},
            {"cursor", cursor},
        },
        [callback](auto wrapper) { callback(wrapper); }, error);
}

/// 主页 追番/影视 分类检索
void BilibiliClient::get_pgc_index(const std::string& param, int page,
                                   const std::function<void(PGCIndexResultWrapper)>& callback,
                                   const ErrorCallback& error) {
    HTTP::getResultAsync<PGCIndexResultWrapper>(
        Api::PGCIndex + "?" + param + "&page=" + std::to_string(page), {},
        [callback](auto wrapper) { callback(wrapper); }, error);
}

/// 主页 追番/影视 获取分类
void BilibiliClient::get_pgc_filter(const std::string& index_type,
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
void BilibiliClient::get_pgc_all_filter(const std::function<void(PGCIndexFilters)>& callback,
                                        const ErrorCallback& error) {
    cpr::async([callback, error]() {
        std::vector<std::string> index_types = {"1", "2", "5", "3", "7", "102"};
        std::vector<std::string> index_names = {"追番", "电影", "电视剧", "纪录片", "综艺", "影视综合"};
        cpr::MultiPerform multiperform;
        for (auto& i : index_types) {
            std::shared_ptr<cpr::Session> session = std::make_shared<cpr::Session>();
            session->SetUrl(Api::PGCIndexFilter);
            session->SetParameters({{"type", "2"}, {"index_type", i}});
            session->SetHeader(HTTP::HEADERS);
            session->SetVerifySsl(HTTP::VERIFY);
            session->SetProxies(HTTP::PROXIES);
            session->SetTimeout(HTTP::TIMEOUT);
            session->SetCookies(HTTP::COOKIES);
            session->SetHttpVersion(cpr::HttpVersion{cpr::HttpVersionCode::VERSION_2_0_TLS});
            multiperform.AddSession(session);
        }

        std::vector<cpr::Response> responses = multiperform.Get();
        PGCIndexFilters res;
        for (size_t i = 0; i < responses.size(); i++) {
            auto& r = responses[i];
            if (r.error) {
                ERROR_MSG(r.error.message, -1);
                return;
            } else if (r.status_code != 200) {
                ERROR_MSG("Network error. [Status code: " + std::to_string(r.status_code) + " ]", r.status_code);
                return;
            }
            int ret = HTTP::parseJson<PGCIndexFilterWrapper>(
                r,
                [&res, &index_types, &index_names, i, callback](auto wrapper) {
                    wrapper.index_name  = index_names[i];
                    res[index_types[i]] = wrapper;
                },
                error);
            if (ret != 0) {
                return;
            }
        }
        callback(res);
    });
}
}  // namespace bilibili