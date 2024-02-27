//
// Created by fang on 2023/5/17.
//

#pragma once

#include <vector>
#include <string>
#include <set>
#include <tinyxml2.h>
#include <borealis/core/singleton.hpp>

class XmlClass {
public:
    int getChildInt(const tinyxml2::XMLElement* element, std::string key, int defaultInt = 0) {
        if (!element) return defaultInt;
        auto* child = element->FirstChildElement(key.c_str());
        if (!child) return defaultInt;
        return child->IntText(defaultInt);
    }

    std::string getChildText(const tinyxml2::XMLElement* element, std::string key, std::string defaultText = "") {
        if (!element) return defaultText;
        auto* child = element->FirstChildElement(key.c_str());
        if (!child) return defaultText;
        return child->GetText();
    }

    void setValid(bool value) { valid = value; };

    bool isValid() { return valid; };

private:
    bool valid = false;
};

class DlnaRendererService : public XmlClass {
public:
    void Deserialize(const tinyxml2::XMLElement* element);
    void setBaseUrl(const std::string& value);
    void print();
    std::string serviceType, serviceId, controlURL, eventSubURL, SCPDURL, baseURL;
};

class DlnaRenderer : public XmlClass {
public:
    static DlnaRenderer parse(const std::string& xml);

    void Deserialize(const tinyxml2::XMLElement* element);
    void setBaseUrl(const std::string& value);
    void play(const std::string& url, const std::string& title, const std::function<void()>& callback,
              const std::function<void()>& error) const;
    void stop(const std::function<void()>& callback, const std::function<void()>& error) const;

    std::string getAvTransportUrl() const;

    void print();

    std::string deviceType, UDN, friendlyName, manufacturer, baseURL;
    int majorVersion = 0, minorVersion = 0;
    std::vector<DlnaRendererService> rendererServiceList;
};

class UpnpDlna : public brls::Singleton<UpnpDlna> {
public:
    /**
     * 阻塞地查询 renderer 列表
     * @param timeout 等待查询时间（毫秒），误差在200ms内
     * @return renderer 列表
     */
    std::vector<DlnaRenderer> searchRenderer(int timeout = 5000);

    void stopSearch();

    static inline std::set<std::string> rendererList;

private:
    struct mg_mgr* mgr               = nullptr;
    struct mg_connection* connection = nullptr;
    std::atomic_bool running         = false;
};