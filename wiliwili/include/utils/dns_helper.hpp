//
// Created by fang on 2024/2/17.
//

#pragma once
#include <thread>
#include <atomic>
#include <unordered_map>
#include <mongoose.h>
#include <borealis/core/singleton.hpp>
#include <borealis/core/time.hpp>

class DNSResult {
public:
    DNSResult() = default;

    std::string host;
    std::vector<std::function<void(const std::string &)>> callback;
    struct mg_connection *c{};
    std::string ip;
    brls::Time availableTime{};

    /**
     * 缓存是否可用
     */
    bool available() const;

    /**
     * 是否正在请求
     */
    bool requesting() const;

    std::atomic<bool> _requesting{true};
};

class DNSHelper : public brls::Singleton<DNSHelper> {
public:
    DNSHelper();

    ~DNSHelper();

    void start();

    void stop();

    /**
     * 解析域名
     * @param host 域名
     * @param callback 回调函数
     * @return 返回连接对象
     */
    struct mg_connection *resolve(const std::string &host, std::function<void(const std::string &)> callback);

    /**
     * 设置自定义的 DNS 服务器
     * @param v4 默认值 "udp://8.8.8.8:53" 设置为空则强制使用 ipv6
     * @param v6 默认值 "udp://[2001:4860:4860::8888]:53"
     */
    void setDNSServer(const std::string &v4, const std::string &v6 = "udp://[2001:4860:4860::8888]:53");

    /**
     * 设置 DNS 请求超时时间
     * @param timeout 默认值 3000 ms
     */
    void setDNSTimeout(int timeout = 3000);

    /**
     * 设置 DNS 缓存时间（暂未实现）
     * @param timeout 默认值 60000 ms
     */
     void setDNSCacheTime(int cache = 60000);

    void cancelAllResolve();

private:
    std::string dns4;
    std::string dns6;
    int dnsTimeout{};
    int dnsCacheTime{};
    struct mg_mgr mgr {};
    std::thread dnsRequestThread{};
    std::atomic<bool> running{};
    std::unordered_map<std::string, DNSResult> dnsCache;

    void _setDNSServer(const std::string &v4, const std::string &v6);

    void _setDNSTimeout(int timeout);
};