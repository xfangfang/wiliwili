//
// Created by fang on 2022/6/9.
//

#include "fragment/mine_tab.hpp"
#include "fragment/mine_qr_login.hpp"
#include "utils/image_helper.hpp"
#include "utils/config_helper.hpp"
#include "utils/number_helper.hpp"

#include "fragment/mine_collection.hpp"
#include "fragment/mine_history.hpp"

using namespace brls;
using namespace brls::literals;

MineTab::MineTab() {
    this->inflateFromXMLRes("xml/fragment/mine_tab.xml");
    brls::Logger::debug("Fragment MineTab: create");

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

    // 初始化用户区域内容
    this->onUserNotLogin();
    // 在用户登录的情况获取到之前先清空点击事件
    boxGotoUserSpace->registerClickAction([](...) -> bool { return true;});
    this->requestData();
}

void MineTab::onCreate() {
    this->registerTabAction("刷新个人信息", brls::ControllerButton::BUTTON_X, [this](brls::View* view)-> bool {
        this->requestData();
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
        dialog->addButton("hints/cancel"_i18n, [](){});
        dialog->open();
        return true;
    }, false, false, SOUND_CLICK);

    ASYNC_RETAIN
    brls::sync([ASYNC_TOKEN](){
        ASYNC_RELEASE
        labelUserName->setText("点击登录");
        imageUserAvater->setImageFromRes("pictures/default_avatar.png");
        labelSign->setText("");
        labelCoins->setText("0");
        labelFollower->setText("0");
        labelFollowing->setText("0");
    });

}

void MineTab::onUserInfo(const bilibili::UserResult& data) {
    boxGotoUserSpace->registerAction("hints/ok"_i18n, BUTTON_A, [](View *) -> bool{
        auto dialog = new Dialog("退出登录当前的账户 (应用将会退出或重启)");
        dialog->addButton("hints/back"_i18n, [](){});
        dialog->addButton("hints/ok"_i18n, [](){
            ProgramConfig::instance().setCookie({});
            ConfigHelper::saveProgramConf();
            brls::Application::getPlatform()->exitToHomeMode(false);
            Application::quit();
        });
        dialog->open();
        return true;
    }, false, false, SOUND_CLICK);

    ASYNC_RETAIN
    brls::sync([ASYNC_TOKEN, data](){
        ASYNC_RELEASE
        labelUserName->setText(data.name);
        ImageHelper::with(this)->load(data.face)->into(imageUserAvater);
        labelSign->setText(data.sign);
        labelCoins->setText(fmt::format("{}", data.coins));
    });
}

void MineTab::onUserDynamicStat(const bilibili::UserDynamicCount& result){
    ASYNC_RETAIN
    brls::sync([ASYNC_TOKEN, result]() {
        ASYNC_RELEASE
        std::string mid = ProgramConfig::instance().getUserID();
        if(result.data.count(mid) != 0)
            this->labelDynamic->setText(wiliwili::num2w(result.data.at(mid)));
    });
}

void MineTab::onUserRelationStat(const bilibili::UserRelationStat& result){
    ASYNC_RETAIN
    brls::sync([ASYNC_TOKEN, result]() {
        ASYNC_RELEASE
        this->labelFollower->setText(wiliwili::num2w(result.follower));
        this->labelFollowing->setText(wiliwili::num2w(result.following));
    });
}

View* MineTab::create() {
    return new MineTab();
}