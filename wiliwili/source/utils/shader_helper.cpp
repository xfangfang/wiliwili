//
// Created by fang on 2024/1/10.
//

#include <unistd.h>
#include <nlohmann/json.hpp>
#include <borealis/core/singleton.hpp>
#include <borealis/core/application.hpp>
#include <borealis/core/util.hpp>
#include <utility>
#include <pystring.h>
#include <cpr/filesystem.h>
#include "utils/config_helper.hpp"
#include "utils/shader_helper.hpp"
#include "view/mpv_core.hpp"

std::string ShaderProfile::getShaderString() {
#ifdef _WIN32
    std::string separator = ";";
#else
    std::string separator = ":";
#endif
    std::vector<std::string> res;
    res.reserve(shaders.size());
    for (auto& s : shaders) {
        res.emplace_back("~~/shaders/" + s);
    }
    return pystring::join(separator, res);
}

ShaderHelper::ShaderHelper() { this->load(); }

void ShaderHelper::setShaderPack(const ShaderPack& data) { this->pack = data; }

ShaderPack ShaderHelper::getShaderPack() { return this->pack; }

void ShaderHelper::setShader(size_t index, bool showHint) {
    MPVCore::instance().setShader(getProfileNameByIndex(index), getProfileShaderByIndex(index),
                                  getByProfileSettingByIndex(index), showHint);
}

void ShaderHelper::clearShader(bool showHint) { MPVCore::instance().clearShader(showHint); }

void ShaderHelper::setProfile(size_t sid, size_t profileIndex) {
    if (!isAvailable()) return;
    if (profileIndex >= pack.profiles.size()) return;
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

void ShaderHelper::clearProfile(size_t sid) {
    if (!isAvailable()) return;
    auto iter = this->pack.animeList.end();
    for (auto i = this->pack.animeList.begin(); i != this->pack.animeList.end(); i++) {
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

std::string ShaderHelper::getProfileBySeason(size_t sid) {
    if (!isAvailable()) return "";
    for (auto& anime : pack.animeList) {
        if (anime.anime == sid) {
            return getProfileByName(anime.profile);
        }
    }
    return "";
}

std::string ShaderHelper::getProfileByName(const std::string& name) {
    if (!isAvailable()) return "";
    for (auto& profile : pack.profiles) {
        if (profile.name == name) {
            return profile.getShaderString();
        }
    }
    return "";
}

std::string ShaderHelper::getProfileNameByIndex(size_t index) {
    if (!isAvailable()) return "";
    if (index >= pack.profiles.size()) return "";
    return pack.profiles[index].name;
}

std::string ShaderHelper::getProfileShaderByIndex(size_t index) {
    if (!isAvailable()) return "";
    if (index >= pack.profiles.size()) return "";
    return pack.profiles[index].getShaderString();
}

std::vector<std::vector<std::string>> ShaderHelper::getByProfileSettingByIndex(size_t index) {
    if (!isAvailable()) return {};
    if (index >= pack.profiles.size()) return {};
    return pack.profiles[index].settings;
}

size_t ShaderHelper::getProfileIndexBySeason(size_t sid) {
    if (!isAvailable()) return -1;
    for (auto& anime : pack.animeList) {
        if (anime.anime == sid) {
            return getProfileIndexByName(anime.profile);
        }
    }
    return -1;
}

size_t ShaderHelper::getProfileIndexByName(const std::string& name) {
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

std::vector<std::string> ShaderHelper::getProfileList() {
    std::vector<std::string> list;
    for (auto& p : pack.profiles) {
        list.emplace_back(p.name);
    }
    return list;
}

[[nodiscard]] bool ShaderHelper::isAvailable() const { return !pack.profiles.empty(); }

void ShaderHelper::load() {
    const std::string path = ProgramConfig::instance().getConfigDir() + "/pack.json";

    std::ifstream readFile(path);
    if (readFile) {
        try {
            nlohmann::json content;
            readFile >> content;
            readFile.close();
            this->setShaderPack(content.get<ShaderPack>());
        } catch (const std::exception& e) {
            brls::Logger::error("ShaderHelper::load: {}", e.what());
            brls::Application::notify("Load custom shader pack failed\n" + std::string(e.what()));
        }
        brls::Logger::info("Load custom shader pack from: {}", path);
    } else {
        brls::Logger::warning("Cannot find custom shader pack, (Searched at: {})", path);
    }
}

void ShaderHelper::save() {
    const std::string path = ProgramConfig::instance().getConfigDir() + "/pack.json";
    // fs is defined in cpr/cpr.h
#ifndef IOS
    cpr::fs::create_directories(ProgramConfig::instance().getConfigDir());
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
