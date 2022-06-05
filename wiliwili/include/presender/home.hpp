//
// Created by fang on 2022/5/26.
//

#pragma once

#include "bilibili.h"
#include "bilibili/result/home_result.h"

class Home {
public:
    virtual void onRecommendVideoList(const bilibili::RecommendVideoListResult &result){}
    virtual void onError(){}

    void requestData() {
        this->requestRecommendVideoList(1, 24);
    }

    void requestRecommendVideoList(int index = 1, int num = 24) {
        Logger::debug("max threads: {}",CPR_DEFAULT_THREAD_POOL_MAX_THREAD_NUM);
        bilibili::BilibiliClient::get_recommend(index, num,
                                                [this](const bilibili::RecommendVideoListResult &result){
                                                    brls::Threading::sync([this, result]() {
                                                        //todo: 当还没获取到推荐列表时，切换页面会销毁当前窗口，从而导致this不可用
                                                        //解决方案：sidebar切换页面不销毁。
                                                        this->onRecommendVideoList(result);
                                                    });
        }, [](const std::string &error) {
            Logger::error(error);
        });
    }
};
