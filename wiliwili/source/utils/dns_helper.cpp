//
// Created by fang on 2024/2/17.
//

#include <pystring.h>
#include <fmt/format.h>

#include "utils/dns_helper.hpp"

// Modified from mongoose (only `dns_connect_resolved` is changed to disable socket connection after dns resolved )

struct dns_data {
    struct dns_data *next;
    struct mg_connection *c;
    uint64_t expire;
    uint16_t txnid;
};

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
    if (ev == MG_EV_RESOLVE) {
        auto cb        = *static_cast<std::function<void(const std::string &)> *>(nc->fn_data);
        auto &p        = nc->rem.ip;
        std::string ip = nc->rem.is_ip6 ? fmt::format("[{:x}:{:x}:{:x}:{:x}:{:x}:{:x}:{:x}:{:x}]", mg_ntohs(p[0]),
                                                      mg_ntohs(p[1]), mg_ntohs(p[2]), mg_ntohs(p[3]), mg_ntohs(p[4]),
                                                      mg_ntohs(p[5]), mg_ntohs(p[6]), mg_ntohs(p[7]))
                                        : fmt::format("{}.{}.{}.{}", p[0], p[1], p[2], p[3]);
        cb(ip);
    }
    (void)ev_data;
}

void dns_connect_resolved(struct mg_connection *c) {
    c->is_resolving = 0;  // Clear resolving flag
    c->is_closing   = 1;  // Close the connection
    mg_call(c, MG_EV_RESOLVE, NULL);
}

static void mg_dns_free(struct dns_data **head, struct dns_data *d) {
    LIST_DELETE(struct dns_data, head, d);
    free(d);
}

static void mg_sendnsreq(struct mg_connection *, struct mg_str *, int, struct mg_dns *, bool);

static void dns_cb(struct mg_connection *c, int ev, void *ev_data) {
    struct dns_data *d, *tmp;
    struct dns_data **head = (struct dns_data **)&c->mgr->active_dns_requests;
    if (ev == MG_EV_POLL) {
        uint64_t now = *(uint64_t *)ev_data;
        for (d = *head; d != NULL; d = tmp) {
            tmp = d->next;
            // MG_DEBUG ("%lu %lu dns poll", d->expire, now));
            if (now > d->expire) mg_error(d->c, "DNS timeout");
        }
    } else if (ev == MG_EV_READ) {
        struct mg_dns_message dm;
        int resolved = 0;
        if (mg_dns_parse(c->recv.buf, c->recv.len, &dm) == false) {
            MG_ERROR(("Unexpected DNS response:"));
            mg_hexdump(c->recv.buf, c->recv.len);
        } else {
            // MG_VERBOSE(("%s %d", dm.name, dm.resolved));
            for (d = *head; d != NULL; d = tmp) {
                tmp = d->next;
                // MG_INFO(("d %p %hu %hu", d, d->txnid, dm.txnid));
                if (dm.txnid != d->txnid) continue;
                if (d->c->is_resolving) {
                    if (dm.resolved) {
                        dm.addr.port = d->c->rem.port;  // Save port
                        d->c->rem    = dm.addr;         // Copy resolved address
                        MG_DEBUG(("%lu %s is %M", d->c->id, dm.name, mg_print_ip, &d->c->rem));
                        dns_connect_resolved(d->c);
#if MG_ENABLE_IPV6
                    } else if (dm.addr.is_ip6 == false && dm.name[0] != '\0' && c->mgr->use_dns6 == false) {
                        struct mg_str x = mg_str(dm.name);
                        mg_sendnsreq(d->c, &x, c->mgr->dnstimeout, &c->mgr->dns6, true);
#endif
                    } else {
                        mg_error(d->c, "%s DNS lookup failed", dm.name);
                    }
                } else {
                    MG_ERROR(("%lu already resolved", d->c->id));
                }
                mg_dns_free(head, d);
                resolved = 1;
            }
        }
        if (!resolved) MG_ERROR(("stray DNS reply"));
        c->recv.len = 0;
    } else if (ev == MG_EV_CLOSE) {
        for (d = *head; d != NULL; d = tmp) {
            tmp = d->next;
            mg_error(d->c, "DNS error");
            mg_dns_free(head, d);
        }
    }
}

static bool mg_dns_send(struct mg_connection *c, const struct mg_str *name, uint16_t txnid, bool ipv6) {
    struct {
        struct mg_dns_header header;
        uint8_t data[256];
    } pkt;
    size_t i, n;
    memset(&pkt, 0, sizeof(pkt));
    pkt.header.txnid         = mg_htons(txnid);
    pkt.header.flags         = mg_htons(0x100);
    pkt.header.num_questions = mg_htons(1);
    for (i = n = 0; i < sizeof(pkt.data) - 5; i++) {
        if (name->ptr[i] == '.' || i >= name->len) {
            pkt.data[n] = (uint8_t)(i - n);
            memcpy(&pkt.data[n + 1], name->ptr + n, i - n);
            n = i + 1;
        }
        if (i >= name->len) break;
    }
    memcpy(&pkt.data[n], "\x00\x00\x01\x00\x01", 5);  // A query
    n += 5;
    if (ipv6) pkt.data[n - 3] = 0x1c;  // AAAA query
    // memcpy(&pkt.data[n], "\xc0\x0c\x00\x1c\x00\x01", 6);  // AAAA query
    // n += 6;
    return mg_send(c, &pkt, sizeof(pkt.header) + n);
}

static void mg_sendnsreq(struct mg_connection *c, struct mg_str *name, int ms, struct mg_dns *dnsc, bool ipv6) {
    struct dns_data *d = NULL;
    if (dnsc->url == NULL) {
        mg_error(c, "DNS server URL is NULL. Call mg_mgr_init()");
    } else if (dnsc->c == NULL) {
        dnsc->c = mg_connect(c->mgr, dnsc->url, NULL, NULL);
        if (dnsc->c != NULL) {
            dnsc->c->pfn = dns_cb;
        }
    }
    if (dnsc->c == NULL) {
        mg_error(c, "resolver");
    } else if ((d = (struct dns_data *)calloc(1, sizeof(*d))) == NULL) {
        mg_error(c, "resolve OOM");
    } else {
        struct dns_data *reqs       = (struct dns_data *)c->mgr->active_dns_requests;
        d->txnid                    = reqs ? (uint16_t)(reqs->txnid + 1) : 1;
        d->next                     = (struct dns_data *)c->mgr->active_dns_requests;
        c->mgr->active_dns_requests = d;
        d->expire                   = mg_millis() + (uint64_t)ms;
        d->c                        = c;
        c->is_resolving             = 1;
        MG_VERBOSE(("%lu resolving %.*s @ %s, txnid %hu", c->id, (int)name->len, name->ptr, dnsc->url, d->txnid));
        if (!mg_dns_send(dnsc->c, name, d->txnid, ipv6)) {
            mg_error(dnsc->c, "DNS send");
        }
    }
}

struct mg_connection *dns_resolve(struct mg_mgr *mgr, const char *url, mg_event_handler_t fn, void *fn_data) {
    struct mg_connection *c = NULL;
    if (url == NULL || url[0] == '\0') {
        MG_ERROR(("null url"));
    } else if ((c = mg_alloc_conn(mgr)) == NULL) {
        MG_ERROR(("OOM"));
    } else {
        LIST_ADD_HEAD(struct mg_connection, &mgr->conns, c);
        c->is_udp          = (strncmp(url, "udp:", 4) == 0);
        c->fd              = (void *)(size_t)MG_INVALID_SOCKET;
        c->fn              = fn;
        c->is_client       = true;
        c->fn_data         = fn_data;
        struct mg_str host = mg_url_host(url);
        c->rem.port        = mg_htons(mg_url_port(url));
        if (mg_aton(host, &c->rem)) {
            // host is an IP address, do not fire name resolution
            dns_connect_resolved(c);
        } else {
            // host is not an IP, send DNS resolution request
            struct mg_dns *dns = c->mgr->use_dns6 ? &c->mgr->dns6 : &c->mgr->dns4;
            mg_sendnsreq(c, &host, c->mgr->dnstimeout, dns, c->mgr->use_dns6);
        }
    }
    return c;
}

DNSHelper::DNSHelper() {
    dns4         = "udp://8.8.8.8:53";
    dns6         = "udp://[2001:4860:4860::8888]:53";
    dnsTimeout   = 3000;
    dnsCacheTime = 60000;
    mg_log_set(MG_LL_NONE);
}

DNSHelper::~DNSHelper() { stop(); }

void DNSHelper::start() {
    if (running.load()) return;
    mg_mgr_init(&mgr);
    mg_wakeup_init(&mgr);
    this->_setDNSServer(dns4, dns6);
    this->_setDNSTimeout(dnsTimeout);
    running.store(true);
    dnsRequestThread = std::thread([this]() {
        while (running.load()) {
            mg_mgr_poll(&mgr, 200);
        }
    });
}

void DNSHelper::stop() {
    running.store(false);
    // set a random connection id to wake up the socket
    mg_wakeup(&mgr, 2, nullptr, 0);
    if (dnsRequestThread.joinable()) dnsRequestThread.join();
    mg_mgr_free(&mgr);
}

struct mg_connection *DNSHelper::resolve(const std::string &url, std::function<void(const std::string &)> callback) {
    if (!running.load()) start();
    // todo: dns cache
    return dns_resolve(&mgr, url.c_str(), ev_handler, &callback);
}

void DNSHelper::setDNSServer(const std::string &v4, const std::string &v6) {
    this->cancelAllResolve();
    this->_setDNSServer(v4, v6);
    this->_setDNSTimeout(dnsTimeout);
}

void DNSHelper::setDNSTimeout(int timeout) {
    this->cancelAllResolve();
    this->dnsTimeout = timeout;
    this->_setDNSTimeout(timeout);
    this->_setDNSServer(dns4, dns6);
}

void DNSHelper::setDNSCacheTime(int cache) { this->dnsCacheTime = cache; }

void DNSHelper::cancelAllResolve() {
    if (!running.load()) return;
    mg_mgr_free(&mgr);
}

void DNSHelper::_setDNSServer(const std::string &v4, const std::string &v6) {
    dns4 = v4;
    dns6 = v6;
    // 设置一个空白的 ipv4 dns 会强制使用 ipv6
    if (dns4.empty()) mgr.use_dns6 = true;
    this->mgr.dns4.url = dns4.c_str();
    this->mgr.dns4.c   = nullptr;
    this->mgr.dns6.url = dns6.c_str();
    this->mgr.dns6.c   = nullptr;
}

void DNSHelper::_setDNSTimeout(int timeout) { this->mgr.dnstimeout = timeout; }

bool DNSResult::available() const {
    if (requesting()) return false;
    auto currentTime = brls::getCPUTimeUsec();
    return currentTime < availableTime;
}

bool DNSResult::requesting() const { return _requesting.load(); }