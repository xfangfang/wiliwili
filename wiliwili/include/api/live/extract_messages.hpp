//
// Created by maye174 on 2023/4/6.
//

#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <cstdint>

typedef enum {
    watched_change,    //在线人数更新，xx人看过
    online_cnt,        //当前在线人数
    online_v2,         //待确定，似乎是当前在线高能用户
    danmaku,           //弹幕
    super_chat,        //sc，醒目留言
    initeract_word,    //普通用户的进场消息
    entry_effect,      //舰长进场消息
    send_gift,         //发送的礼物
    combo_send,        //连击礼物
    like_info_update,  //人气值(助力值)
    like_info_click,   //给主播点赞
} message_t;           //还有上舰长，续费信息什么的，暂时不加

typedef struct {
    //b站会直接传3个值{"num":23592,"text_large":"2.3万人看过","text_small":"2.3万"}
    //真的浪费
    int num;
} watched_change_t;

typedef struct {
    //代表在线人数
    int count;
} online_cnt_t;

typedef struct {
    //头像
    void* face_photo[10];
    //名字
    char* user_name[10];
    //暂时先写这俩
} online_v2_t;

//弹幕类型，内容太多，暂时写这么多
//
//很多重复内容，感觉不是同一批人写的，或者可能b站想换协议，
typedef struct {
    //用户名字
    char* user_name;
    //用户名字颜色，一般为舰长以上有，即vip等级1以上
    char* user_name_color;
    //弹幕内容
    char* dan;
    //粉丝牌子名字
    char* fan_medal_name;
    //粉丝牌子对应主播名字
    char* fan_medal_liveuser_name;
    //用户uid
    int user_uid;
    //弹幕颜色
    int dan_color;
    //牌子对应直播间号
    int fan_medal_roomid;
    //牌子字体颜色 目前应该都是白色
    int fan_medal_font_color;
    //牌子边框颜色
    int fan_medal_border_color;
    //牌子开始颜色
    int fan_medal_start_color;
    //牌子结束颜色
    int fan_medal_end_color;
    //牌子对应主播uid
    int fan_medal_liveuser_uid;
    //是否为表情
    uint8_t is_emoticon;
    //弹幕类型
    uint8_t dan_type;
    //弹幕尺寸
    uint8_t dan_size;
    //用户直播等级 0~60
    //待确定，有的用户发的消息为0级
    uint8_t user_level;
    //用户在本房VIP等级，0~3，不是vip为0
    uint8_t user_vip_level;
    //牌子等级
    uint8_t fan_medal_level;
    //牌子对应直播间VIP等级，0~3，不是vip为0
    uint8_t fan_medal_vip_level;
    //是否为房管
    uint8_t is_guard;
    //荣耀值，就是用户名和牌子前面那个带蓝色框子的数字 暂时不用
    uint8_t glory_v;
} danmaku_t;  //Maye174: 为了对齐内存，乱序排

danmaku_t* danmaku_t_init();
danmaku_t* danmaku_t_copy(const danmaku_t* p);
void danmaku_t_free(const danmaku_t* p);

typedef struct {
    //todo
} super_chat_t;

//这里b站的接口命名更混乱，感觉b站后面会逐步换掉
typedef struct {
    //用户名字
    char* user_name;
    //牌子字体颜色 目前应该都是白色
    int fan_medal_font_color;
    //牌子边框颜色
    int fan_medal_border_color;
    //牌子开始颜色
    int fan_medal_start_color;
    //牌子结束颜色
    int fan_medal_end_color;
    //牌子所在主播uid
    int fan_medal_uid;
    //牌子等级
    uint8_t fan_medal_level;
} initeract_word_t;

initeract_word_t* initeract_word_t_init();

typedef struct {
    //todo
} send_gift_t;

typedef struct {
    //todo
} combo_send_t;

typedef struct {
    //人气值，但是这个值叫click_count，我有点搞不清
    int count;
} like_info_update_t;

typedef struct {
    //todo
} like_info_click_t;

typedef struct {
    message_t type;
    void* ptr;
} live_t;

std::vector<live_t> extract_messages(const std::vector<std::string>& messages);