//
// Created by fang on 2024/3/22.
//

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

#pragma once

namespace brls {
class Label;
class Image;
}  // namespace brls

class DynamicVideoCardView : public brls::Box {
public:
    DynamicVideoCardView();

    void setCard(const std::string& cover, const std::string& title, const std::string& count,
                 const std::string& danmaku, const std::string& duration);

    static brls::View* create();

    // 视频封面
    BRLS_BIND(brls::Image, picture, "video/card/picture");
    // 视频标题
    BRLS_BIND(brls::Label, labelTitle, "dynamic/title");
    // 播放信息
    BRLS_BIND(brls::Label, labelCount, "video/card/label/count");
    BRLS_BIND(brls::Label, labelDanmaku, "video/card/label/danmaku");
    BRLS_BIND(brls::Label, labelDuration, "video/card/label/duration");
};
