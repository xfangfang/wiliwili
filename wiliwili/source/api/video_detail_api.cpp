#include <nlohmann/json.hpp>
#include <utility>

#include "bilibili.h"
#include "bilibili/util/http.hpp"
#include "bilibili/result/video_detail_result.h"
#include "bilibili/result/home_live_result.h"

namespace bilibili {

void BilibiliClient::get_video_detail(
    const std::string& bvid,
    const std::function<void(VideoDetailResult)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<VideoDetailResult>(Api::Detail, {{"bvid", bvid}},
                                            callback, error);
}

void BilibiliClient::get_video_detail(
    const int aid, const std::function<void(VideoDetailResult)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<VideoDetailResult>(
        Api::Detail, {{"aid", std::to_string(aid)}}, callback, error);
}

void BilibiliClient::get_video_detail_all(
    const std::string& bvid,
    const std::function<void(VideoDetailAllResult)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<VideoDetailAllResult>(Api::DetailAll, {{"bvid", bvid}},
                                               callback, error);
}

void BilibiliClient::get_video_pagelist(
    const std::string& bvid,
    const std::function<void(VideoDetailPageListResult Result)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<VideoDetailPageListResult>(
        Api::PlayPageList, {{"bvid", std::string(bvid)}}, callback, error);
}

void BilibiliClient::get_video_pagelist(
    const int aid,
    const std::function<void(VideoDetailPageListResult)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<VideoDetailPageListResult>(
        Api::PlayPageList, {{"aid", std::to_string(aid)}}, callback, error);
}

void BilibiliClient::get_video_url(
    const std::string& bvid, const int cid, const int qn,
    const std::function<void(VideoUrlResult)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<VideoUrlResult>(Api::PlayInformation,
                                         {{"bvid", std::string(bvid)},
                                          {"cid", std::to_string(cid)},
                                          {"qn", std::to_string(qn)},
                                          {"fourk", "1"},
                                          {"fnval", "1744"},
                                          {"fnver", "0"}},
                                         callback, error);
}

void BilibiliClient::get_video_url(
    const int aid, const int cid, const int qn,
    const std::function<void(VideoUrlResult)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<VideoUrlResult>(Api::PlayInformation,
                                         {{"aid", std::to_string(aid)},
                                          {"cid", std::to_string(cid)},
                                          {"qn", std::to_string(qn)},
                                          {"fourk", "1"},
                                          {"fnval", "1744"},
                                          {"fnver", "0"}},
                                         callback, error);
}

void BilibiliClient::get_comment(
    int aid, int next, int mode,
    const std::function<void(VideoCommentResultWrapper)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<VideoCommentResultWrapper>(
        Api::Comment,
        {{"mode", std::to_string(mode)},
         {"next", std::to_string(next)},
         {"oid", std::to_string(aid)},
         {"plat", "1"},
         {"type", "1"}},
        callback, error);
}

void BilibiliClient::get_season_detail(
    const int seasonID, const int epID,
    const std::function<void(SeasonResultWrapper)>& callback,
    const ErrorCallback& error) {
    cpr::Parameters params;
    if (epID == 0) {
        params = {{"season_id", std::to_string(seasonID)}};
    } else {
        params = {{"ep_id", std::to_string(epID)}};
    }

    HTTP::getResultAsync<SeasonResultWrapper>(Api::SeasonDetail, params,
                                              callback, error);
}

void BilibiliClient::get_season_url(
    const int cid, const int qn,
    const std::function<void(VideoUrlResult)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<VideoUrlResult>(Api::SeasonUrl,
                                         {{"cid", std::to_string(cid)},
                                          {"qn", std::to_string(qn)},
                                          {"fourk", "1"},
                                          {"fnval", "16"},
                                          {"fnver", "0"}},
                                         callback, error);
}

void BilibiliClient::get_live_url(
    const int roomid, const int qn,
    const std::function<void(LiveUrlResultWrapper)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<LiveUrlResultWrapper>(Api::LiveUrl,
                                               {{"cid", std::to_string(roomid)},
                                                {"platform", "web"},
                                                {"qn", std::to_string(qn)}},
                                               callback, error);
}

/// 视频页 获取单个视频播放人数
void BilibiliClient::get_video_online(
    int aid, int cid, const std::function<void(VideoOnlineTotal)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<VideoOnlineTotal>(Api::OnlineViewerCount,
                                           {
                                               {"cid", std::to_string(cid)},
                                               {"aid", std::to_string(aid)},
                                           },
                                           callback, error);
}

void BilibiliClient::get_video_online(
    const std::string& bvid, int cid,
    const std::function<void(VideoOnlineTotal)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<VideoOnlineTotal>(Api::OnlineViewerCount,
                                           {
                                               {"cid", std::to_string(cid)},
                                               {"bvid", bvid},
                                           },
                                           callback, error);
}

/// 视频页 获取点赞/收藏/投屏情况
void BilibiliClient::get_video_relation(
    const std::string& bvid, const std::function<void(VideoRelation)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<VideoRelation>(Api::VideoRelation, {{"bvid", bvid}},
                                        callback, error);
}

void BilibiliClient::get_danmaku(
    const unsigned int cid, const std::function<void(std::string)>& callback,
    const ErrorCallback& error) {
    cpr::GetCallback<>(
        [callback, error](cpr::Response r) {
            try {
                callback(r.text);
            } catch (const std::exception& e) {
                ERROR_MSG("Network error. [Status code: " +
                              std::to_string(r.status_code) + " ]",
                          -404);
                printf("data: %s\n", r.text.c_str());
                printf("ERROR: %s\n", e.what());
            }
        },
#ifndef VERIFY_SSL
        cpr::VerifySsl{false},
#endif
        cpr::Url{Api::VideoDanmaku}, HTTP::HEADERS,
        cpr::Parameters({{"oid", std::to_string(cid)}}), HTTP::COOKIES,
        cpr::Timeout{HTTP::TIMEOUT});
}

/// 视频页 上报历史记录
void BilibiliClient::report_history(
    const std::string& mid, const std::string& access_key, unsigned int aid,
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

    HTTP::__cpr_post(
        Api::ProgressReport, {}, payload,
        [callback, error](const cpr::Response& r) {
            if (r.status_code != 200) {
                ERROR_MSG(
                    "ERROR report_history: " + std::to_string(r.status_code),
                    r.status_code);
            } else {
                nlohmann::json res = nlohmann::json::parse(r.text);
                int code           = res.at("code");
                if (code == 0) {
                    callback();
                } else {
                    ERROR_MSG("ERROR report_history:" +
                                  res.at("message").get<std::string>(),
                              code);
                }
            }
        });
}

/// 直播页 上报历史
void BilibiliClient::report_live_history(const int room_id,
                                         const std::string& csrf,
                                         const std::function<void()>& callback,
                                         const ErrorCallback& error) {
    cpr::Payload payload = {
        {"room_id", std::to_string(room_id)},
        {"platform", "pc"},
        {"csrf_token", csrf},
        {"csrf", csrf},
        {"visit_id", ""},
    };

    HTTP::__cpr_post(Api::LiveReport, {}, payload,
                     [callback, error](const cpr::Response& r) {
                         if (r.status_code != 200) {
                             ERROR_MSG(
                                 "ERROOR: report_live_history: status_code: " +
                                     std::to_string(r.status_code),
                                 r.status_code);
                         } else {
                             callback();
                         }
                     });
}

void BilibiliClient::be_agree(const std::string& access_key, int aid,
                              bool is_like,
                              const std::function<void()>& callback,
                              const ErrorCallback& error) {
    cpr::Payload payload = {
        {"aid", std::to_string(aid)},
        {"like", std::to_string(is_like)},
        {"csrf", access_key},
    };
    HTTP::__cpr_post("http://api.bilibili.com/x/web-interface/archive/like", {},
                     payload, [callback, error](const cpr::Response& r) {
                         if (r.status_code != 200) {
                             ERROR_MSG("ERROOR: report_history: status_code: " +
                                           std::to_string(r.status_code),
                                       r.status_code);
                         } else {
                             callback();
                         }
                     });
}

void BilibiliClient::add_coin(const std::string& access_key, int aid,
                              unsigned int coin_number, bool is_like,
                              const std::function<void()>& callback,
                              const ErrorCallback& error) {
    cpr::Payload payload = {
        {"aid", std::to_string(aid)},
        {"select_like", std::to_string(is_like)},
        {"multiply", std::to_string(coin_number)},
        {"csrf", access_key},
    };
    HTTP::__cpr_post("http://api.bilibili.com/x/web-interface/coin/add", {},
                     payload, [callback, error](const cpr::Response& r) {
                         if (r.status_code != 200) {
                             ERROR_MSG("ERROOR: report_history: status_code: " +
                                           std::to_string(r.status_code),
                                       r.status_code);
                         } else {
                             callback();
                         }
                     });
}

void BilibiliClient::add_resource(const std::string& access_key, int aid,
                                  const std::function<void()>& callback,
                                  const ErrorCallback& error) {
    cpr::Payload payload = {
        {"rid", std::to_string(aid)},
        {"type", std::to_string(2)},
        {"add_media_ids", std::to_string(1)},
        {"csrf", access_key},
    };
    HTTP::__cpr_post(
        "http://api.bilibili.com/medialist/gateway/coll/resource/deal", {},
        payload, [callback, error](const cpr::Response& r) {
            if (r.status_code != 200) {
                ERROR_MSG("ERROOR: report_history: status_code: " +
                              std::to_string(r.status_code),
                          r.status_code);
            } else {
                callback();
            }
        });
}

}  // namespace bilibili