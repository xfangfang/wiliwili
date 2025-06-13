//
// Created by maye174 on 2023/4/6.
//

#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <cstdint>
#include <memory>

// 消息类型枚举
enum class MessageType {
    WATCHED_CHANGE,    //在线人数更新，xx人看过
    ONLINE_CNT,        //当前在线人数
    ONLINE_V2,         //待确定，似乎是当前在线高能用户
    DANMAKU,           //弹幕
    SUPER_CHAT,        //sc，醒目留言
    INITERACT_WORD,    //普通用户的进场消息
    ENTRY_EFFECT,      //舰长进场消息
    SEND_GIFT,         //发送的礼物
    COMBO_SEND,        //连击礼物
    LIKE_INFO_UPDATE,  //人气值(助力值)
    LIKE_INFO_CLICK,   //给主播点赞
};            //还有上舰长，续费信息什么的，暂时不加

namespace message {

// 看过人数更新
class WatchedChange {
public:
    WatchedChange(int num = 0) : num(num) {}
    
    // 人数
    int num;
};

// 在线人数
class OnlineCount {
public:
    OnlineCount(int count = 0) : count(count) {}
    
    // 在线人数
    int count;
};

// 在线高能用户
class OnlineV2 {
public:
    // todo: 实现在线高能用户
};

// 弹幕类
class Danmaku {
public:
    Danmaku();
    Danmaku(const Danmaku& other);
    ~Danmaku() = default;
    
    // 用户名
    std::string user_name;
    // 用户名颜色
    std::string user_name_color;
    // 弹幕内容
    std::string dan;
    // 粉丝牌名
    std::string fan_medal_name;
    // 粉丝牌主播名
    std::string fan_medal_liveuser_name;
    
    // 用户uid
    int user_uid = 0;
    // 弹幕颜色
    int dan_color = 16777215; // 默认白色
    // 粉丝牌直播间号
    int fan_medal_roomid = 0;
    // 粉丝牌字体颜色
    int fan_medal_font_color = 0;
    // 粉丝牌边框颜色
    int fan_medal_border_color = 0;
    // 粉丝牌开始颜色
    int fan_medal_start_color = 0;
    // 粉丝牌结束颜色
    int fan_medal_end_color = 0;
    // 粉丝牌对应主播uid
    int fan_medal_liveuser_uid = 0;
    
    // 是否为表情
    uint8_t is_emoticon = 0;
    // 弹幕类型: 1/2/3普通, 4底部, 5顶部
    uint8_t dan_type = 1;
    // 弹幕尺寸
    uint8_t dan_size = 25;
    // 用户直播等级 0~60
    uint8_t user_level = 1;
    // 用户VIP等级 0~3
    uint8_t user_vip_level = 0;
    // 粉丝牌等级
    uint8_t fan_medal_level = 0;
    // 粉丝牌VIP等级 0~3
    uint8_t fan_medal_vip_level = 0;
    // 是否为房管
    uint8_t is_guard = 0;
    // 荣耀值
    uint8_t glory_v = 0;
};

// 超级留言
class SuperChat {
public:
    SuperChat();
    SuperChat(const SuperChat& other);
    ~SuperChat() = default;
    
    // 用户名
    std::string user_name;
    // 用户头像URL
    std::string user_face;
    // SC内容
    std::string message;
    // SC字体颜色
    std::string message_font_color;
    // SC背景颜色
    std::string background_color;
    // SC背景颜色(开始)
    std::string background_color_start;
    // SC背景颜色(结束)
    std::string background_color_end;
    // SC价格背景颜色
    std::string background_price_color;
    // 粉丝牌名字
    std::string fan_medal_name;
    // 粉丝牌主播名字
    std::string fan_medal_liveuser_name;
    
    // 用户UID
    int user_uid = 0;
    // SC价格(金额)
    int price = 0;
    // SC持续时间(秒)
    int time = 0;
    // SC开始时间戳
    int64_t start_time = 0;
    // SC结束时间戳
    int64_t end_time = 0;
    // 粉丝牌等级
    uint8_t fan_medal_level = 0;
};

// 互动消息
class InteractWord {
public:
    InteractWord();
    ~InteractWord() = default;
    
    // 用户名
    std::string user_name;
    // 粉丝牌字体颜色
    int fan_medal_font_color = 0;
    // 粉丝牌边框颜色
    int fan_medal_border_color = 0;
    // 粉丝牌开始颜色
    int fan_medal_start_color = 0;
    // 粉丝牌结束颜色
    int fan_medal_end_color = 0;
    // 粉丝牌主播uid
    int fan_medal_uid = 0;
    // 粉丝牌等级
    uint8_t fan_medal_level = 0;
};

// 礼物
class SendGift {
public:
    // todo: 实现礼物
};

// 连击礼物
class ComboSend {
public:
    // todo: 实现连击礼物  
};

// 人气值更新
class LikeInfoUpdate {
public:
    LikeInfoUpdate(int count = 0) : count(count) {}
    
    // 人气值
    int count;
};

// 点赞
class LikeInfoClick {
public:
    // todo: 实现点赞
};

// 直播消息基类
class LiveMessage {
public:
    virtual ~LiveMessage() = default;
    MessageType type;
};

// 各种类型的直播消息
class LiveWatchedChange : public LiveMessage {
public:
    LiveWatchedChange(int num = 0) : data(std::make_shared<WatchedChange>(num)) {
        type = MessageType::WATCHED_CHANGE;
    }
    
    std::shared_ptr<WatchedChange> data;
};

class LiveDanmaku : public LiveMessage {
public:
    LiveDanmaku(std::shared_ptr<Danmaku> dan) : data(dan) {
        type = MessageType::DANMAKU;
    }
    
    std::shared_ptr<Danmaku> data;
};

class LiveSuperChat : public LiveMessage {
public:
    LiveSuperChat(std::shared_ptr<SuperChat> sc) : data(sc) {
        type = MessageType::SUPER_CHAT;
    }
    
    std::shared_ptr<SuperChat> data;
};

class LiveOnlineCount : public LiveMessage {
public:
    LiveOnlineCount(int count = 0) : data(std::make_shared<OnlineCount>(count)) {
        type = MessageType::ONLINE_CNT;
    }
    
    std::shared_ptr<OnlineCount> data;
};

class LiveInteractWord : public LiveMessage {
public:
    LiveInteractWord(std::shared_ptr<InteractWord> word) : data(word) {
        type = MessageType::INITERACT_WORD;
    }
    
    std::shared_ptr<InteractWord> data;
};

class LiveLikeInfoUpdate : public LiveMessage {
public:
    LiveLikeInfoUpdate(int count = 0) : data(std::make_shared<LikeInfoUpdate>(count)) {
        type = MessageType::LIKE_INFO_UPDATE;
    }
    
    std::shared_ptr<LikeInfoUpdate> data;
};

} // namespace message

// 解析消息并返回消息列表
std::vector<std::shared_ptr<message::LiveMessage>> extract_messages(const std::vector<std::string>& messages);