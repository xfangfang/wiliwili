#pragma once

#include <string>
#include <vector>

namespace bilibili {

    enum LoginInfo{
        SUCCESS = 1,
        OAUTH_KEY_ERROR = -1,
        OAUTH_KEY_TIMEOUT = -2,
        NEED_SCAN = -4,
        NEED_CONFIRM = -5
    };

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

    class UserDetail{
        public:
            int mid = -1;
            int level;
            int following;
            int follower;
            std::string name;
            std::string face;
            std::string sex;
            std::string sign;
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

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserDetail, mid, name, face, following, follower, sex, sign, level);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(User, mid, name, face);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Video, bvid, cid, title, pic, owner);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Page, num, size, count);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Data, page, archives);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoList, code, message, data);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoSlice, order, length, size, url);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoPage, durl, accept_quality, accept_description, quality);


    namespace space_user_videos {
        class Video{
            public:
                int aid;
                std::string bvid;
                std::string title;
                std::string subtitle;
                std::string pic;
                std::string author;
                std::string description;
                std::string length;
                int play;
                int video_review;
                int mid;
                int created;
        };
        class List {
            public:
                std::vector<Video> vlist;
        };
        class Page {
            public:
                int pn;
                int ps;
                int count;
        };
        class VideoList {
            public:
                Page page;
                List list;
                bool has_more = false;
                std::vector<Video>& getList(){
                    return this->list.vlist;
                }
        };

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Video, bvid, title, pic, play, created, length);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(List, vlist);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Page, pn, ps, count);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoList, page, list);
    }

    namespace space_user_collections {
        class Collection{
            public:
                int id;
                int mid;
                int media_count;
                std::string title;
                std::string cover;
                User upper;
        };
        class CollectionList{
            public:
                int count = 0;
                std::vector<Collection> list;
                bool has_more = false;
                std::vector<Collection>& getList(){
                    return this->list;
                }
        };
        class CollectionVideo{
            public:
                std::string bvid;
                std::string title;
                std::string intro;
                std::string cover;
                User upper;
                int fav_time;
        };
        class CollectionInfo{
            public:
                int id;
                int mid;
                int media_count;
                std::string title;
                std::string cover;
                User upper;
        };
        class CollectionDetail{
            public:
                CollectionInfo info;
                std::vector<CollectionVideo> medias;
                std::vector<CollectionVideo>& getList(){
                    return this->medias;
                }
        };

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Collection, id, mid, media_count, title, upper, cover);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CollectionList, count, list, has_more);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CollectionVideo, bvid, title, intro, cover, upper, fav_time);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CollectionInfo, id, mid, media_count, title, cover, upper);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CollectionDetail, info, medias);
    }
}