#include <borealis/core/i18n.hpp>
#include <borealis/core/application.hpp>
#include <borealis/core/thread.hpp>
#include <borealis/core/util.hpp>
#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/views/dialog.hpp>
#include <cpr/cpr.h>

#include "fragment/share_dialog.hpp"
#include "view/qr_image.hpp"
#include "view/button_close.hpp"

using namespace brls::literals;

ShareBox::ShareBox() {
    this->inflateFromXMLRes("xml/fragment/share_box.xml");
    this->registerStringXMLAttribute("title", [this](const std::string& value) { this->title->setText(value); });
    this->registerFilePathXMLAttribute("icon",
                                       [this](const std::string& value) { this->image->setImageFromSVGFile(value); });
    this->registerStringXMLAttribute("action", [this](const std::string& value) { this->setAction(value); });

#if defined(__APPLE__) || defined(__linux__) || defined(_WIN32)
    this->registerClickAction([this](...) {
        if (this->action == "clipboard") {
            brls::Application::getPlatform()->pasteToClipboard(this->link);
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

ShareDialog::ShareDialog() {
    this->inflateFromXMLRes("xml/fragment/share_dialog.xml");
    brls::Logger::debug("Fragment ShareDialog: create");
}

void ShareDialog::open(const std::string& link) {
    this->qrcode->setImageFromQRContent(link);
    this->qrcode->setCustomNavigationRoute(brls::FocusDirection::RIGHT, this->qrcode);
    this->boxShare->setVisibility(brls::Visibility::GONE);
    auto dialog = new brls::Dialog(this);
    dialog->getAppletFrame()->setWidth(350);
    dialog->open();
}

void ShareDialog::open(const std::string& link, const std::string& title, const std::string& desc,
                       const std::string& pic, const std::string& uploader) {
    this->qrcode->setImageFromQRContent(link);

    std::string content = uploader;
    if (!uploader.empty())
        content = fmt::format("{} UP: {}", title, uploader);

    this->qq->setLink(
        fmt::format("https://connect.qq.com/widget/shareqq/index.html?"
                    "url={}&desc={}&title={}&summary={}&pics={}&style=201&width=32&height=32",
                    cpr::util::urlEncode(link), cpr::util::urlEncode(content), cpr::util::urlEncode(title), cpr::util::urlEncode(desc),
                    cpr::util::urlEncode(pic)));
    this->qzone->setLink(
        fmt::format("https://sns.qzone.qq.com/cgi-bin/qzshare/cgi_qzshare_onekey?"
                    "url={}&showcount=1&desc={}&title={}&summary={}&pics={}",
                    cpr::util::urlEncode(link), cpr::util::urlEncode(content), cpr::util::urlEncode(title),
                    cpr::util::urlEncode(desc), cpr::util::urlEncode(pic)));
    this->tieba->setLink(
        fmt::format("http://tieba.baidu.com/f/commit/share/openShareApi?"
                    "url={}&title={}&uid=726865&to=tieba&type=text&comment={}&pic={}",
                    cpr::util::urlEncode(link), cpr::util::urlEncode(title), cpr::util::urlEncode(content),
                    cpr::util::urlEncode(pic)));
    this->weibo->setLink(
        fmt::format("https://service.weibo.com/share/share.php?"
                    "url={}&type=3&count=1&appkey=2841902482&title={}&pic={}&language=zh_cn",
                    cpr::util::urlEncode(link), cpr::util::urlEncode(title + "#哔哩哔哩动画#"), cpr::util::urlEncode(pic)));

    this->dynamic->setLink(link);
    this->dynamic->getEvent()->subscribe([this]() { this->showHint(); });
    this->wechat->setLink(link);
    this->wechat->getEvent()->subscribe([this]() { this->showHint(); });

    auto dialog = new brls::Dialog(this);
    dialog->open();
}

ShareDialog::~ShareDialog() { brls::Logger::debug("Fragment ShareDialog: delete"); }

void ShareDialog::showHint() {
    brls::Application::notify("wiliwili/player/clipboard"_i18n);
}