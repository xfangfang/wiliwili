//
// Created by fang on 2022/6/9.
//

#include "fragment/mine_tab.hpp"
#include "fragment/mine_qr_login.hpp"
#include "utils/image_helper.hpp"

#include "fragment/mine_collection.hpp"
#include "fragment/mine_history.hpp"

MineTab::MineTab() {
    this->inflateFromXMLRes("xml/fragment/mine_tab.xml");
    brls::Logger::debug("Fragment MineTab: create");
    this->requestData();

    this->loginCb.subscribe([this](bilibili::LoginInfo status){
        if(status == bilibili::LoginInfo::SUCCESS){
            brls::Logger::debug("登录成功");
            this->requestData();
            try {
                this->mineHistory->requestData(true);
                this->mineCollection->requestData(true);
            } catch (ViewNotFoundException& e) {}
        }
    });

    this->onUserNotLogin();
}

void MineTab::onCreate() {
    this->registerTabAction("刷新个人信息", brls::ControllerButton::BUTTON_X, [this](brls::View* view)-> bool {
        this->requestData();
        //todo: 几个子页面一并刷新
        return true;
    });
    this->boxGotoUserSpace->addGestureRecognizer(new TapGestureRecognizer(this->boxGotoUserSpace));

    this->registerTabAction("上一项", brls::ControllerButton::BUTTON_LB,
                            [this](brls::View* view)-> bool {
                                tabFrame->focus2LastTab();
                                return true;
                            }, true);

    this->registerTabAction("下一项", brls::ControllerButton::BUTTON_RB,
                            [this](brls::View* view)-> bool {
                                tabFrame->focus2NextTab();
                                return true;
                            }, true);
}

MineTab::~MineTab() {
    brls::Logger::debug("Fragment MineTabActivity: delete");
}

void MineTab::onUserNotLogin() {

    boxGotoUserSpace->registerAction("hints/ok"_i18n, BUTTON_A, [this](View *) -> bool{
        auto dialog = new brls::Dialog(MineQrLogin::create(this->loginCb));
        dialog->addButton("Cancel", [](){});
        dialog->open();
        return true;
    }, false, false, SOUND_CLICK);

    ASYNC_RETAIN
    brls::sync([ASYNC_TOKEN](){
        ASYNC_RELEASE
        labelUserName->setText("点击登录");
        imageUserAvater->setImageFromRes("icon/akari.jpg");
    });

}

void MineTab::onUserInfo(const bilibili::UserResult& data) {
    boxGotoUserSpace->registerAction("hints/ok"_i18n, BUTTON_A, [this](View *) -> bool{
        // open user space;
        return true;
    }, false, false, SOUND_CLICK);

    ASYNC_RETAIN
    brls::sync([ASYNC_TOKEN, data](){
        ASYNC_RELEASE
        labelUserName->setText(data.name);
        ImageHelper::with(this)->load(data.face)->into(imageUserAvater);
    });
}

View* MineTab::create() {
    return new MineTab();
}