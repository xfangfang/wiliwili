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
                                          {"fnval", FNVAL},
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
                                          {"fnval", FNVAL},
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

void BilibiliClient::get_comment_detail(
    const std::string& access_key, size_t oid, int64_t rpid, size_t next,
    const std::function<void(VideoSingleCommentDetail)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<VideoSingleCommentDetail>(
        Api::CommentDetail,
        {{"csrf", access_key},
         {"next", std::to_string(next)},
         {"oid", std::to_string(oid)},
         {"root", std::to_string(rpid)},
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

void BilibiliClient::get_season_status(
    size_t seasonID, const std::function<void(SeasonStatusResult)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<SeasonStatusResult>(
        Api::SeasonStatus,
        cpr::Parameters{{"season_id", std::to_string(seasonID)},
                        {"ts", std::to_string(wiliwili::getUnixTime() * 1000)}},
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
                                          {"fnval", FNVAL},
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

/// 视频页 获取点赞/收藏/投币情况
void BilibiliClient::get_video_relation(
    const std::string& bvid, const std::function<void(VideoRelation)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<VideoRelation>(Api::VideoRelation, {{"bvid", bvid}},
                                        callback, error);
}

/// 视频页 获取番剧点赞/收藏/投币情况
void BilibiliClient::get_video_relation(
    size_t epid, const std::function<void(VideoEpisodeRelation)>& callback,
    const ErrorCallback& error) {
    HTTP::getResultAsync<VideoEpisodeRelation>(
        Api::VideoEpisodeRelation, {{"ep_id", std::to_string(epid)}}, callback,
        error);
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

    HTTP::postResultAsync(Api::ProgressReport, {}, payload, callback, error);
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

    HTTP::postResultAsync(Api::LiveReport, {}, payload, callback, error);
}

void BilibiliClient::be_agree(const std::string& access_key, int aid,
                              bool is_like,
                              const std::function<void()>& callback,
                              const ErrorCallback& error) {
    cpr::Payload payload = {
        {"aid", std::to_string(aid)},
        {"like", is_like ? "1" : "2"},
        {"csrf", access_key},
    };
    HTTP::postResultAsync(Api::LikeWeb, {}, payload, callback, error);
}

void BilibiliClient::be_agree_comment(const std::string& access_key, size_t oid,
                                      int64_t rpid, bool is_like,
                                      const std::function<void()>& callback,
                                      const ErrorCallback& error) {
    cpr::Payload payload = {
        {"oid", std::to_string(oid)},
        {"rpid", std::to_string(rpid)},
        {"action", is_like ? "1" : "0"},
        {"csrf", access_key},
        {"type", "1"},
        {"ordering", "heat"},
    };
    HTTP::postResultAsync(Api::CommentLike, {}, payload, callback, error);
}

void BilibiliClient::delete_comment(const std::string& access_key, size_t oid,
                                    int64_t rpid,
                                    const std::function<void()>& callback,
                                    const ErrorCallback& error) {
    cpr::Payload payload = {
        {"oid", std::to_string(oid)},
        {"rpid", std::to_string(rpid)},
        {"csrf", access_key},
        {"type", "1"},
    };
    HTTP::postResultAsync(Api::CommentDel, {}, payload, callback, error);
}

void BilibiliClient::add_comment(
    const std::string& access_key, const std::string& message, size_t oid,
    int64_t parent, int64_t root,
    const std::function<void(VideoCommentAddResult)>& callback,
    const ErrorCallback& error) {
    cpr::Payload payload = {
        {"oid", std::to_string(oid)},
        {"csrf", access_key},
        {"message", message},
        {"type", "1"},
        {"plat", "1"},
    };
    if (parent) payload.Add({"parent", std::to_string(parent)});
    if (root) payload.Add({"root", std::to_string(root)});

    HTTP::postResultAsync<VideoCommentAddResult>(
        Api::CommentAdd, {}, payload,
        [callback, error](const VideoCommentAddResult& result) {
            if (result.success_action != 0) {
                ERROR_MSG("cannot add comment");
            } else {
                CALLBACK(result);
            }
        },
        error);
}

void BilibiliClient::add_coin(const std::string& access_key, int aid,
                              unsigned int coin_number, bool is_like,
                              const std::function<void()>& callback,
                              const ErrorCallback& error) {
    cpr::Payload payload = {
        {"aid", std::to_string(aid)},
        {"select_like", is_like ? "1" : "0"},
        {"multiply", std::to_string(coin_number)},
        {"csrf", access_key},
    };

    printf("[add coin] aid: %d; coin: %d; select_like: %d;\n", aid, coin_number,
           is_like);
    HTTP::postResultAsync(Api::CoinWeb, {}, payload, callback, error);
}

void BilibiliClient::get_coin_exp(const std::function<void(int)>& callback,
                                  const ErrorCallback& error) {
    HTTP::getResultAsync<int>(Api::CoinExp, {}, callback, error);
}

void BilibiliClient::add_resource(const std::string& access_key, int rid,
                                  int type, const std::string& add_ids,
                                  const std::string& del_ids,
                                  const std::function<void()>& callback,
                                  const ErrorCallback& error) {
    cpr::Payload payload = {
        {"rid", std::to_string(rid)}, {"type", std::to_string(type)},
        {"add_media_ids", add_ids},   {"del_media_ids", del_ids},
        {"csrf", access_key},
    };
    HTTP::postResultAsync(Api::CollectionVideoListSave, {}, payload, callback,
                          error);
}

void BilibiliClient::triple_like(const std::string& access_key, int aid,
                                 const std::function<void()>& callback,
                                 const ErrorCallback& error) {
    cpr::Payload payload = {
        {"aid", std::to_string(aid)},
        {"csrf", access_key},
    };
    HTTP::postResultAsync(Api::TripleWeb, {}, payload, callback, error);
}

void BilibiliClient::follow_up(const std::string& access_key,
                               const std::string& mid, bool follow,
                               const std::function<void()>& callback,
                               const ErrorCallback& error) {
    cpr::Payload payload = {
        {"fid", mid},
        {"act", follow ? "1" : "2"},
        {"csrf", access_key},
    };
    HTTP::postResultAsync(Api::Follow, {}, payload, callback, error);
}

}  // namespace bilibili