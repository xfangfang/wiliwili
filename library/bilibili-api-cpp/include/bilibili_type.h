#pragma once

#include <string>
#include <vector>

namespace bilibili {

    enum VideoType{
        ALL=160,
        HAND_MAKE=161,
        PAINT=162
    };

    class Video{
        public:
            std::string aid;
            std::string bvid;
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

    };

    class VideoList{
        public:
            int code;
            std::string message;
            std::vector<Video> data;
    };

}