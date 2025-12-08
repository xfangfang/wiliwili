//
// Created by fang on 2023/11/14.
//

#include <borealis/core/thread.hpp>
#include <borealis/core/logger.hpp>

#include "presenter/video_detail.hpp"
#include "presenter/live_data.hpp"
#include "utils/config_helper.hpp"
#include "bilibili.h"
#include "api/bilibili/util/http.hpp"

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

void LiveDataRequest::requestLiveAnchorInfo(int roomid) {
    if (liveRoomPlayInfo.uid > 0) {
        // 从liveRoomPlayInfo中获取的数据
        std::string uid = std::to_string(liveRoomPlayInfo.uid);
        brls::Logger::debug("get live anchor info, uid: {}", uid);
        
        // 调用已有的用户卡片API
        std::vector<std::string> uids = {uid};
        ASYNC_RETAIN
        BILI::get_user_cards(
            uids,
            [ASYNC_TOKEN](const bilibili::UserCardListResult& result) {
                if (!result.empty()) {
                    std::string face = result[0].face;
                    std::string uname = result[0].name;
                    
                    brls::sync([ASYNC_TOKEN, face, uname]() {
                        ASYNC_RELEASE
                        this->onAnchorInfo(face, uname);
                    });
                } else {
                    brls::Logger::error("get live anchor info: empty result");
                    ASYNC_RELEASE
                }
            },
            [ASYNC_TOKEN](BILI_ERR) {
                brls::Logger::error("get live anchor info: {}", error);
                brls::sync([ASYNC_TOKEN]() {
                    ASYNC_RELEASE
                    this->onAnchorInfo("", "主播");
                });
            }
        );
    } else {
        brls::Logger::error("get live anchor info: no uid available");
        this->onAnchorInfo("", "主播");
    }
}

// 新增：获取主播称号信息
void LiveDataRequest::requestLiveAnchorTitle(int roomid) {
    brls::Logger::debug("requesting anchor title for room: {}", roomid);
    
    ASYNC_RETAIN
    cpr::Parameters params = {{"roomid", std::to_string(roomid)}};
    cpr::Url url = cpr::Url{"https://api.live.bilibili.com/live_user/v1/UserInfo/get_anchor_in_room"};
    
    auto session = bilibili::HTTP::createSession();
    session->SetUrl(url);
    session->SetParameters(params);
    
    session->GetCallback<>(
        [ASYNC_TOKEN](const cpr::Response& r) {
            if (r.status_code == 200) {
                try {
                    auto json = nlohmann::json::parse(r.text);
                    std::string title = "";
                    
                    if (json.contains("data") && json["data"].contains("info") && 
                        json["data"]["info"].contains("official_verify") && 
                        json["data"]["info"]["official_verify"].contains("desc")) {
                        title = json["data"]["info"]["official_verify"]["desc"].get<std::string>();
                    }
                    
                    brls::sync([ASYNC_TOKEN, title]() {
                        ASYNC_RELEASE
                        this->onAnchorTitleInfo(title);
                    });
                } catch (const std::exception& e) {
                    brls::Logger::error("Failed to parse anchor title data: {}", e.what());
                    ASYNC_RELEASE
                }
            } else {
                brls::Logger::error("Failed to get anchor title: HTTP {}", r.status_code);
                ASYNC_RELEASE
            }
        });
}