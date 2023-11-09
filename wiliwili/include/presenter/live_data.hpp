//
// Created by fang on 2022/9/13.
//

#pragma once

#include "bilibili.h"
#include "bilibili/result/home_live_result.h"
#include "presenter/presenter.h"
#include "utils/config_helper.hpp"
#include "presenter/video_detail.hpp"

class LiveDataRequest : public Presenter {
public:
    virtual void onLiveData(const bilibili::LiveRoomPlayInfo& result) {}

    virtual void onError(const std::string& error) {}

    void requestData(int roomid) {
        ASYNC_RETAIN
        BILI::get_live_room_play_info(
            roomid, defaultQuality,
            [ASYNC_TOKEN](const auto& result) {
                ASYNC_RELEASE
                liveRoomPlayInfo = result;
                qualityDescriptionMap.clear();
                for (auto& i :
                     liveRoomPlayInfo.playurl_info.playurl.g_qn_desc) {
                    qualityDescriptionMap[i.qn] = i.desc;
                }

                // 选择第一个 protocol 的 第一个 format 的第一个 codec 作为播放源
                // protocol: http_stream / http_hls
                // format: flv / ts / fmp4
                // codec: avc / hevc / av1
                bilibili::LiveStream stream;
                for (auto& i : liveRoomPlayInfo.playurl_info.playurl.stream) {
                    stream = i;
                    break;
                }
                bilibili::LiveStreamFormat format;
                for (auto& i : stream.format) {
                    format = i;
                    break;
                }
                for (auto& i : format.codec) {
                    liveUrl = i;
                    break;
                }
                onLiveData(liveRoomPlayInfo);
            },
            [ASYNC_TOKEN](BILI_ERR) {
                ASYNC_RELEASE
                this->onError(error);
            });

        reportHistory(roomid);
    }

    void reportHistory(int roomid) {
        // 复用视频播放页面的标记
        if (!VideoDetail::REPORT_HISTORY) return;

        BILI::report_live_history(
            roomid, ProgramConfig::instance().getCSRF(),
            [roomid]() {
                brls::Logger::debug("report live history {}", roomid);
            },
            [this](BILI_ERR) { this->onError(error); });
    }

    std::string getQualityDescription(int qn) {
        if (qualityDescriptionMap.count(qn) == 0)
            return "Unknown Quality " + std::to_string(qn);
        return qualityDescriptionMap[qn];
    }

    static inline int defaultQuality = 0;
    bilibili::LiveRoomPlayInfo liveRoomPlayInfo;
    bilibili::LiveStreamFormatCodec liveUrl;
    std::unordered_map<int, std::string> qualityDescriptionMap;
};