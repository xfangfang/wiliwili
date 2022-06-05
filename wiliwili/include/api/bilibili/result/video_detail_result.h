//
// Created by fang on 2022/5/2.
//

#pragma once

#include "nlohmann/json.hpp"
#include "user_result.h"

using namespace std;

namespace bilibili {

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
        int duration;//时长
        UserSimpleResult owner;

    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoDetailResult, bvid, aid, videos, owner, title, pic, desc, pubdate);


    class VideoDetailPage {
    public:
        int cid;
        int page; // 分p的序号
        int duration; // 视频长度，单位秒
        string part; //标题
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoDetailPage, cid, page, duration, part);


    typedef std::vector<VideoDetailPage> VideoDetailPageListResult;


    class VideoDUrl {
    public:
        int order;
        int length;
        int size;
        std::string url;
        std::vector<std::string> backup_url;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoDUrl, order, length, size, url, backup_url);


    class VideoUrlResult {
    public:
        int quality; //当前画质
        int timelength; // 视频长度
        std::vector<std::string> accept_description; //可供选择的分辨率
        std::vector<int> accept_quality; //可供选择的分辨率编号
        std::vector<VideoDUrl> durl;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoUrlResult, quality, timelength, accept_description, accept_quality, durl);
}
