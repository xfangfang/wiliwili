//
// CDN optimization, ported from
// https://github.com/SukkaW/Make-Bilibili-Great-Than-Ever-Before
//   (src/utils/get-cdn-url.ts)
//

#include <random>
#include <cctype>
#include <cstring>
#include <algorithm>

#include <borealis/core/logger.hpp>

#include "utils/cdn_helper.hpp"
#include "utils/config_helper.hpp"
#include "utils/string_helper.hpp"

namespace CDNHelper {

inline const std::string PROXY_TF          = "proxy-tf-all-ws.bilivideo.com";
inline const std::string FALLBACK_CDN_HOST = "upos-sz-mirrorali.bilivideo.com";

struct ParsedUrl {
    std::string scheme, host, port, path, query;
    bool valid = false;
};

static ParsedUrl parseUrl(const std::string& raw) {
    ParsedUrl u;
    std::string url = raw;
    if (url.rfind("//", 0) == 0) {
        u.scheme = "https";
        url      = "https:" + url;
    }
    size_t schemeEnd = url.find("://");
    if (schemeEnd == std::string::npos) return u;
    u.scheme = url.substr(0, schemeEnd);

    size_t hostStart = schemeEnd + 3;
    size_t hostEnd   = url.find_first_of("/?#", hostStart);
    std::string authority =
        hostEnd == std::string::npos ? url.substr(hostStart) : url.substr(hostStart, hostEnd - hostStart);
    if (authority.empty()) return u;

    size_t colon = authority.rfind(':');
    if (colon != std::string::npos) {
        u.host = authority.substr(0, colon);
        u.port = authority.substr(colon + 1);
    } else {
        u.host = authority;
    }
    if (u.host.empty()) return u;

    std::string rest = hostEnd == std::string::npos ? "" : url.substr(hostEnd);
    size_t q         = rest.find('?');
    if (q == std::string::npos) {
        u.path = rest.empty() ? "/" : rest;
    } else {
        u.path  = q == 0 ? "/" : rest.substr(0, q);
        u.query = rest.substr(q + 1);
    }
    u.valid = true;
    return u;
}

static std::string buildUrl(const ParsedUrl& u) {
    std::string out = u.scheme + "://" + u.host;
    if (!u.port.empty() && u.port != "443" && u.port != "80") out += ":" + u.port;
    out += u.path;
    if (!u.query.empty()) out += "?" + u.query;
    return out;
}

static bool startsWith(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

static bool endsWith(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static std::string getQueryParam(const std::string& query, const std::string& key) {
    size_t pos = 0;
    while (pos <= query.size()) {
        size_t amp     = query.find('&', pos);
        std::string kv = query.substr(pos, amp == std::string::npos ? std::string::npos : amp - pos);
        size_t eq      = kv.find('=');
        if (eq != std::string::npos && kv.substr(0, eq) == key) return kv.substr(eq + 1);
        if (amp == std::string::npos) break;
        pos = amp + 1;
    }
    return "";
}

bool isP2PHost(const std::string& host) {
    static const char* keywords[] = {"302ppio",
                                     "302kodo",
                                     ".mcdn.bilivideo",
                                     "szbdyd.com",
                                     ".nexusedgeio.com",
                                     ".ahdohpiechei.com",
                                     "upos-sz-mirror14b.bilivideo.com"};
    for (const char* kw : keywords) {
        if (host.find(kw) != std::string::npos) return true;
    }
    std::string firstLabel = host.substr(0, host.find('.'));
    return firstLabel.find("302") != std::string::npos;
}

static bool isMirrorHost(const std::string& host) {
    static const char* suffixes[] = {".bilivideo.com", ".bilivideo.net", ".akamaized.com", ".akamaized.net"};
    for (const char* suffix : suffixes) {
        if (!endsWith(host, suffix)) continue;
        std::string sub = host.substr(0, host.size() - strlen(suffix));
        if (startsWith(sub, "upos-tf-") || startsWith(sub, "proxy-tf-")) return true;
        if (startsWith(sub, "upos-")) {
            std::string rest = sub.substr(5);
            size_t dash      = rest.find('-');
            if (dash > 0 && dash != std::string::npos && dash + 1 < rest.size() &&
                !startsWith(rest.substr(dash + 1), "302"))
                return true;
        }
    }
    return false;
}

static bool isIPv4Host(const std::string& host) {
    int dots = 0;
    for (char c : host) {
        if (c == '.')
            dots++;
        else if (!isdigit((unsigned char)c))
            return false;
    }
    return dots == 3;
}

static bool isMcdnTfUrl(const ParsedUrl& u) {
    bool mcdnHost = isIPv4Host(u.host) || endsWith(u.host, ".mcdn.bilivideo.com") ||
                    endsWith(u.host, ".mcdn.bilivideo.cn") || endsWith(u.host, ".mcdn.bilivideo.net");
    if (!mcdnHost) return false;
    if (!startsWith(u.path, "/v") || u.path.size() < 3 || !isdigit((unsigned char)u.path[2])) return false;
    size_t next = u.path.find('/', 1);
    return next != std::string::npos && u.path.compare(next, 9, "/resource") == 0;
}

URLType classify(const std::string& url) {
    ParsedUrl u = parseUrl(url);
    if (!u.valid) return URLType::UNKNOWN;

    if (url.find("/upgcxcode/") != std::string::npos) {
        if (isMirrorHost(u.host)) {
            if (getQueryParam(u.query, "os") == "mcdn" || isP2PHost(u.host)) return URLType::P2P_UPGCXCODE;
            return URLType::MIRROR;
        }
        if (isP2PHost(u.host)) return URLType::P2P_UPGCXCODE;
        return URLType::BCACHE;
    }

    if (isMcdnTfUrl(u)) return URLType::MCDN_TF;

    if (url.find("szbdyd.com") != std::string::npos) return URLType::SZBDYD;

    return URLType::UNKNOWN;
}

void HostPool::collect(const std::string& url) {
    URLType type = classify(url);
    if (type == URLType::MIRROR) {
        mirrorHosts.insert(parseUrl(url).host);
    } else if (type == URLType::BCACHE) {
        bcacheHosts.insert(parseUrl(url).host);
    }
}

static std::mt19937& rng() {
    static std::mt19937 gen{std::random_device{}()};
    return gen;
}

static std::string pickOne(const std::unordered_set<std::string>& hosts) {
    if (hosts.empty()) return "";
    auto it = hosts.begin();
    std::advance(it, std::uniform_int_distribution<size_t>(0, hosts.size() - 1)(rng()));
    return *it;
}

static std::string forceHttps(const std::string& url) {
    ParsedUrl u = parseUrl(url);
    if (!u.valid) return url;
    u.scheme = "https";
    u.port.clear();
    return buildUrl(u);
}

static std::string replaceUpgcxcodeHost(const std::string& url, const HostPool& pool) {
    ParsedUrl u = parseUrl(url);
    if (!u.valid) return url;
    u.scheme = "https";
    u.port.clear();

    std::string host = pickOne(pool.mirrorHosts);
    if (host.empty()) host = pickOne(pool.bcacheHosts);
    if (host.empty()) host = FALLBACK_CDN_HOST;
    u.host = host;
    return buildUrl(u);
}

Mode getMode() {
    int value = ProgramConfig::instance().getIntOption(SettingItem::CDN_OPTIMIZE);
    // 配置文件被手动改出范围时按关闭处理
    if (value < static_cast<int>(Mode::OFF) || value > static_cast<int>(Mode::FULL)) return Mode::OFF;
    return static_cast<Mode>(value);
}

std::vector<std::string> optimize(const std::string& baseUrl, const std::vector<std::string>& backupUrls,
                                  const HostPool& pool) {
    std::vector<std::string> input;
    input.reserve(backupUrls.size() + 1);
    input.emplace_back(baseUrl);
    input.insert(input.end(), backupUrls.begin(), backupUrls.end());

    Mode mode = getMode();
    if (mode == Mode::OFF || baseUrl.empty()) return input;

    std::vector<std::string> mirrorUrls, bcacheUrls, p2pUrls, szbdydUrls, tfUrls, unknownUrls;
    for (const auto& url : input) {
        switch (classify(url)) {
            case URLType::MIRROR:
                mirrorUrls.emplace_back(forceHttps(url));
                break;
            case URLType::BCACHE:
                bcacheUrls.emplace_back(url);
                break;
            case URLType::P2P_UPGCXCODE:
                p2pUrls.emplace_back(url);
                break;
            case URLType::SZBDYD:
                szbdydUrls.emplace_back(url);
                break;
            case URLType::MCDN_TF:
                tfUrls.emplace_back(url);
                break;
            default:
                unknownUrls.emplace_back(url);
                break;
        }
    }

    // 同一优先级内随机排序以分散负载，不同优先级之间保持顺序
    std::shuffle(mirrorUrls.begin(), mirrorUrls.end(), rng());
    std::shuffle(bcacheUrls.begin(), bcacheUrls.end(), rng());
    std::shuffle(p2pUrls.begin(), p2pUrls.end(), rng());
    std::shuffle(szbdydUrls.begin(), szbdydUrls.end(), rng());
    std::shuffle(tfUrls.begin(), tfUrls.end(), rng());

    std::vector<std::string> result;
    if (mode == Mode::MIRROR_ONLY) {
        result.insert(result.end(), mirrorUrls.begin(), mirrorUrls.end());
        result.insert(result.end(), bcacheUrls.begin(), bcacheUrls.end());
        // UNKNOWN 未被判定为 P2P，保留作为兜底
        result.insert(result.end(), unknownUrls.begin(), unknownUrls.end());
    } else {
        // 各优先级依次拼接，低优先级链接改写后保留为播放失败时的备选
        result.insert(result.end(), mirrorUrls.begin(), mirrorUrls.end());
        result.insert(result.end(), bcacheUrls.begin(), bcacheUrls.end());
        for (const auto& url : p2pUrls) result.emplace_back(replaceUpgcxcodeHost(url, pool));
        for (const auto& url : szbdydUrls) {
            ParsedUrl u      = parseUrl(url);
            std::string usrc = u.valid ? getQueryParam(u.query, "xy_usource") : "";
            if (!usrc.empty()) {
                u.scheme = "https";
                u.port.clear();
                u.host = usrc;
                result.emplace_back(buildUrl(u));
            } else {
                result.emplace_back(replaceUpgcxcodeHost(url, pool));
            }
        }
        for (const auto& url : tfUrls) {
            result.emplace_back("https://" + PROXY_TF + "/?url=" + wiliwili::urlEncode(url));
        }
        result.insert(result.end(), unknownUrls.begin(), unknownUrls.end());
    }

    if (result.empty()) {
        brls::Logger::warning("CDNHelper: no usable CDN url, fallback to original list");
        return input;
    }

    brls::Logger::debug("CDNHelper: {} -> {}", baseUrl, result[0]);
    return result;
}

std::string stripSmtcdns(const std::string& url) {
    size_t scheme = url.find("://");
    if (scheme == std::string::npos) return url;
    size_t hostStart = scheme + 3;
    size_t hostEnd   = url.find('/', hostStart);
    if (hostEnd == std::string::npos) return url;
    // 外层 host 必须以 .smtcdns.net 结尾
    if (!endsWith(url.substr(hostStart, hostEnd - hostStart), ".smtcdns.net")) return url;
    // 内层 host 必须紧随其后且以 .bilivideo.com 结尾
    std::string rest = url.substr(hostEnd + 1);
    size_t innerEnd  = rest.find_first_of("/?#");
    if (innerEnd == std::string::npos) return url;
    if (!endsWith(rest.substr(0, innerEnd), ".bilivideo.com")) return url;
    return url.substr(0, hostStart) + rest;
}

std::vector<std::string> rankLive(const std::vector<std::string>& urls) {
    Mode mode = getMode();
    if (mode == Mode::OFF) return urls;

    std::vector<std::string> clean, rest;
    for (const auto& raw : urls) {
        std::string url = stripSmtcdns(raw);
        ParsedUrl u     = parseUrl(url);
        bool ok         = u.valid && !isP2PHost(u.host) &&
                          (endsWith(u.host, ".bilivideo.com") || endsWith(u.host, ".bilivideo.net") ||
                           endsWith(u.host, ".akamaized.net"));
        (ok ? clean : rest).emplace_back(url);
    }

    if (mode == Mode::MIRROR_ONLY) {
        return clean.empty() ? urls : clean;
    }
    clean.insert(clean.end(), rest.begin(), rest.end());
    return clean.empty() ? urls : clean;
}

};  // namespace CDNHelper
