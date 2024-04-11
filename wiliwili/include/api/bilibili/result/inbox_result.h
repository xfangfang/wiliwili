#pragma once

#include <nlohmann/json.hpp>

namespace bilibili {

class UserCardResult {
public:
    unsigned int mid;
    std::string name;
    std::string face;
    int level;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserCardResult, mid, name, face, level);

class MsgFeedUser {
public:
    unsigned int mid;
    std::string nickname;
    std::string avatar;
    bool follow;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MsgFeedUser, mid, nickname, avatar, follow);

class MsgFeedItem {
public:
    unsigned int subject_id;
    unsigned int source_id;
    std::string type;
    std::string title;
    std::string image;
    std::string source_content;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MsgFeedItem, subject_id, source_id, type, title, image, source_content);

class FeedReplyResult {
public:
    uint64_t id;
    MsgFeedUser user;
    MsgFeedItem item;
    time_t reply_time;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FeedReplyResult, id, user, item, reply_time);

typedef std::vector<FeedReplyResult> FeedReplyListResult;

class MsgFeedCursor {
public:
    bool is_end;
    uint64_t id;
    time_t time;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MsgFeedCursor, is_end, id, time);

class FeedReplyResultWrapper {
public:
    MsgFeedCursor cursor;
    FeedReplyListResult items;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, FeedReplyResultWrapper& nlohmann_json_t) {
    if (nlohmann_json_j.contains("items") && nlohmann_json_j.at("items").is_array()) {
        nlohmann_json_j.at("items").get_to(nlohmann_json_t.items);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, cursor));
}

class FeedAtResult {
public:
    uint64_t id;
    MsgFeedUser user;
    MsgFeedItem item;
    time_t at_time;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FeedAtResult, id, user, item, at_time);

typedef std::vector<FeedAtResult> FeedAtListResult;

class FeedAtResultWrapper {
public:
    MsgFeedCursor cursor;
    FeedAtListResult items;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, FeedAtResultWrapper& nlohmann_json_t) {
    if (nlohmann_json_j.contains("items") && nlohmann_json_j.at("items").is_array()) {
        nlohmann_json_j.at("items").get_to(nlohmann_json_t.items);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, cursor));
}

class FeedLikeResult {
public:
    uint64_t id;
    std::vector<MsgFeedUser> users;
    MsgFeedItem item;
    time_t like_time;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FeedLikeResult, id, users, item, like_time);

typedef std::vector<FeedLikeResult> FeedLikeListResult;

class FeedLikeResultTotal {
public:
    MsgFeedCursor cursor;
    FeedLikeListResult items;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FeedLikeResultTotal, cursor, items);

class FeedLikeResultWrapper {
public:
    FeedLikeResultTotal total;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FeedLikeResultWrapper, total);

class InboxMessageResult {
public:
    uint64_t sender_uid, receiver_id;
    uint64_t msg_seqno;
    std::vector<uint64_t> at_uids;
    int receiver_type, msg_type;
    std::string content;
    time_t timestamp;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(InboxMessageResult, sender_uid, receiver_id, receiver_type, msg_seqno, at_uids,
                                   msg_type, content, timestamp);

typedef std::vector<InboxMessageResult> InboxMessageListResult;

class InboxEmote {
public:
    std::string text;
    std::string url;
    int size = 1;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(InboxEmote, text, url);

class InboxMessageResultWrapper {
public:
    InboxMessageListResult messages;
    std::vector<InboxEmote> e_infos;
    int has_more;
    uint64_t max_seqno;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, InboxMessageResultWrapper& nlohmann_json_t) {
    if (nlohmann_json_j.contains("messages") && nlohmann_json_j.at("messages").is_array()) {
        nlohmann_json_j.at("messages").get_to(nlohmann_json_t.messages);
    }
    if (nlohmann_json_j.contains("e_infos") && nlohmann_json_j.at("e_infos").is_array()) {
        nlohmann_json_j.at("e_infos").get_to(nlohmann_json_t.e_infos);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, has_more, max_seqno));
}

class InboxAccountInfo {
public:
    std::string name;
    std::string pic_url;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(InboxAccountInfo, name, pic_url);

class InboxChatResult {
public:
    uint64_t talker_id;
    int session_type;
    time_t session_ts;
    InboxAccountInfo account_info;
    InboxMessageResult last_msg;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(InboxChatResult, talker_id, session_type, session_ts, account_info,
                                                last_msg);

typedef std::vector<InboxChatResult> InboxChatListResult;

class InboxChatResultWrapper {
public:
    InboxChatListResult session_list;
    int has_more;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, InboxChatResultWrapper& nlohmann_json_t) {
    if (nlohmann_json_j.contains("session_list") && nlohmann_json_j.at("session_list").is_array()) {
        nlohmann_json_j.at("session_list").get_to(nlohmann_json_t.session_list);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, has_more));
}

}  // namespace bilibili