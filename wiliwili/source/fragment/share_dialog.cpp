#include <borealis/core/i18n.hpp>
#include <borealis/core/application.hpp>
#include <borealis/core/util.hpp>
#include <borealis/core/touch/tap_gesture.hpp>
#include <cpr/cpr.h>

#if defined(__APPLE__) || defined(__linux__) || defined(_WIN32)
#include <clip/clip.h>
#endif

#include "fragment/share_dialog.hpp"
#include "view/qr_image.hpp"
#include "view/button_close.hpp"

ShareBox::ShareBox() {
    this->inflateFromXMLRes("xml/fragment/share_box.xml");
    this->registerStringXMLAttribute("title", [this](const std::string& value) { this->title->setText(value); });
    this->registerFilePathXMLAttribute("icon",
                                       [this](const std::string& value) { this->image->setImageFromSVGFile(value); });
    this->registerStringXMLAttribute("action", [this](const std::string& value) { this->setAction(value); });

#if defined(__APPLE__) || defined(__linux__) || defined(_WIN32)
    this->registerClickAction([this](...) {
        if (this->action == "clipboard") {
            clip::set_text(this->link);
            this->eventClipboard.fire();
        } else {
            brls::Application::getPlatform()->openBrowser(this->link);
            brls::Application::popActivity();
        }
        return true;
    });
    this->addGestureRecognizer(new brls::TapGestureRecognizer(this));
#endif
}

brls::View* ShareBox::create() { return new ShareBox(); }

ShareDialog::ShareDialog(const std::string& link) {
    this->inflateFromXMLRes("xml/fragment/share_dialog.xml");
    brls::Logger::debug("Fragment ShareDialog: create");
    this->qrcode->setImageFromQRContent(link);
    this->boxShare->setVisibility(brls::Visibility::GONE);
}

ShareDialog::ShareDialog(const bilibili::VideoDetailResult& result) {
    this->inflateFromXMLRes("xml/fragment/share_dialog.xml");
    brls::Logger::debug("Fragment ShareDialog: create");

    std::string link = "https://www.bilibili.com/video/" + result.bvid;
    this->qrcode->setImageFromQRContent(link);

    std::string desc = fmt::format("{} UP: {}", result.title, result.owner.name);

    this->qq->setLink(
        fmt::format("https://connect.qq.com/widget/shareqq/index.html?"
                    "url={}&title={}&desc={}&summary={}&pics={}&style=201&width=32&height=32",
                    cpr::util::urlEncode(link), cpr::util::urlEncode(result.title), cpr::util::urlEncode(desc),
                    cpr::util::urlEncode(result.desc), cpr::util::urlEncode(result.pic)));
    this->qzone->setLink(
        fmt::format("https://sns.qzone.qq.com/cgi-bin/qzshare/cgi_qzshare_onekey?"
                    "url={}&showcount=1&title={}&desc={}&summary={}&pics={}",
                    cpr::util::urlEncode(link), cpr::util::urlEncode(result.title), cpr::util::urlEncode(desc),
                    cpr::util::urlEncode(result.desc), cpr::util::urlEncode(result.pic)));
    this->tieba->setLink(
        fmt::format("http://tieba.baidu.com/f/commit/share/openShareApi?"
                    "url={}&title={}&uid=726865&to=tieba&type=text&comment={}&pic={}",
                    cpr::util::urlEncode(link), cpr::util::urlEncode(result.title), cpr::util::urlEncode(result.desc),
                    cpr::util::urlEncode(result.pic)));
    this->weibo->setLink(
        fmt::format("https://service.weibo.com/share/share.php?"
                    "url={}&type=3&count=1&appkey=2841902482&title={}&pic={}&language=zh_cn",
                    cpr::util::urlEncode(link), cpr::util::urlEncode(result.title), cpr::util::urlEncode(result.pic)));

    this->boxHint->hide([]() {}, false, 0);
    this->boxHint->setVisibility(brls::Visibility::VISIBLE);

    this->dynamic->setLink(link);
    this->dynamic->getEvent()->subscribe([this]() { this->boxHint->show([this]() { this->boxHint->hide([]() {}); }); });
    this->wechat->setLink(link);
    this->wechat->getEvent()->subscribe([this]() { this->boxHint->show([this]() { this->boxHint->hide([]() {}); }); });
}

ShareDialog::~ShareDialog() { brls::Logger::debug("Fragment ShareDialog: delete"); }