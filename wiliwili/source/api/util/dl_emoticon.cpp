//
//Created by Maye174 on 2023/8/7.
//

#include <nlohmann/json.hpp>
// #include <cpr/cpr.h>

#include <cstdint>
#include <string>
#include <vector>

#include "live/dl_emoticon.hpp"
#include <borealis/core/logger.hpp>
#include "utils/config_helper.hpp"
#include "bilibili/util/http.hpp"
#include "bilibili.h"

static void to_url(int room_id, std::vector<std::string> &names, std::vector<std::string> &urls) {
    std::string api_url = "https://api.live.bilibili.com/xlive/web-ucenter/v2/emoticon/GetEmoticons?platform=pc&room_id=" + std::to_string(room_id);
    std::vector<uint8_t> data;
    
    // 确保使用HTTP::createSession带上cookie
    auto session = bilibili::HTTP::createSession();
    session->SetUrl(api_url);
    
    // 添加必要的请求头
    session->SetHeader(cpr::Header{
        {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36"},
        {"Referer", "https://live.bilibili.com/" + std::to_string(room_id)},
        {"Accept", "application/json, text/plain, */*"}
    });
    
    // 添加Buvid3 cookie如果存在
    std::string buvid3 = ProgramConfig::instance().getBuvid3();
    if (!buvid3.empty()) {
        brls::Logger::debug("使用Buvid3: {}", buvid3);
    } else {
        buvid3 = bilibili::BilibiliClient::genRandomBuvid3();
        brls::Logger::debug("生成新的Buvid3: {}", buvid3);
    }
    
    // 执行请求
    auto r = session->Get();
    
    if (r.status_code != 200) {
        brls::Logger::error("下载表情列表失败: room_id={}, 状态码: {}", room_id, r.status_code);
        return;
    }
    
    std::string json_str = r.text;
    brls::Logger::debug("表情API返回: {}", json_str.substr(0, 100) + (json_str.length() > 100 ? "..." : ""));
    
    try {
        nlohmann::json _json = nlohmann::json::parse(json_str);
        
        if (_json["code"].get<int>() != 0) {
            brls::Logger::error("表情API返回错误码: {}, 消息: {}", _json["code"].get<int>(), _json["message"].get<std::string>());
            return;
        }
        
        auto data_obj = _json["data"];
        if (!data_obj.contains("data") || !data_obj["data"].is_array()) {
            brls::Logger::error("表情API返回格式异常");
            return;
        }
        
        auto packages = data_obj["data"];
        brls::Logger::debug("发现 {} 个表情包", packages.size());
        
        for (auto& package : packages) {
            if (!package.contains("emoticons") || !package["emoticons"].is_array()) {
                continue;
            }
            
            for (auto& emoticon : package["emoticons"]) {
                if (!emoticon.contains("emoji") || !emoticon.contains("url")) {
                    continue;
                }
                
                std::string emoji = emoticon["emoji"].get<std::string>();
                std::string url = emoticon["url"].get<std::string>();
                
                names.push_back(emoji);
                urls.push_back(url);
                brls::Logger::debug("发现表情: {} -> {}", emoji, url);
            }
        }
    } catch (const std::exception& e) {
        brls::Logger::error("解析表情列表失败: {}", e.what());
    }
}

lmp dl_emoticon(int room_id) {
    lmp ret;
    brls::Logger::debug("开始获取直播间 {} 的表情包URL", room_id);

    std::vector<std::string> names;
    std::vector<std::string> urls;

    to_url(room_id, names, urls);
    
    brls::Logger::debug("找到 {} 个表情", names.size());

    // 保存表情名称和URL
    for (size_t i = 0; i < names.size(); ++i) {
        brls::Logger::debug("保存表情URL {}/{}: {} -> {}", i+1, names.size(), names[i], urls[i]);
        ret[names[i]] = urls[i];
    }
    
    brls::Logger::debug("成功获取 {} 个表情包URL", ret.size());

    return ret;
}
