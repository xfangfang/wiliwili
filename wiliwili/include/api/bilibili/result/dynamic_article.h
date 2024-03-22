//
// Created by fang on 2022/8/18.
//

#pragma once

#include <variant>

#include "bilibili/util/json.hpp"
#include "user_result.h"
#include "home_result.h"

namespace bilibili {

class DynamicArticleModuleNone {};

/// 动态作者
class DynamicArticleModuleAuthor {
public:
    std::string pub_text;
    UserSimpleResult user;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DynamicArticleModuleAuthor, pub_text, user);

/// 动态转发回复点赞
class DynamicArticleModuleStateComment {
public:
    std::string comment_id;
    int comment_type{}, count{}, type{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DynamicArticleModuleStateComment, count, type, comment_id, comment_type);

class DynamicArticleModuleStateForward {
public:
    int count{}, type{};
    std::string forbidden_msg; // 无法转发时的提示语
    bool is_forbidden{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DynamicArticleModuleStateForward, count, type, forbidden_msg, is_forbidden);

class DynamicArticleModuleStateLike {
public:
    int count{}, type{};
    bool like_state{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DynamicArticleModuleStateLike, count, type, like_state);

class DynamicArticleModuleState {
public:
    DynamicArticleModuleStateComment comment;
    DynamicArticleModuleStateForward forward;
    DynamicArticleModuleStateLike like;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DynamicArticleModuleState, comment, forward, like);

/// 动态文本
class DynamicArticleModuleDesc {
public:
    // todo: 富文本
    std::string text;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DynamicArticleModuleDesc, text);

/// 动态视频
class DynamicArticleModuleArchive {
public:
    std::string aid, bvid, cover, title, duration_text;
    std::string desc;     // 可能为 null
    unsigned int epid{};  // 可能为 null
    int type{};           // 1: UGC
    VideoSimpleStateResultV2 stat;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DynamicArticleModuleArchive, aid, bvid, cover, title, duration_text, type, stat,
                                   desc, epid);

/// 动态图片
class DynamicArticleModuleDrawImage {
public:
    // todo: tags
    int width, height;
    //    float size;
    std::string src;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DynamicArticleModuleDrawImage, src, width, height);

class DynamicArticleModuleDraw {
public:
    std::vector<DynamicArticleModuleDrawImage> items{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DynamicArticleModuleDraw, items);

/// 动态话题
class DynamicArticleModuleTopic{
public:
    int id{};
    std::string jump_url, name;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DynamicArticleModuleTopic, id, jump_url, name);

/// 动态空白
class DynamicArticleModuleNull {
public:
    std::string text; // 动态失效具体的原因
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DynamicArticleModuleNull, text);

enum class DynamicArticleModuleType {
    MODULE_TYPE_NONE = 0,
    MODULE_TYPE_AUTHOR,
    MODULE_TYPE_DESC,
    MODULE_TYPE_DATA,
    MODULE_TYPE_STAT,
    MODULE_TYPE_TOPIC,
    MODULE_TYPE_NULL,  // 转发的动态已失效
};

enum class DynamicArticleModuleDataType {
    MODULE_TYPE_NONE = 0,
    MODULE_TYPE_VIDEO,
    MODULE_TYPE_IMAGE,
    MODULE_TYPE_FORWARD,
};

/// 动态模块定义
#define DynamicArticleModuleResult_DELC                                                                         \
    class DynamicArticleModuleResult {                                                                          \
    public:                                                                                                     \
        std::string module_type;                                                                                \
        std::variant<DynamicArticleModuleNone, DynamicArticleModuleAuthor, DynamicArticleModuleDesc,            \
                     DynamicArticleModuleData, DynamicArticleModuleState, DynamicArticleModuleTopic,            \
                     DynamicArticleModuleNull>                                                                  \
            data;                                                                                               \
    };                                                                                                          \
    inline void from_json(const nlohmann::json& nlohmann_json_j, DynamicArticleModuleResult& nlohmann_json_t) { \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, module_type));                             \
        if (nlohmann_json_t.module_type == "MODULE_TYPE_AUTHOR") {                                              \
            nlohmann_json_t.data = nlohmann_json_j.at("module_author").get<DynamicArticleModuleAuthor>();       \
        } else if (nlohmann_json_t.module_type == "MODULE_TYPE_DYNAMIC") {                                      \
            nlohmann_json_t.data = nlohmann_json_j.at("module_dynamic").get<DynamicArticleModuleData>();        \
        } else if (nlohmann_json_t.module_type == "MODULE_TYPE_STAT") {                                         \
            nlohmann_json_t.data = nlohmann_json_j.at("module_stat").get<DynamicArticleModuleState>();          \
        } else if (nlohmann_json_t.module_type == "MODULE_TYPE_DESC") {                                         \
            nlohmann_json_t.data = nlohmann_json_j.at("module_desc").get<DynamicArticleModuleDesc>();           \
        } else if (nlohmann_json_t.module_type == "MODULE_TYPE_TOPIC") {                                        \
            nlohmann_json_t.data = nlohmann_json_j.at("module_topic").get<DynamicArticleModuleTopic>();          \
        } else if (nlohmann_json_t.module_type == "MODULE_TYPE_ITEM_NULL") {                                    \
            nlohmann_json_t.data = nlohmann_json_j.at("module_item_null").get<DynamicArticleModuleNull>();      \
        } else {                                                                                                \
            printf("unknown module type: %s\n", nlohmann_json_t.module_type.c_str());                           \
        }                                                                                                       \
    }

/// 动态定义
/// type: DYNAMIC_TYPE_AV: 视频; DYNAMIC_TYPE_DRAW: 图文; DYNAMIC_TYPE_FORWARD: 转发
#define DynamicArticleResult_DELC                                                                         \
    class DynamicArticleResult {                                                                          \
    public:                                                                                               \
        bool visible{};                                                                                   \
        std::string id_str;                                                                               \
        std::vector<DynamicArticleModuleResult> modules;                                                  \
        std::string type;                                                                                 \
    };                                                                                                    \
    inline void from_json(const nlohmann::json& nlohmann_json_j, DynamicArticleResult& nlohmann_json_t) { \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, visible, id_str, modules, type));    \
    }

/// 转发
namespace dynamic_forward {

class DynamicArticleModuleData {
public:
    std::string type;
    std::variant<DynamicArticleModuleNone, DynamicArticleModuleArchive, DynamicArticleModuleDraw> data;
};

inline void from_json(const nlohmann::json& nlohmann_json_j, DynamicArticleModuleData& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, type));
    if (nlohmann_json_t.type == "MDL_DYN_TYPE_ARCHIVE") {
        // 视频
        nlohmann_json_t.data = nlohmann_json_j.at("dyn_archive").get<DynamicArticleModuleArchive>();
    } else if (nlohmann_json_t.type == "MDL_DYN_TYPE_DRAW") {
        // 图片
        nlohmann_json_t.data = nlohmann_json_j.at("dyn_draw").get<DynamicArticleModuleDraw>();
    } else {
        printf("unknown module data type: %s\n", nlohmann_json_t.type.c_str());
    }
}

DynamicArticleModuleResult_DELC;

DynamicArticleResult_DELC;

}  // namespace dynamic_forward

// 图文转发
class DynamicArticleModuleForward {
public:
    dynamic_forward::DynamicArticleResult item;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, DynamicArticleModuleForward& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, item));
}

class DynamicArticleModuleData {
public:
    std::string type;
    std::variant<DynamicArticleModuleNone, DynamicArticleModuleArchive, DynamicArticleModuleDraw,
                 DynamicArticleModuleForward>
        data;
};

inline void from_json(const nlohmann::json& nlohmann_json_j, DynamicArticleModuleData& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, type));
    if (nlohmann_json_t.type == "MDL_DYN_TYPE_ARCHIVE") {
        // 视频
        nlohmann_json_t.data = nlohmann_json_j.at("dyn_archive").get<DynamicArticleModuleArchive>();
    } else if (nlohmann_json_t.type == "MDL_DYN_TYPE_DRAW") {
        // 图片
        nlohmann_json_t.data = nlohmann_json_j.at("dyn_draw").get<DynamicArticleModuleDraw>();
    } else if (nlohmann_json_t.type == "MDL_DYN_TYPE_FORWARD") {
        // 转发
        nlohmann_json_t.data = nlohmann_json_j.at("dyn_forward").get<DynamicArticleModuleForward>();
    } else {
        printf("unknown module data type: %s\n", nlohmann_json_t.type.c_str());
    }
}

DynamicArticleModuleResult_DELC;

DynamicArticleResult_DELC;

};  // namespace bilibili