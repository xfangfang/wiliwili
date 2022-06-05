//
// Created by fang on 2022/5/13.
//

#pragma once

#include <borealis.hpp>
#include "net_image.hpp"


using namespace brls;
class UserInfoView : public brls::Box {

public:
    UserInfoView(){
        this->inflateFromXMLRes("xml/views/user_info.xml");
    }

    void setUserInfo(std::string avatar, std::string username, std::string misc){
        this->labelUsername->setText(username);
        this->labeMisc->setText(misc);
        this->avattarView->setImageFromNet(avatar);
    }

    static brls::View* create(){
        return new UserInfoView();
    }

private:
    BRLS_BIND(NetImage, avattarView, "avatar");
    BRLS_BIND(Label, labelUsername, "username");
    BRLS_BIND(Label, labeMisc, "misc");
};