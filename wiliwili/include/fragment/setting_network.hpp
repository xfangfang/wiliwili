//
// Created by fang on 2022/9/19.
//

// register this fragment in main.cpp
//#include "fragment/setting_network.hpp"
//    brls::Application::registerXMLView("SettingNetwork", SettingNetwork::create);
// <brls:View xml=@res/xml/fragment/setting_network.xml

#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

namespace brls {
class Label;
class Header;
}  // namespace brls

class SettingNetwork : public brls::Box {
public:
    SettingNetwork();

    ~SettingNetwork() override;

    void networkTest();

    void getUnixTime();

    static View* create();

private:
    BRLS_BIND(brls::Label, labelTest1, "setting/net/test1");
    BRLS_BIND(brls::Label, labelNetTime, "setting/net/netTime");
    BRLS_BIND(brls::Label, labelSysTime, "setting/net/sysTime");
    BRLS_BIND(brls::Label, labelWIFI, "setting/net/wifi");
    BRLS_BIND(brls::Label, labelIP, "setting/net/ip");
    BRLS_BIND(brls::Label, labelDNS, "setting/net/dns");
    BRLS_BIND(brls::Header, headerTest, "setting/net/test/header");
};