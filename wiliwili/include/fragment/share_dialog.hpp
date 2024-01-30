#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>
#include <borealis/core/event.hpp>
#include <api/bilibili/result/video_detail_result.h>

namespace brls {
class Label;
}  // namespace brls

class SVGImage;
class QRImage;
class ButtonClose;
class ShareBox : public brls::Box {
public:
    ShareBox();

    void setLink(const std::string& value) { this->link = value; }

    void setAction(const std::string& value) { this->action = value; }

    brls::VoidEvent* getEvent() { return &this->eventClipboard; }

    static View* create();

private:
    BRLS_BIND(brls::Label, title, "share/title");
    BRLS_BIND(SVGImage, image, "share/image");

    std::string link;
    std::string action;
    brls::VoidEvent eventClipboard;
};

class ShareDialog : public brls::Box {
public:
    ShareDialog(const std::string& link);
    ShareDialog(const bilibili::VideoDetailResult& result);
    ~ShareDialog() override;

private:
    BRLS_BIND(QRImage, qrcode, "share/qrcode");
    BRLS_BIND(brls::Box, boxShare, "share/box");
    BRLS_BIND(brls::Box, boxHint, "share/hint/box");
    BRLS_BIND(ShareBox, qq, "share/box/qq");
    BRLS_BIND(ShareBox, qzone, "share/box/qzone");
    BRLS_BIND(ShareBox, tieba, "share/box/tieba");
    BRLS_BIND(ShareBox, wechat, "share/box/wechat");
    BRLS_BIND(ShareBox, weibo, "share/box/weibo");
    BRLS_BIND(ShareBox, dynamic, "share/box/dynamic");
};