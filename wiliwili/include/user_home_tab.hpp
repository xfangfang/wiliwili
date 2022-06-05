//
// Created by fang on 2022/6/1.
//

#pragma once


#include <borealis.hpp>
#include "activity/video_detail_activity.hpp"
#include "presender/user_home.hpp"
#include "view/video_grid.hpp"
#include "view/video_card.hpp"

using namespace brls;

class UserHomeTab : public brls::Box, public UserHome
{
public:
    UserHomeTab(){
        this->inflateFromXMLRes("xml/tabs/user_home.xml");
        this->requestData();
        this->registerAction("refresh", ControllerButton::BUTTON_Y, [this](brls::View* view)-> bool {
//            this->requestRecommendVideoList();
            return true;
        });

        BRLS_REGISTER_CLICK_BY_ID("user_home/goto_userspace", [](View *) -> bool{
            auto dialog = new brls::Dialog((brls::Box*)brls::View::createFromXMLResource("tabs/user_home_qr_login.xml"));
            dialog->addButton("Cancel", [](){});
            dialog->open();


            return true;
        });

    }

    ~UserHomeTab(){
        Logger::error("del UserHomeTab");
    }


    static brls::View* create(){
        return new UserHomeTab();
    }

private:
    BRLS_BIND(VideoGrid, videoGrid, "user_home/video_grid");
    BRLS_BIND(brls::ScrollingFrame, videoGridScrollingFrame, "user_home/video_scroll");

};