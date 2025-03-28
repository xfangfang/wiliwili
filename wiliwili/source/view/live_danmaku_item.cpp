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
    
    // 首先根据用户等级设置不同的颜色
    int level = danmaku.danmaku->user_level;
    NVGcolor levelColor;
    
    if (level >= 0 && level <= 60) {
        // 根据等级设置不同的颜色
        if (level >= 0 && level <= 5) {
            // 新手色 - 浅绿色
            levelColor = nvgRGBA(137, 207, 127, 255);
        } else if (level <= 10) {
            // 初级色 - 绿色
            levelColor = nvgRGBA(95, 179, 86, 255);
        } else if (level <= 20) {
            // 中级色 - 蓝色
            levelColor = nvgRGBA(92, 179, 239, 255);
        } else if (level <= 30) {
            // 高级色 - 紫色
            levelColor = nvgRGBA(172, 117, 243, 255);
        } else if (level <= 40) {
            // 资深色 - 橙色
            levelColor = nvgRGBA(255, 163, 72, 255);
        } else if (level <= 50) {
            // 专家色 - 红色
            levelColor = nvgRGBA(255, 102, 102, 255);
        } else {
            // 大神色 - 金色
            levelColor = nvgRGBA(255, 215, 0, 255);
        }
        this->levelBox->setBackgroundColor(levelColor);
    } else {
        // 根据弹幕类型设置不同的背景色（保留原有的逻辑）
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
}

LiveDanmakuItemView* LiveDanmakuItemView::create() {
    return new LiveDanmakuItemView();
} 