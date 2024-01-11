//
// Created by fang on 2022/7/13.
//

#pragma once

#include "bilibili/result/home_live_result.h"
#include "presenter/presenter.h"

class HomeLiveRequest : public Presenter {
public:
    virtual void onLiveList(const bilibili::LiveVideoListResult& result, int index);
    virtual void onAreaList(const bilibili::LiveFullAreaListResult& result);

    virtual void onError(const std::string& error) = 0;

    /**
     * 获取页面数据
     * 如果指定了参数，则设定当前的主附分区ID，与页号，进行访问
     * 如果没有指定参数，则按照之前的设置访问下一页
     * 默认设置 main sub 为 0 表示全站推荐
     * @param main
     * @param sub
     */
    void requestData(int main = -1, int sub = -1, int page = -1);

    /**
     * 获取指定分区的推荐直播间
     *
     * 整合了关注的直播间的接口
     * 现仅用于 parent_area 与 area 均为 0 时访问，为直播页的默认数据，
     * 获取全站推荐的同时获取关注的正在直播的up主
     *
     */
    void requestLiveList(int parent_area, int area, int page);

    /**
     * 获取指定分区的推荐直播间
     * @param parent_area 一级分区ID
     * @param area 二级分区ID
     * @param page 从 1 开始的页号，默认一页加载 20 项
     */
    void requestLiveListV2(int parent_area, int area, int page);

    /**
     * 获取分区列表
     */
    void requestAreaList();

protected:
    bilibili::LiveFullAreaListResult fullAreaList;

    int staticMain  = 0;
    int staticSub   = 0;
    int staticPage  = 0;
    int staticEntry = 0;
};