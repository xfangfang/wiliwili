//
// Created by fang on 2023/11/14.
//

#include <borealis/core/thread.hpp>
#include <borealis/core/logger.hpp>

#include "presenter/video_detail.hpp"
#include "presenter/live_data.hpp"
#include "utils/config_helper.hpp"
#include "bilibili.h"

void LiveDataRequest::requestData(int roomid) {
    ASYNC_RETAIN
    BILI::get_live_room_play_info(
        roomid, defaultQuality,
        [ASYNC_TOKEN](const auto& result) {
            liveRoomPlayInfo = result;
            qualityDescriptionMap.clear();
            for (auto& i : liveRoomPlayInfo.playurl_info.playurl.g_qn_desc) {
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
            brls::sync([ASYNC_TOKEN]() {
                ASYNC_RELEASE
                onLiveData(liveRoomPlayInfo);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                this->onError(error);
            });
        });

    reportHistory(roomid);
}

void LiveDataRequest::reportHistory(int roomid) {
    // 复用视频播放页面的标记
    if (!VideoDetail::REPORT_HISTORY) return;

    BILI::report_live_history(
        roomid, ProgramConfig::instance().getCSRF(),
        [roomid]() { brls::Logger::debug("report live history {}", roomid); },
        [](BILI_ERR) { brls::Logger::error("report live history: {}", error); });
}

void LiveDataRequest::requestPayLiveInfo(int roomid) {
    ASYNC_RETAIN
    BILI::get_live_pay_info(
        roomid,
        [ASYNC_TOKEN, roomid](const auto& payInfo) {
            brls::Logger::debug("get live pay info {}", payInfo.permission);
            if (payInfo.permission != 0) return;
            BILI::get_live_pay_link(
                roomid,
                [ASYNC_TOKEN, payInfo](const auto& payLink) {
                    brls::Logger::debug("get live pay link {}", payLink.goods_link);
                    brls::sync([ASYNC_TOKEN, payInfo, payLink]() {
                        ASYNC_RELEASE
                        this->onNeedPay(payInfo.message, payLink.goods_link, payLink.start_time, payLink.end_time);
                    });
                },
                [ASYNC_TOKEN, payInfo](BILI_ERR) {
                    brls::Logger::error("get live pay link:", error);
                    brls::sync([ASYNC_TOKEN, payInfo]() {
                        ASYNC_RELEASE
                        this->onNeedPay(payInfo.message, "", "", "");
                    });
                });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            ASYNC_RELEASE
            brls::Logger::error("get live pay info: {}", error);
        });
}

void LiveDataRequest::requestLiveDanmakuToken(int roomid) {
    ASYNC_RETAIN
    BILI::get_live_danmaku_info(
        roomid,
        [ASYNC_TOKEN, roomid](const auto& info) {
            brls::sync([ASYNC_TOKEN, roomid, info]() {
                ASYNC_RELEASE
                this->onDanmakuInfo(roomid, info);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            ASYNC_RELEASE
            brls::Logger::error("get live danmaku info: {}", error);
        });
}

std::string LiveDataRequest::getQualityDescription(int qn) {
    if (qualityDescriptionMap.count(qn) == 0) return "Unknown Quality " + std::to_string(qn);
    return qualityDescriptionMap[qn];
}