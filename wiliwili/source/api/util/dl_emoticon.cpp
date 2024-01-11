//
//Created by Maye174 on 2023/8/7.
//

#include <nlohmann/json.hpp>
#include <curl/curl.h>

#include <cstdint>
#include <string>
#include <vector>

#include "live/dl_emoticon.hpp"

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::vector<char> *)userp)
        ->insert(((std::vector<char> *)userp)->end(), (char *)contents, (char *)contents + size * nmemb);
    return size * nmemb;
}

static void download(const std::string &url, std::vector<uint8_t> &data) {
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    curl_easy_perform(curl);

    curl_easy_cleanup(curl);
}

static void to_url(int room_id, std::vector<std::string> &names, std::vector<std::string> &urls) {
    std::string u = "https://api.live.bilibili.com/xlive/web-ucenter/v2/emoticon/GetEmoticons?platform=pc&room_id=";
    std::vector<uint8_t> data;
    download(u + std::to_string(room_id), data);

    nlohmann::json _json = nlohmann::json::parse(std::string{data.begin(), data.end()});

    auto it = _json["data"]["data"];

    if (it.is_array()) {
        int n = it.size();

        for (int i = 0; i < n; ++i) {
            if (!it.is_array()) break;

            int e_n   = it[i].size();
            auto e_it = it[i]["emoticons"];

            if (!e_it.is_array()) break;

            for (int j = 0; j < e_n; ++j) {
                auto em_name = e_it[i]["emoji"];

                if (!em_name.is_string()) break;

                auto em_url = e_it[j]["url"];

                if (!em_url.is_string()) break;

                names.emplace_back(em_name.get_ref<const std::string &>());
                urls.emplace_back(em_url.get_ref<const std::string &>());
            }
        }
    }
}

lmp dl_emoticon(int room_id) {
    lmp ret;

    std::vector<std::string> names;
    std::vector<std::string> urls;

    to_url(room_id, names, urls);

    int n = names.size();
    for (int i = 0; i < n; ++i) {
        std::vector<uint8_t> data;
        download(urls[i], data);
        ret[names[i]] = std::move(data);
    }

    return ret;
}
