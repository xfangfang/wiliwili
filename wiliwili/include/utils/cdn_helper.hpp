//
// CDN optimization, ported from "Make Bilibili Great Than Ever Before"
// https://github.com/SukkaW/Make-Bilibili-Great-Than-Ever-Before (src/utils/get-cdn-url.ts)
//

#pragma once

#include <string>
#include <vector>
#include <unordered_set>

namespace CDNHelper {

enum class Mode {
    OFF         = 0,
    MIRROR_ONLY = 1,
    FULL        = 2,
};

Mode getMode();

enum class URLType {
    MIRROR,
    BCACHE,
    P2P_UPGCXCODE,
    MCDN_TF,
    SZBDYD,
    UNKNOWN,
};

URLType classify(const std::string& url);

bool isP2PHost(const std::string& host);

struct HostPool {
    std::unordered_set<std::string> mirrorHosts;
    std::unordered_set<std::string> bcacheHosts;

    void collect(const std::string& url);
};

std::vector<std::string> optimize(const std::string& baseUrl, const std::vector<std::string>& backupUrls,
                                  const HostPool& pool);

std::string stripSmtcdns(const std::string& url);

std::vector<std::string> rankLive(const std::vector<std::string>& urls);

};  // namespace CDNHelper
