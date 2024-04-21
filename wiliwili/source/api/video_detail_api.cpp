#include <nlohmann/json.hpp>
#include <utility>

#include "bilibili.h"
#include "bilibili/util/http.hpp"
#include "bilibili/result/video_detail_result.h"
#include "bilibili/result/home_live_result.h"
#include "bilibili/result/home_pgc_season_result.h"

namespace bilibili {

void BilibiliClient::get_video_detail(const std::string& bvid, const std::function<void(VideoDetailResult)>& callback,
                                      const ErrorCallback& error) {
    HTTP::getResultAsync<VideoDetailResult>(Api::Detail, {{"bvid", bvid}}, callback, error);
}

void BilibiliClient::get_video_detail(int aid, const std::function<void(VideoDetailResult)>& callback,
                                      const ErrorCallback& error) {
    HTTP::getResultAsync<VideoDetailResult>(Api::Detail, {{"aid", std::to_string(aid)}}, callback, error);
}

void BilibiliClient::get_video_detail_all(const std::string& bvid,
                                          const std::function<void(VideoDetailAllResult)>& callback,
                                          const ErrorCallback& error) {
    HTTP::getResultAsync<VideoDetailAllResult>(Api::DetailAll, {{"bvid", bvid}}, callback, error);
}

void BilibiliClient::get_page_detail(int aid, int cid, const std::function<void(VideoPageResult)>& callback,
                                     const ErrorCallback& error) {
    HTTP::getResultAsync<VideoPageResult>(Api::PageDetail, {{"aid", std::to_string(aid)}, {"cid", std::to_string(cid)}},
                                          callback, error);
}

void BilibiliClient::get_page_detail(const std::string& bvid, int cid,
                                     const std::function<void(VideoPageResult)>& callback, const ErrorCallback& error) {
    HTTP::getResultAsync<VideoPageResult>(Api::PageDetail, {{"bvid", bvid}, {"cid", std::to_string(cid)}}, callback,
                                          error);
}

void BilibiliClient::get_webmask(const std::string& url, uint64_t rangeStart, uint64_t rangeEnd,
                                 const std::function<void(std::string)>& callback, const ErrorCallback& error) {
    std::optional<std::int64_t> start, end;
    if (rangeStart != -1) start = rangeStart;
    if (rangeEnd != -1) end = rangeEnd;
    cpr::GetCallback<>(
        [callback, error](const cpr::Response& r) {
            try {
                callback(r.text);
            } catch (const std::exception& e) {
                ERROR_MSG("Network error. [Status code: " + std::to_string(r.status_code) + " ]", r.status_code);
            }
        },
        cpr::Range{start, end}, HTTP::VERIFY, HTTP::PROXIES, cpr::Url{url}, HTTP::HEADERS, HTTP::COOKIES,
        cpr::Timeout{HTTP::TIMEOUT});
}

void BilibiliClient::get_video_pagelist(const std::string& bvid,
                                        const std::function<void(VideoDetailPageListResult Result)>& callback,
                                        const ErrorCallback& error) {
    HTTP::getResultAsync<VideoDetailPageListResult>(Api::PlayPageList, {{"bvid", std::string(bvid)}}, callback, error);
}

void BilibiliClient::get_video_pagelist(int aid, const std::function<void(VideoDetailPageListResult)>& callback,
                                        const ErrorCallback& error) {
    HTTP::getResultAsync<VideoDetailPageListResult>(Api::PlayPageList, {{"aid", std::to_string(aid)}}, callback, error);
}

void BilibiliClient::get_video_url(const std::string& bvid, int cid, int qn,
                                   const std::function<void(VideoUrlResult)>& callback, const ErrorCallback& error) {
    HTTP::getResultAsync<VideoUrlResult>(Api::PlayInformation,
                                         {{"bvid", std::string(bvid)},
                                          {"cid", std::to_string(cid)},
                                          {"qn", std::to_string(qn)},
                                          {"fourk", "1"},
                                          {"fnval", FNVAL},
                                          {"fnver", "0"}},
                                         callback, error);
}

void BilibiliClient::get_video_url(int aid, int cid, int qn, const std::function<void(VideoUrlResult)>& callback,
                                   const ErrorCallback& error) {
    HTTP::getResultAsync<VideoUrlResult>(Api::PlayInformation,
                                         {{"aid", std::to_string(aid)},
                                          {"cid", std::to_string(cid)},
                                          {"qn", std::to_string(qn)},
                                          {"fourk", "1"},
                                          {"fnval", FNVAL},
                                          {"fnver", "0"}},
                                         callback, error);
}

void BilibiliClient::get_video_url_cast(int oid, int cid, int type, int qn, const std::string& csrf,
                                        const std::function<void(VideoUrlResult)>& callback,
                                        const ErrorCallback& error) {
    HTTP::getResultAsync<VideoUrlResult>(Api::PlayUrlCast,
                                         {{"build", "105001"},
                                          {"is_proj", "1"},
                                          {"device_type", "1"},
                                          {"protocol", "1"},
                                          {"mobile_access_key", csrf},
                                          {"mobi_app", "android_tv_yst"},
                                          {"platform", "android"},
                                          {"playurl_type", std::to_string(type)},
                                          {"object_id", std::to_string(oid)},
                                          {"cid", std::to_string(cid)},
                                          {"qn", std::to_string(qn)},
                                          {"fourk", "1"},
                                          {"fnval", "128"},
                                          {"fnver", "0"}},
                                         callback, error);
}

void BilibiliClient::get_comment(const std::string& oid, int next, int mode, int type,
                                 const std::function<void(VideoCommentResultWrapper)>& callback,
                                 const ErrorCallback& error) {
    HTTP::getResultAsync<VideoCommentResultWrapper>(
        Api::Comment,
        {{"mode", std::to_string(mode)},
         {"next", std::to_string(next)},
         {"oid", oid},
         {"plat", "1"},
         {"type", std::to_string(type)}},
        [callback, next](VideoCommentResultWrapper result) {
            result.requestIndex = next;
            callback(result);
        },
        error);
}

void BilibiliClient::get_comment_detail(const std::string& access_key, const std::string& oid, int64_t rpid,
                                        size_t next, int type,
                                        const std::function<void(VideoSingleCommentDetail)>& callback,
                                        const ErrorCallback& error) {
    HTTP::getResultAsync<VideoSingleCommentDetail>(Api::CommentDetail,
                                                   {{"csrf", access_key},
                                                    {"next", std::to_string(next)},
                                                    {"oid", oid},
                                                    {"root", std::to_string(rpid)},
                                                    {"type", std::to_string(type)}},
                                                   callback, error);
}

void BilibiliClient::get_season_detail(int seasonID, int epID, const std::function<void(SeasonResultWrapper)>& callback,
                                       const ErrorCallback& error) {
    cpr::Parameters params;
    if (epID == 0) {
        params = {{"season_id", std::to_string(seasonID)}};
    } else {
        params = {{"ep_id", std::to_string(epID)}};
    }

    HTTP::getResultAsync<SeasonResultWrapper>(Api::SeasonDetail, params, callback, error);
}

void BilibiliClient::get_season_recommend(size_t seasonID, const std::function<void(SeasonRecommendWrapper)>& callback,
                                          const ErrorCallback& error) {
    HTTP::getResultAsync<SeasonRecommendWrapper>(Api::SeasonRCMD, {{"season_id", std::to_string(seasonID)}}, callback,
                                                 error);
}

void BilibiliClient::get_season_status(size_t seasonID, const std::function<void(SeasonStatusResult)>& callback,
                                       const ErrorCallback& error) {
    HTTP::getResultAsync<SeasonStatusResult>(Api::SeasonStatus,
                                             cpr::Parameters{{"season_id", std::to_string(seasonID)},
                                                             {"ts", std::to_string(wiliwili::getUnixTime() * 1000)}},
                                             callback, error);
}

void BilibiliClient::get_season_url(int cid, int qn, const std::function<void(VideoUrlResult)>& callback,
                                    const ErrorCallback& error) {
    HTTP::getResultAsync<VideoUrlResult>(
        Api::SeasonUrl,
        {{"cid", std::to_string(cid)}, {"qn", std::to_string(qn)}, {"fourk", "1"}, {"fnval", FNVAL}, {"fnver", "0"}},
        callback, error);
}

void BilibiliClient::get_live_url(int roomid, int qn, const std::function<void(LiveUrlResultWrapper)>& callback,
                                  const ErrorCallback& error) {
    HTTP::getResultAsync<LiveUrlResultWrapper>(
        Api::LiveUrl, {{"cid", std::to_string(roomid)}, {"platform", "web"}, {"qn", std::to_string(qn)}}, callback,
        error);
}

void BilibiliClient::get_live_room_play_info(int roomid, int qn, const std::function<void(LiveRoomPlayInfo)>& callback,
                                             const ErrorCallback& error) {
    HTTP::getResultAsync<LiveRoomPlayInfo>(Api::RoomPlayInfo,
                                           {{"room_id", std::to_string(roomid)},
                                            {"no_playurl", "0"},
                                            {"mask", "1"},
                                            {"qn", std::to_string(qn)},
                                            {"platform", "web"},
                                            {"protocol", "0,1"},
                                            {"format", "0,1,2"},
                                            {"codec", "0,1,2"},
                                            {"dolby", "5"},
                                            {"ptype", "8"},
                                            {"panorama", "1"}},
                                           callback, error);
}

void BilibiliClient::get_live_pay_info(int roomid, const std::function<void(LivePayInfo)>& callback,
                                       const ErrorCallback& error) {
    HTTP::__cpr_get(
        Api::RoomPayInfo, {{"room_id", std::to_string(roomid)}},
        [callback, error](const cpr::Response& r) {
            try {
                nlohmann::json res = nlohmann::json::parse(r.text);
                auto ret           = res.at("data").get<LivePayInfo>();
                ret.message        = res.at("message").get<std::string>();
                CALLBACK(ret);
            } catch (const std::exception& e) {
                ERROR_MSG("cannot get live pay info", -1);
            }
        },
        error);
}

void BilibiliClient::get_live_pay_link(int roomid, const std::function<void(LivePayLink)>& callback,
                                       const ErrorCallback& error) {
    HTTP::getResultAsync<LivePayLink>(Api::RoomPayLink, {{"room_id", std::to_string(roomid)}}, callback, error);
}

/// 视频页 获取单个视频播放人数
void BilibiliClient::get_video_online(int aid, int cid, const std::function<void(VideoOnlineTotal)>& callback,
                                      const ErrorCallback& error) {
    HTTP::getResultAsync<VideoOnlineTotal>(Api::OnlineViewerCount,
                                           {
                                               {"cid", std::to_string(cid)},
                                               {"aid", std::to_string(aid)},
                                           },
                                           callback, error);
}

void BilibiliClient::get_video_online(const std::string& bvid, int cid,
                                      const std::function<void(VideoOnlineTotal)>& callback,
                                      const ErrorCallback& error) {
    HTTP::getResultAsync<VideoOnlineTotal>(Api::OnlineViewerCount,
                                           {
                                               {"cid", std::to_string(cid)},
                                               {"bvid", bvid},
                                           },
                                           callback, error);
}

/// 视频页 获取点赞/收藏/投币情况
void BilibiliClient::get_video_relation(const std::string& bvid, const std::function<void(VideoRelation)>& callback,
                                        const ErrorCallback& error) {
    HTTP::getResultAsync<VideoRelation>(Api::VideoRelation, {{"bvid", bvid}}, callback, error);
}

/// 视频页 获取番剧点赞/收藏/投币情况
void BilibiliClient::get_video_relation(size_t epid, const std::function<void(VideoEpisodeRelation)>& callback,
                                        const ErrorCallback& error) {
    HTTP::getResultAsync<VideoEpisodeRelation>(Api::VideoEpisodeRelation, {{"ep_id", std::to_string(epid)}}, callback,
                                               error);
}

void BilibiliClient::get_danmaku(unsigned int cid, const std::function<void(std::string)>& callback,
                                 const ErrorCallback& error) {
    cpr::GetCallback<>(
        [callback, error](const cpr::Response& r) {
            try {
                callback(r.text);
            } catch (const std::exception& e) {
                ERROR_MSG("Network error. [Status code: " + std::to_string(r.status_code) + " ]", r.status_code);
                printf("data: %s\n", r.text.c_str());
                printf("ERROR: %s\n", e.what());
            }
        },
        HTTP::VERIFY, HTTP::PROXIES, cpr::Url{Api::VideoDanmaku}, HTTP::HEADERS,
        cpr::Parameters({{"oid", std::to_string(cid)}}), HTTP::COOKIES, cpr::Timeout{HTTP::TIMEOUT});
}

void BilibiliClient::get_highlight_progress(unsigned int cid,
                                            const std::function<void(VideoHighlightProgress)>& callback,
                                            const ErrorCallback& error) {
    cpr::GetCallback<>(
        [callback, error](const cpr::Response& r) {
            if (r.status_code != 200) {
                ERROR_MSG("Network error", r.status_code);
                return;
            }
            try {
                nlohmann::json res = nlohmann::json::parse(r.text);
                callback(res.get<VideoHighlightProgress>());
            } catch (const std::exception& e) {
                ERROR_MSG(e.what(), -1);
            }
        },
        HTTP::VERIFY, HTTP::PROXIES, cpr::Url{Api::VideoHighlight}, HTTP::HEADERS,
        cpr::Parameters({{"cid", std::to_string(cid)}}), HTTP::COOKIES, cpr::Timeout{HTTP::TIMEOUT});
}

void BilibiliClient::get_subtitle(const std::string& link, const std::function<void(SubtitleData)>& callback,
                                  const ErrorCallback& error) {
    std::string url = link;
    if (link.compare(0, 2, "//") == 0) {
        url = "https:" + url;
    }

    cpr::GetCallback<>(
        [callback, error](const cpr::Response& r) {
            try {
                nlohmann::json res = nlohmann::json::parse(r.text);
                callback(res.get<SubtitleData>());
            } catch (const std::exception& e) {
                ERROR_MSG("Network error. [Status code: " + std::to_string(r.status_code) + " ]", r.status_code);
                printf("data: %s\n", r.text.c_str());
                printf("ERROR: %s\n", e.what());
            }
        },
        HTTP::VERIFY, HTTP::PROXIES, cpr::Url{url}, HTTP::HEADERS, cpr::Parameters({}), HTTP::COOKIES,
        cpr::Timeout{HTTP::TIMEOUT});
}

/// 视频页 上报历史记录
void BilibiliClient::report_history(const std::string& mid, const std::string& access_key, unsigned int aid,
                                    unsigned int cid, int type, unsigned int progress, unsigned int duration,
                                    unsigned int sid, unsigned int epid, const std::function<void()>& callback,
                                    const ErrorCallback& error) {
    cpr::Payload payload = {
        {"mid", mid},
        {"csrf", access_key},
        {"aid", std::to_string(aid)},
        {"cid", std::to_string(cid)},
        {"progress", std::to_string(progress)},
        {"duration", std::to_string(duration)},
        {"type", std::to_string(type)},
        {"dt", "9"},  // 显示为智能音箱的播放数据
    };
    if (type == 4) {
        if (sid != 0) payload.Add({"sid", std::to_string(sid)});
        if (epid != 0) payload.Add({"epid", std::to_string(epid)});
    }

    HTTP::postResultAsync(Api::ProgressReport, {}, payload, callback, error);
}

/// 直播页 上报历史
void BilibiliClient::report_live_history(int room_id, const std::string& csrf, const std::function<void()>& callback,
                                         const ErrorCallback& error) {
    cpr::Payload payload = {
        {"room_id", std::to_string(room_id)},
        {"platform", "pc"},
        {"csrf_token", csrf},
        {"csrf", csrf},
        {"visit_id", ""},
    };

    HTTP::postResultAsync(Api::LiveReport, {}, payload, callback, error);
}

void BilibiliClient::be_agree(const std::string& access_key, int aid, bool is_like,
                              const std::function<void()>& callback, const ErrorCallback& error) {
    cpr::Payload payload = {
        {"aid", std::to_string(aid)},
        {"like", is_like ? "1" : "2"},
        {"csrf", access_key},
    };
    HTTP::postResultAsync(Api::LikeWeb, {}, payload, callback, error);
}

void BilibiliClient::be_agree_comment(const std::string& access_key, const std::string& oid, int64_t rpid, bool is_like,
                                      int type, const std::function<void()>& callback, const ErrorCallback& error) {
    cpr::Payload payload = {
        {"oid", oid},         {"rpid", std::to_string(rpid)}, {"action", is_like ? "1" : "0"},
        {"csrf", access_key}, {"type", std::to_string(type)}, {"ordering", "heat"},
    };
    HTTP::postResultAsync(Api::CommentLike, {}, payload, callback, error);
}

void BilibiliClient::ugc_season_subscribe(int id, const std::string& csrf, const std::function<void()>& callback,
                                          const ErrorCallback& error) {
    cpr::Payload payload = {
        {"season_id", std::to_string(id)},
        {"csrf", csrf},
        {"platform", "web"},
    };
    HTTP::postResultAsync(Api::UGCSeasonSubscribe, {}, payload, callback, error);
}

void BilibiliClient::ugc_season_unsubscribe(int id, const std::string& csrf, const std::function<void()>& callback,
                                            const ErrorCallback& error) {
    cpr::Payload payload = {
        {"season_id", std::to_string(id)},
        {"csrf", csrf},
        {"platform", "web"},
    };
    HTTP::postResultAsync(Api::UGCSeasonUnsubscribe, {}, payload, callback, error);
}

void BilibiliClient::delete_comment(const std::string& access_key, const std::string& oid, int64_t rpid, int type,
                                    const std::function<void()>& callback, const ErrorCallback& error) {
    cpr::Payload payload = {
        {"oid", oid},
        {"rpid", std::to_string(rpid)},
        {"csrf", access_key},
        {"type", std::to_string(type)},
    };
    HTTP::postResultAsync(Api::CommentDel, {}, payload, callback, error);
}

void BilibiliClient::add_comment(const std::string& access_key, const std::string& message, const std::string& oid,
                                 int64_t parent, int64_t root, int type,
                                 const std::function<void(VideoCommentAddResult)>& callback,
                                 const ErrorCallback& error) {
    cpr::Payload payload = {
        {"oid", oid}, {"csrf", access_key}, {"message", message}, {"type", std::to_string(type)}, {"plat", "1"},
    };
    if (parent) payload.Add({"parent", std::to_string(parent)});
    if (root) payload.Add({"root", std::to_string(root)});

    HTTP::postResultAsync<VideoCommentAddResult>(
        Api::CommentAdd, {}, payload,
        [callback, error](const VideoCommentAddResult& result) {
            if (result.success_action != 0) {
                ERROR_MSG("cannot add comment", -1);
            } else {
                CALLBACK(result);
            }
        },
        error);
}

void BilibiliClient::add_coin(const std::string& access_key, int aid, unsigned int coin_number, bool is_like,
                              const std::function<void()>& callback, const ErrorCallback& error) {
    cpr::Payload payload = {
        {"aid", std::to_string(aid)},
        {"select_like", is_like ? "1" : "0"},
        {"multiply", std::to_string(coin_number)},
        {"csrf", access_key},
    };

    printf("[add coin] aid: %d; coin: %d; select_like: %d;\n", aid, coin_number, is_like);
    HTTP::postResultAsync(Api::CoinWeb, {}, payload, callback, error);
}

void BilibiliClient::get_coin_exp(const std::function<void(int)>& callback, const ErrorCallback& error) {
    HTTP::getResultAsync<int>(Api::CoinExp, {}, callback, error);
}

void BilibiliClient::add_resource(const std::string& access_key, int rid, int type, const std::string& add_ids,
                                  const std::string& del_ids, const std::function<void()>& callback,
                                  const ErrorCallback& error) {
    cpr::Payload payload = {
        {"rid", std::to_string(rid)}, {"type", std::to_string(type)}, {"add_media_ids", add_ids},
        {"del_media_ids", del_ids},   {"csrf", access_key},
    };
    HTTP::postResultAsync(Api::CollectionVideoListSave, {}, payload, callback, error);
}

void BilibiliClient::triple_like(const std::string& access_key, int aid, const std::function<void()>& callback,
                                 const ErrorCallback& error) {
    cpr::Payload payload = {
        {"aid", std::to_string(aid)},
        {"csrf", access_key},
    };
    HTTP::postResultAsync(Api::TripleWeb, {}, payload, callback, error);
}

void BilibiliClient::follow_up(const std::string& access_key, const std::string& mid, bool follow,
                               const std::function<void()>& callback, const ErrorCallback& error) {
    cpr::Payload payload = {
        {"fid", mid},
        {"act", follow ? "1" : "2"},
        {"csrf", access_key},
    };
    HTTP::postResultAsync(Api::Follow, {}, payload, callback, error);
}

void BilibiliClient::follow_season(const std::string& access_key, size_t season, bool follow,
                                   const std::function<void()>& callback, const ErrorCallback& error) {
    cpr::Payload payload = {
        {"season_id", std::to_string(season)},
        {"csrf", access_key},
    };
    if (follow)
        HTTP::postResultAsync(Api::FollowSeason, {}, payload, callback, error);
    else
        HTTP::postResultAsync(Api::UndoFollowSeason, {}, payload, callback, error);
}

}  // namespace bilibili