#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "bilibili.h"
#include "bilibili/util/md5.hpp"
#include "curl/curl.h"
#include "bilibili/util/http.hpp"

#include "bilibili/result/home_pgc_result.h"

namespace bilibili {

    /// 主页 推荐
    void BilibiliClient::get_recommend(const int index, const int num,
                                       const std::function<void(RecommendVideoListResult)>& callback,
                                       const ErrorCallback& error){
//        BilibiliClient::pool.enqueue([=]{
            HTTP::getResultAsync<RecommendVideoListResultWrapper>(Api::Recommend,
                                               {{"fresh_idx", std::to_string(index)},
                                                {"ps", std::to_string(num)},
                                                {"feed_version", "V1"},
                                                {"fresh_type", "4"},
                                                {"plat", "1"},
                                                },
                                               [callback](const RecommendVideoListResultWrapper& wrapper){
                                                   callback(wrapper.item);
                }, error);
//        });
    }

    /// 主页 热门 热门综合
    void BilibiliClient::get_hots_all(const int index, const int num,
                                       const std::function<void(HotsAllVideoListResult, bool)>& callback,
                                       const ErrorCallback& error){
        HTTP::getResultAsync<HotsAllVideoListResultWrapper>(Api::HotsAll,
                                                              {{"pn", std::to_string(index)},
                                                               {"ps", std::to_string(num)}
                                                              },
                                                              [callback](const HotsAllVideoListResultWrapper& wrapper){
                                                                  callback(wrapper.list, wrapper.no_more);
                                                              }, error);
    }

    /// 主页 热门 每周推荐列表
    void BilibiliClient::get_hots_weekly_list(const std::function<void(HotsWeeklyListResult)>& callback,
                                     const ErrorCallback& error){
        HTTP::getResultAsync<HotsWeeklyResultWrapper>(Api::HotsWeeklyList,
                                                            {},
                                                            [callback](const HotsWeeklyResultWrapper& wrapper){
                                                                callback(wrapper.list);
                                                            }, error);
    }

    /// 主页 热门 每周推荐
    void BilibiliClient::get_hots_weekly(const int number,
                                const std::function<void(HotsWeeklyVideoListResult, string, string)>& callback,
                                const ErrorCallback& error){
        HTTP::getResultAsync<HotsWeeklyVideoListResultWrapper>(Api::HotsWeekly,
                                                            {{"number", std::to_string(number)}},
                                                            [callback](const HotsWeeklyVideoListResultWrapper& wrapper){
                                                                callback(wrapper.list, wrapper.config.label, wrapper.reminder);
                                                            }, error);

    }

    /// 主页 热门 入站必刷
    void BilibiliClient::get_hots_history(
            const std::function<void(HotsHistoryVideoListResult, string)>& callback,
            const ErrorCallback& error){
        HTTP::getResultAsync<HotsHistoryVideoListResultWrapper>(Api::HotsHistory,
                                                               {},
                                                               [callback](const HotsHistoryVideoListResultWrapper& wrapper){
                                                                   callback(wrapper.list, wrapper.explain);
                                                               }, error);
    }

    /// 主页 热门 排行榜 投稿视频
    void BilibiliClient::get_hots_rank(const int rid, const string type,
                              const std::function<void(HotsRankVideoListResult , string)>& callback,
                              const ErrorCallback& error){
        HTTP::getResultAsync<HotsRankVideoListResultWrapper>(Api::HotsRank,
                                                                {{"rid", std::to_string(rid)},
                                                                 {"type", type}},
                                                                [callback](auto wrapper){
                                                                    callback(wrapper.list, wrapper.note);
                                                                }, error);
    }

    /// 主页 热门 排行榜 官方
    void BilibiliClient::get_hots_rank_pgc(const int season_type, const int day,
                                  const std::function<void(HotsRankPGCVideoListResult , string)>& callback,
                                  const ErrorCallback& error){
        HTTP::getResultAsync<HotsRankPGCVideoListResultWrapper>(Api::HotsRankPGC,
                                                                {{"season_type", std::to_string(season_type)},
                                                                 {"day", std::to_string(day)}},
                                                                [callback](auto wrapper){
                                                                    callback(wrapper.list, wrapper.note);
                                                                }, error);
    }

    /// 主页 直播推荐
     void BilibiliClient::get_live_recommend(int parent_area_id, int area_id, int page,
                                   const std::function<void(LiveAreaListResult , LiveVideoListResult, int)>& callback,
                                   const ErrorCallback& error){
        HTTP::getResultAsync<LiveResultWrapper>(Api::LiveFeed,
                                                                {{"parent_area_id", std::to_string(parent_area_id)},
                                                                 {"area_id", std::to_string(area_id)},
                                                                 {"device", "switch"},
                                                                 {"page", std::to_string(page)},
                                                                 {"platform", "web"},
                                                                 {"scale", "xxhdpi"},
                                                                 {"source_name", "pc"},
                                                                 {"mobi_app", "pc_electron"},
                                                                 },
                                                                [callback](auto wrapper){
                                                                    callback(wrapper.live_list, wrapper.card_list, wrapper.has_more);
                                                                }, error, true);
    }

    /// 主页 追番列表
    void BilibiliClient::get_bangumi(int is_refresh, int cursor,
                            const std::function<void(PGCModuleListResult , int, std::string)>& callback,
                            const ErrorCallback& error){
        HTTP::getResultAsync<PGCResultWrapper>(Api::Bangumi,
                                               {{"is_refresh", std::to_string(is_refresh)},
                                                 {"cursor", std::to_string(cursor)},
                                                },
                                               [callback](auto wrapper){
                                                    callback(wrapper.modules, wrapper.has_next, wrapper.next_cursor);
                                                }, error);
    }


    /// 主页 影视列表
    void BilibiliClient::get_cinema(int is_refresh, int cursor,
                                     const std::function<void(PGCModuleListResult , int, std::string)>& callback,
                                     const ErrorCallback& error){
        HTTP::getResultAsync<PGCResultWrapper>(Api::Cinema,
                                               {{"is_refresh", std::to_string(is_refresh)},
                                                    {"cursor", std::to_string(cursor)},
                                                   },
                                               [callback](auto wrapper){
                                                       callback(wrapper.modules, wrapper.has_next, wrapper.next_cursor);
                                                   }, error);
    }
}