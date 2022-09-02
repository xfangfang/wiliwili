//
// Created by fang on 2022/5/2.
//

#pragma once

#include "nlohmann/json.hpp"
#include "user_result.h"
#include "mine_result.h"

using namespace std;

namespace bilibili {

    class VideoDetailPage {
    public:
        int cid = 0;
        int page; // 分p的序号
        int duration; // 视频长度，单位秒
        int progress = -1; // 视频初始化的播放时间，用于加载历史记录
        string part; //标题
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoDetailPage, cid, page, duration, part);

    typedef std::vector<VideoDetailPage> VideoDetailPageListResult;

    class VideoDetailStat {
    public:
        uint aid;
        uint view;
        uint danmaku;
        uint favorite;
        uint coin;
        uint share;
        uint like;
        uint reply;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoDetailStat, aid, view, danmaku, favorite, coin, share, like, reply);

    class VideoDetailResult {
    public:
        string bvid;
        int aid;
        int videos; // 视频数量
        int tid; //分类ID
        int tname; //分类名称
        int copyright; //版权声明
        string pic; //封面图
        string title; //标题
        string desc; //简介
        int pubdate; //发布时间
        int ctime; //修改时间？
        int duration = 0;//时长
        UserSimpleResult owner;
        VideoDetailPageListResult pages;
        VideoDetailStat stat;
    };
    inline void from_json(const nlohmann::json& nlohmann_json_j, VideoDetailResult& nlohmann_json_t) {
        if(nlohmann_json_j.contains("videos")){
            nlohmann_json_j.at("videos").get_to(nlohmann_json_t.videos);
        }
        if(nlohmann_json_j.contains("pages")){
            nlohmann_json_j.at("pages").get_to(nlohmann_json_t.pages);
        }
        if(nlohmann_json_j.contains("duration")){
            nlohmann_json_j.at("duration").get_to(nlohmann_json_t.duration);
        }
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, bvid, aid, owner, title,
                                                 pic, desc, pubdate, stat, copyright));
    }
    inline void to_json(nlohmann::json& nlohmann_json_j, const VideoDetailResult& nlohmann_json_t) {
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, bvid, aid, owner, title,
                                                 pic, desc, pubdate, stat, copyright, videos, pages))
    }

    typedef std::vector<VideoDetailResult> VideoDetailListResult;


    class UserDetailResult{
    public:
        string mid, name, sex, rank, face, sign;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserDetailResult, mid, name, sex, rank, face, sign);

    class UserDetailResultWrapper {
    public:
        uint like_num, follower, article_count, archive_count;
        bool following;
        UserDetailResult card;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserDetailResultWrapper, like_num, follower, article_count, archive_count, following, card);

    class VideoDetailAllResult {
    public:
        VideoDetailResult View;
        UserDetailResultWrapper Card;
        VideoDetailListResult Related;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoDetailAllResult, View, Card, Related);


    class VideoDUrl {
    public:
        int order;
        int length;
        int size;
        std::string url;
        std::vector<std::string> backup_url;
    };
    inline void from_json(const nlohmann::json& nlohmann_json_j, VideoDUrl& nlohmann_json_t) {
        if(!nlohmann_json_j.at("backup_url").is_null()){
            nlohmann_json_j.at("backup_url").get_to(nlohmann_json_t.backup_url);
        }
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, order, length, size, url));
    }


    class VideoUrlResult {
    public:
        int quality; //当前画质
        int timelength; // 视频长度
        std::vector<std::string> accept_description; //可供选择的分辨率
        std::vector<int> accept_quality; //可供选择的分辨率编号
        std::vector<VideoDUrl> durl;
    };
    inline void from_json(const nlohmann::json& nlohmann_json_j, VideoUrlResult& nlohmann_json_t) {
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, quality, timelength, accept_description, accept_quality, durl));
    }

    // todo：up主精选评论

    typedef std::unordered_map<std::string, std::string> VideoCommentEmoteMap;

    class VideoCommentContent {
    public:
        VideoCommentEmoteMap emote;
        std::string message;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoCommentContent, message);

    class VideoCommentResult {
    public:
        int ctime;
        UserSimpleResult2 member;
        VideoCommentContent content;
        std::vector<VideoCommentResult> replies;
    };
    inline void from_json(const nlohmann::json& nlohmann_json_j, VideoCommentResult& nlohmann_json_t) {
        if(!nlohmann_json_j.at("replies").is_null()){
            nlohmann_json_j.at("replies").get_to(nlohmann_json_t.replies);
        }
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, ctime, member, content));
    }


    class VideoCommentCursor{
    public:
        int all_count = 0;
        int mode; // 3: 热门评论
        int next;
        int prev;
        bool is_end;
    };
    inline void from_json(const nlohmann::json& nlohmann_json_j, VideoCommentCursor& nlohmann_json_t) {
        if(nlohmann_json_j.contains("all_count")){
            nlohmann_json_j.at("all_count").get_to(nlohmann_json_t.all_count);
        }
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, mode, next, is_end, prev));
    }

    typedef vector<VideoCommentResult> VideoCommentListResult;

    class VideoCommentResultWrapper {
    public:
        VideoCommentCursor cursor;
        vector<VideoCommentResult> replies;
        vector<VideoCommentResult> top_replies;
    };
    inline void from_json(const nlohmann::json& nlohmann_json_j, VideoCommentResultWrapper& nlohmann_json_t) {
        if(!nlohmann_json_j.at("top_replies").is_null()){
            nlohmann_json_j.at("top_replies").get_to(nlohmann_json_t.top_replies);
        }
        if(!nlohmann_json_j.at("replies").is_null()){
            nlohmann_json_j.at("replies").get_to(nlohmann_json_t.replies);
        }
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, cursor));
    }


    class VideoRelation{
    public:
        bool attention, favorite, season_fav, like, dislike;
        int coin;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoRelation, attention, favorite, season_fav, like, dislike, coin);


    class VideoOnlineTotal{
    public:
        std::string total;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoOnlineTotal, total);


    class Video{
    public:
        int aid;
        std::string bvid;

        Video() = default;
        Video(const VideoDetailResult& r):aid(r.aid), bvid(r.bvid) {}
        Video(const UserUploadedVideoResult& r):aid(r.aid), bvid(r.bvid) {}
    };


}
