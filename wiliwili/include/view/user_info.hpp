//
// Created by fang on 2022/5/13.
//

#pragma once

#include <borealis.hpp>
#include "utils/image_helper.hpp"


using namespace brls;
class UserInfoView : public brls::Box {

public:
    UserInfoView(){
        this->inflateFromXMLRes("xml/views/user_info.xml");
    }

    void setUserInfo(std::string avatar, std::string username, std::string misc){
        this->labelUsername->setText(username);
        this->labeMisc->setText(misc);
        ImageHelper::with(this)->load(avatar)->into(this->avatarView);
    }

    static brls::View* create(){
        return new UserInfoView();
    }

    brls::Image* getAvatar(){
        return this->avatarView;
    }

private:
    BRLS_BIND(brls::Image, avatarView, "avatar");
    BRLS_BIND(Label, labelUsername, "username");
    BRLS_BIND(Label, labeMisc, "misc");
};