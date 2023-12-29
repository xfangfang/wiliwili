//
// Created by fang on 2023/3/3.
//

#pragma once

#include <unistd.h>
#include <nlohmann/json.hpp>
#include <borealis/core/singleton.hpp>
#include <borealis/core/util.hpp>
#include <utility>
#include <pystring.h>
#include "utils/config_helper.hpp"
#include "view/mpv_core.hpp"

class ShaderProfile {
public:
    std::string name;
    std::vector<std::string> shaders;

    std::string getShaderString() {
#ifdef _WIN32
        std::string separator = ";";
#else
        std::string separator = ":";
#endif
        std::vector<std::string> res;
        for (auto& s : shaders) {
            res.emplace_back("~~/shaders/" + s);
        }
        return pystring::join(separator, res);
    }
};

inline void to_json(nlohmann::json& nlohmann_json_j,
                    const ShaderProfile& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, name, shaders));
}

inline void from_json(const nlohmann::json& nlohmann_json_j,
                      ShaderProfile& nlohmann_json_t) {
    if (nlohmann_json_j.contains("name")) {
        auto& name = nlohmann_json_j.at("name");
        if (name.is_string()) name.get_to(nlohmann_json_t.name);
    }
    if (nlohmann_json_j.contains("shaders")) {
        auto& shaders = nlohmann_json_j.at("shaders");
        if (shaders.is_array()) shaders.get_to(nlohmann_json_t.shaders);
    }
}

typedef std::vector<ShaderProfile> ShaderProfileList;

class AnimeProfile {
public:
    AnimeProfile() = default;
    AnimeProfile(size_t anime, std::string profile)
        : anime(anime), profile(std::move(profile)) {}

    size_t anime = 0;
    std::string profile;
};

inline void to_json(nlohmann::json& nlohmann_json_j,
                    const AnimeProfile& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, anime, profile));
}

inline void from_json(const nlohmann::json& nlohmann_json_j,
                      AnimeProfile& nlohmann_json_t) {
    if (nlohmann_json_j.contains("anime")) {
        auto& anime = nlohmann_json_j.at("anime");
        if (anime.is_number()) anime.get_to(nlohmann_json_t.anime);
    }
    if (nlohmann_json_j.contains("profile")) {
        auto& profile = nlohmann_json_j.at("profile");
        if (profile.is_string()) profile.get_to(nlohmann_json_t.profile);
    }
}

typedef std::vector<AnimeProfile> AnimeProfileList;

class ShaderPack {
public:
    ShaderProfileList profiles;
    AnimeProfileList animeList;
};

inline void to_json(nlohmann::json& nlohmann_json_j,
                    const ShaderPack& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(
        NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, profiles, animeList));
}

inline void from_json(const nlohmann::json& nlohmann_json_j,
                      ShaderPack& nlohmann_json_t) {
    if (nlohmann_json_j.contains("profiles")) {
        auto& profiles = nlohmann_json_j.at("profiles");
        if (profiles.is_array()) profiles.get_to(nlohmann_json_t.profiles);
    }
    if (nlohmann_json_j.contains("animeList")) {
        auto& animeList = nlohmann_json_j.at("animeList");
        if (animeList.is_array()) animeList.get_to(nlohmann_json_t.animeList);
    }
}

class ShaderHelper : public brls::Singleton<ShaderHelper> {
public:
    ShaderHelper() { this->load(); }

    void setShaderPack(const ShaderPack& data) { this->pack = data; }

    ShaderPack getShaderPack() { return this->pack; }

    /// 下面部分涉及到剧集ID的函数并没有实际使用到，相关函数的用意是：
    /// 在某个番剧打开shader时，会记录下番剧和shader的信息，下次再打开同一个番剧就不需要重新开启。
    /// 准备先使用简单模式：即重启软件需要重新开启shader，打开 shader 后对全体视频生效（直播无效）
    /// 看后续反馈再做决定

    void setShader(size_t index, bool showHint = true) {
        MPVCore::instance().setShader(getProfileNameByIndex(index),
                                      getProfileByIndex(index), showHint);
    }

    void clearShader(bool showHint = true) {
        MPVCore::instance().clearShader(showHint);
    }

    /**
     * 设置番剧配置
     * @param sid
     * @param profileIndex
     */
    void setProfile(size_t sid, size_t profileIndex) {
        if (!isAvailable()) return;
        if (profileIndex < 0 || profileIndex >= pack.profiles.size()) return;
        for (auto& anime : pack.animeList) {
            if (anime.anime == sid) {
                // 修改已有配置
                anime.profile = pack.profiles[profileIndex].name;
                this->save();
                return;
            }
        }

        // 新配置
        pack.animeList.emplace_back(sid, pack.profiles[profileIndex].name);
        this->save();
    }

    /**
     * 清空番剧的配置
     * @param sid
     */
    void clearProfile(size_t sid) {
        if (!isAvailable()) return;
        auto iter = this->pack.animeList.end();
        for (auto i = this->pack.animeList.begin();
             i != this->pack.animeList.end(); i++) {
            if (i->anime == sid) {
                iter = i;
                break;
            }
        }
        if (iter != this->pack.animeList.end()) {
            this->pack.animeList.erase(iter);
        }
        this->save();
    }

    /**
     * 通过剧集 id 获取 profile 字符串
     * @param sid anime season id
     * @return
     */
    std::string getProfileBySeason(size_t sid) {
        if (!isAvailable()) return "";
        for (auto& anime : pack.animeList) {
            if (anime.anime == sid) {
                return getProfileByName(anime.profile);
            }
        }
        return "";
    }

    /**
     * 通过配置名称获取 profile 字符串
     * @param name
     * @return
     */
    std::string getProfileByName(const std::string& name) {
        if (!isAvailable()) return "";
        for (auto& profile : pack.profiles) {
            if (profile.name == name) {
                return profile.getShaderString();
            }
        }
        return "";
    }

    /**
     * 通过配置索引获取配置名称
     * @param index
     * @return
     */
    std::string getProfileNameByIndex(size_t index) {
        if (!isAvailable()) return "";
        if (index < 0 || index >= pack.profiles.size()) return "";
        return pack.profiles[index].name;
    }

    /**
     * 通过配置索引获取 profile 字符串
     * @param index
     * @return
     */
    std::string getProfileByIndex(size_t index) {
        if (!isAvailable()) return "";
        if (index < 0 || index >= pack.profiles.size()) return "";
        return pack.profiles[index].getShaderString();
    }

    /**
     * 通过剧集 id 获取配置索引
     * @param sid
     * @return
     */
    size_t getProfileIndexBySeason(size_t sid) {
        if (!isAvailable()) return -1;
        for (auto& anime : pack.animeList) {
            if (anime.anime == sid) {
                return getProfileIndexByName(anime.profile);
            }
        }
        return -1;
    }

    /**
     * 通过配置名称获取配置索引
     * @param name
     * @return
     */
    size_t getProfileIndexByName(const std::string& name) {
        if (!isAvailable()) return -1;
        size_t index = 0;
        for (auto& profile : pack.profiles) {
            if (profile.name == name) {
                return index;
            }
            index++;
        }
        return -1;
    }

    /**
     * 获取配置列表
     * @return
     */
    std::vector<std::string> getProfileList() {
        std::vector<std::string> list;
        for (auto& p : pack.profiles) {
            list.emplace_back(p.name);
        }
        return list;
    }

    [[nodiscard]] bool isAvailable() const { return !pack.profiles.empty(); }

    void load() {
        const std::string path =
            ProgramConfig::instance().getConfigDir() + "/pack.json";

        std::ifstream readFile(path);
        if (readFile) {
            try {
                nlohmann::json content;
                readFile >> content;
                readFile.close();
                this->setShaderPack(content.get<ShaderPack>());
            } catch (const std::exception& e) {
                brls::Logger::error("ShaderHelper::load: {}", e.what());
            }
            brls::Logger::info("Load custom shader pack from: {}", path);
        } else {
            brls::Logger::warning(
                "Cannot find custom shader pack, (Searched at: {})", path);
        }
    }

    void save() {
        const std::string path =
            ProgramConfig::instance().getConfigDir() + "/pack.json";
        // fs is defined in cpr/cpr.h
#ifndef IOS
        fs::create_directories(ProgramConfig::instance().getConfigDir());
#endif
        nlohmann::json content(pack);
        std::ofstream writeFile(path);
        if (!writeFile) {
            brls::Logger::error("Cannot write shader pack to: {}", path);
            return;
        }
        writeFile << content.dump(2);
        writeFile.close();
        brls::Logger::info("Write shader pack to: {}", path);
    }

private:
    ShaderPack pack;
};