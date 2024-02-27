//
// Created by fang on 2023/5/17.
//

#include <set>
#include <future>
#include <mongoose.h>
#include <cpr/cpr.h>
#include <pystring.h>
#include <borealis/core/logger.hpp>
#include "utils/string_helper.hpp"
#include "utils/config_helper.hpp"
#include "dlna/dlna.h"

static std::string AVTransport =
    "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" "
    "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
    "xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">"
    "<s:Body>"
    "<u:SetAVTransportURI "
    "xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">"
    "<InstanceID>0</InstanceID>"
    "<CurrentURI>{}</CurrentURI>"
    "<CurrentURIMetaData>&lt;DIDL-Lite "
    "xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" "
    "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" "
    "xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\"&gt;&lt;item "
    "id=\"123\" parentID=\"-1\" restricted=\"1\"&gt;&lt;res "
    "protocolInfo=\"http-get:*:video/"
    "*:*;DLNA.ORG_OP=01;DLNA.ORG_FLAGS=01700000000000000000000000000000\"&gt;{}"
    "&lt;/res&gt;&lt;upnp:storageMedium&gt;UNKNOWN&lt;/"
    "upnp:storageMedium&gt;&lt;upnp:writeStatus&gt;UNKNOWN&lt;/"
    "upnp:writeStatus&gt;&lt;dc:title&gt;{}&lt;/"
    "dc:title&gt;&lt;upnp:class&gt;object.item.videoItem&lt;/"
    "upnp:class&gt;&lt;/item&gt;&lt;/DIDL-Lite&gt;</CurrentURIMetaData>"
    "</u:SetAVTransportURI>"
    "</s:Body>"
    "</s:Envelope>";

static std::string Stop =
    "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" "
    "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
    "xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">"
    "<s:Body>"
    "<u:Stop xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">"
    "<InstanceID>0</InstanceID>"
    "</u:Stop>"
    "</s:Body>"
    "</s:Envelope>";

static const char* s_ssdp_url = "udp://239.255.255.250:1900";

static void fn(struct mg_connection* c, int ev, void* ev_data) {
    MG_DEBUG(("%p got event: %d %p %p", c, ev, ev_data, c->fn_data));
    if (ev == MG_EV_OPEN) {
        //    c->is_hexdumping = 1;
    } else if (ev == MG_EV_RESOLVE) {
        // c->rem gets populated with multicast address. Store it in c->data
        memcpy(c->data, &c->rem, sizeof(c->rem));
    } else if (ev == MG_EV_READ) {
        MG_INFO(("Got a response"));
        struct mg_http_message hm {};
        if (mg_http_parse((const char*)c->recv.buf, c->recv.len, &hm) > 0) {
            size_t i, max = sizeof(hm.headers) / sizeof(hm.headers[0]);
            // Iterate over request headers
            for (i = 0; i < max && hm.headers[i].name.len > 0; i++) {
                struct mg_str *k = &hm.headers[i].name, *v = &hm.headers[i].value;
                if (mg_vcasecmp(k, "LOCATION") == 0) {
                    UpnpDlna::rendererList.insert(std::string{v->ptr}.substr(0, v->len));
                }
            }
        }
        // Each response to the SSDP socket will change c->rem.
        // We can now do mg_printf(c, "haha"); to respond back to the remote side.
        // But in our case, we should restore the multicast address in order
        // to have next search to go to the multicast address
        memcpy(&c->rem, c->data, sizeof(c->rem));
        // Discard the content of this response as we expect each SSDP response
        // to generate at most one MG_EV_READ event.
        c->recv.len = 0UL;
    }
}

static void sendSearch(void* param) {
    auto* c = (struct mg_connection*)param;
    if (c == nullptr) return;
    MG_INFO(("Sending M-SEARCH"));
    mg_printf(c, "%s",
              "M-SEARCH * HTTP/1.1\r\n"
              "HOST: 239.255.255.250:1900\r\n"
              "MAN: \"ssdp:discover\"\r\n"
              "ST: urn:schemas-upnp-org:device:MediaRenderer:1\r\n"
              "MX: 1\r\n"
              "\r\n");
}

std::vector<DlnaRenderer> UpnpDlna::searchRenderer(int timeout) {
    brls::Logger::info("开始SSDP搜索");
    if (mgr != nullptr) return {};
    running.store(true);
    mgr = new struct mg_mgr;
    mg_mgr_init(mgr);
    mg_wakeup_init(mgr);
    connection = mg_connect(mgr, s_ssdp_url, fn, nullptr);
    sendSearch(connection);
    rendererList.clear();
    auto startTime = std::chrono::system_clock::now();
    while (running) {
        auto nowTime = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - startTime);
        if (elapsed.count() >= timeout) {
            break;
        }
        mg_mgr_poll(mgr, 200);
    }
    mg_mgr_free(mgr);
    delete mgr;
    mgr = nullptr;
    connection = nullptr;
    brls::Logger::info("SSDP搜索结束，开始获取renderer信息");
    std::vector<DlnaRenderer> list;

    cpr::MultiPerform multiperform;
    std::vector<std::shared_ptr<cpr::Session>> sessionList;
    for (auto& i : rendererList) {
        brls::Logger::debug("Got renderer: {}", i);
        auto s = std::make_shared<cpr::Session>();
        sessionList.emplace_back(s);
        s->SetUrl(i);
        s->SetTimeout(5000);
        multiperform.AddSession(s);
    }
    std::vector<cpr::Response> responses = multiperform.Get();
    for (auto& r : responses) {
        if (r.status_code != 200) {
            continue;
        }
        auto renderer = DlnaRenderer::parse(r.text);
        if (!renderer.isValid()) continue;
        renderer.setBaseUrl(r.url.str());
        list.emplace_back(renderer);
    }

    brls::Logger::info("renderer信息获取结束");
    return list;
}

void UpnpDlna::stopSearch() {
    if (!running.load()) return;
    running.store(false);
    if (mgr && connection)
        mg_wakeup(this->mgr, connection->id, nullptr, 0);
}

void DlnaRenderer::play(const std::string& url, const std::string& title, const std::function<void()>& callback,
                        const std::function<void()>& error) const {
    std::string point = getAvTransportUrl();
    if (point.empty()) {
        brls::Logger::error("renderer not support AVTransport");
        return;
    }

    std::string urlEncode = pystring::replace(url, "&", "&amp;");
    std::string data      = wiliwili::format(AVTransport, urlEncode, "", title);
    std::string server    = "System/1.0 UPnP/1.0 wiliwili/" + APPVersion::instance().getVersionStr();

    cpr::PostCallback(
        [callback, error](const cpr::Response& r) {
            if (r.status_code != 200) {
                error();
                return;
            }
            callback();
        },
        cpr::Url{point},
        cpr::Header{
            {"Content-Type", "text/xml; charset=\"utf-8\""},
            {"SOAPACTION",
             "\"urn:schemas-upnp-org:service:AVTransport:1#"
             "SetAVTransportURI\""},
            {"User-Agent", server},
        },
        cpr::Body{data});

    brls::Logger::verbose("post to: {}\nwith data: {}", point, data);
}

void DlnaRenderer::stop(const std::function<void()>& callback, const std::function<void()>& error) const {
    std::string point = getAvTransportUrl();
    if (point.empty()) {
        brls::Logger::error("renderer not support AVTransport");
        return;
    }
    std::string server = "System/1.0 UPnP/1.0 wiliwili/" + APPVersion::instance().getVersionStr();

    cpr::PostCallback(
        [callback, error](const cpr::Response& r) {
            if (r.status_code != 200) {
                error();
                return;
            }
            callback();
        },
        cpr::Url{point},
        cpr::Header{
            {"Content-Type", "text/xml; charset=\"utf-8\""},
            {"SOAPACTION", "\"urn:schemas-upnp-org:service:AVTransport:1#Stop\""},
            {"User-Agent", server},
        },
        cpr::Body{Stop});

    brls::Logger::verbose("post to: {}\nwith data: {}", point, Stop);
}

std::string DlnaRenderer::getAvTransportUrl() const {
    std::string point;
    for (auto& i : rendererServiceList) {
        if (i.serviceType == "urn:schemas-upnp-org:service:AVTransport:1") {
            if (i.controlURL.empty()) break;
            std::string connector;
            if (i.controlURL[0] != '/') connector = "/";
            point = i.baseURL + connector + i.controlURL;
            break;
        }
    }
    return point;
}

DlnaRenderer DlnaRenderer::parse(const std::string& xml) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error = doc.Parse(xml.c_str());
    if (error != tinyxml2::XMLError::XML_SUCCESS) {
        brls::Logger::error("Invalid XML: {}", std::to_string(error));
        return {};
    }

    auto* root = doc.RootElement();
    if (!root) {
        brls::Logger::error("Invalid XML: no element found");
        return {};
    }

    DlnaRenderer renderer;
    try {
        renderer.Deserialize(root);
    } catch (const std::exception& e) {
        brls::Logger::error("Error: {}", e.what());
        return {};
    }

    return renderer;
}

void DlnaRenderer::Deserialize(const tinyxml2::XMLElement* element) {
    // upnp版本
    auto* specVersion = element->FirstChildElement("specVersion");
    majorVersion      = getChildInt(specVersion, "major");
    minorVersion      = getChildInt(specVersion, "minor");

    // 主要信息
    auto* device = element->FirstChildElement("device");
    deviceType   = getChildText(device, "deviceType");
    UDN          = getChildText(device, "UDN");
    friendlyName = getChildText(device, "friendlyName");
    manufacturer = getChildText(device, "manufacturer");

    // 服务列表
    if (!device) return;
    auto* serviceList = device->FirstChildElement("serviceList");

    rendererServiceList.clear();
    for (auto* child = serviceList->FirstChildElement("service"); child; child = child->NextSiblingElement("service")) {
        DlnaRendererService service;
        service.Deserialize(child);
        if (!service.isValid()) return;
        rendererServiceList.emplace_back(service);
    }

    if (deviceType != "urn:schemas-upnp-org:device:MediaRenderer:1") return;
    this->setValid(true);
}

void DlnaRenderer::setBaseUrl(const std::string& value) {
    const char* uri = mg_url_uri(value.c_str());
    if (uri - value.c_str() > 0)
        this->baseURL = value.substr(0, uri - value.c_str());
    else
        this->baseURL = value;

    for (auto& i : rendererServiceList) {
        i.setBaseUrl(this->baseURL);
    }
}

void DlnaRenderer::print() {
    brls::Logger::debug("renderer:\nbaseURL: {}\ndeviceType: {}\nUDN: {}\nfriendlyName: {}", baseURL, deviceType, UDN,
                        friendlyName, manufacturer);

    for (auto& i : rendererServiceList) {
        i.print();
    }
}

void DlnaRendererService::Deserialize(const tinyxml2::XMLElement* element) {
    serviceType = getChildText(element, "serviceType");
    serviceId   = getChildText(element, "serviceId");
    controlURL  = getChildText(element, "controlURL");
    eventSubURL = getChildText(element, "eventSubURL");
    SCPDURL     = getChildText(element, "SCPDURL");
    this->setValid(true);
}

void DlnaRendererService::setBaseUrl(const std::string& value) { this->baseURL = value; }

void DlnaRendererService::print() {
    brls::Logger::debug("service:\n\t\t{}\n\t\t{}\n\t\t{}\n\t\t{}", serviceType, serviceId, controlURL, eventSubURL,
                        eventSubURL, SCPDURL);
}