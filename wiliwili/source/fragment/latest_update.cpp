//
// Created by fang on 2022/11/19.
//

#include "fragment/latest_update.hpp"

LatestUpdate::LatestUpdate(std::string name, std::string content,
                           std::string author_name, std::string author_avatar,
                           std::string publish_date) {
    this->inflateFromXMLRes("xml/fragment/latest_update.xml");
    brls::Logger::debug("Fragment LatestUpdate: create");

    header->setText(name);
    label->setText(content);
    
    author->setUserInfo(author_avatar, author_name, publish_date);
}

LatestUpdate::~LatestUpdate() {
    brls::Logger::debug("Fragment LatestUpdate: delete");
}
