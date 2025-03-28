#include "view/live_danmaku_item.hpp"

// using namespace brls::literals;

LiveDanmakuItemView::LiveDanmakuItemView() {
    this->inflateFromXMLRes("xml/views/live_danmaku_item.xml");
}

void LiveDanmakuItemView::setDanmaku(const LiveDanmakuItem& danmaku) {
    // 设置用户名
    this->usernameLabel->setText(danmaku.danmaku->user_name ? danmaku.danmaku->user_name : "用户");
    
    // 设置弹幕内容
    this->contentLabel->setText(danmaku.danmaku->dan ? danmaku.danmaku->dan : "");
    
    // 设置用户等级
    this->levelLabel->setText("UL" + std::to_string(danmaku.danmaku->user_level));
    
    // 根据弹幕类型设置不同的背景色
    switch (danmaku.danmaku->dan_type) {
        case 0:  // 普通
            this->levelBox->setBackgroundColor(nvgRGBA(255, 102, 153, 255));
            break;
        case 1:  // 系统
            this->levelBox->setBackgroundColor(nvgRGBA(254, 153, 0, 255));
            break;
        case 2:  // 礼物
            this->levelBox->setBackgroundColor(nvgRGBA(102, 204, 51, 255));
            break;
        default:
            this->levelBox->setBackgroundColor(nvgRGBA(255, 102, 153, 255));
    }
}

LiveDanmakuItemView* LiveDanmakuItemView::create() {
    return new LiveDanmakuItemView();
} 