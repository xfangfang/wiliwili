#pragma once

#include <string>
#include <vector>

namespace bilibili {

    enum VideoType{
        ALL=160,
        HAND_MAKE=161,
        PAINT=162
    };

    class User{
        public:
            int mid;
            std::string name;
            std::string face;
    };

    class Video{
        public:
            int aid;
            std::string bvid;
            int cid;
            std::string videotype;
            std::string title;
            std::string subtitle;
            std::string pic;
            std::string author;
            std::string create;
            std::string description;
            std::string duration;
            int play;
            int review;
            int video_review;
            int favorites;
            int mid; //up's uid
            int coins;
            bool badgepay;
            int pts;
            User owner;

    };

    //  A video page consists of one or more video file.
    //  We call it VideoSlice
    class VideoSlice{
        public:
            int order;          // video order
            int length;         // video length(ms)
            int size;           // video file size(byte)
            std::string url;    // video file url
    };

    //  A video consists of one or more video page.
    class VideoPage{
        public:
            std::vector<VideoSlice> durl;   // list of video slice
            std::vector<int> accept_quality;
            std::vector<std::string> accept_description;
            int quality;                    //current quality
            int cid;
    };



    class Page{
        public:
            int num;
            int size;
            int count;
    };

    class Data{
        public:
            Page page;
            std::vector<Video> archives;
    };

    class VideoList{
        public:
            int code;
            std::string message;
            Data data;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(User, mid, name, face);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Video, bvid, cid, title, pic, owner);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Page, num, size, count);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Data, page, archives);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoList, code, message, data);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoSlice, order, length, size, url);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoPage, durl, accept_quality, accept_description, quality);

}