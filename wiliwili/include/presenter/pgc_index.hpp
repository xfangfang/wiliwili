//
// Created by fang on 2022/8/24.
//

#pragma once

#include "bilibili/result/home_pgc_result.h"
#include "presenter/presenter.h"

typedef std::pair<std::string, std::string> IndexItemSinglePairData;  //键值对
typedef std::vector<IndexItemSinglePairData> IndexItemPairData;  // 键值对列表：组成检索表单中的一行
typedef std::map<std::string, std::string> UserRequestData;      // 用户当前请求数据

class PGCIndexRequest : public Presenter {
public:
    virtual void onPGCIndex(const bilibili::PGCIndexResultWrapper& result);

    virtual void onPGCFilter(const bilibili::PGCIndexFilters& result);

    virtual void onError(const std::string& error);

    void requestData(UserRequestData data, bool refresh = false);

    void requestPGCIndex(const std::string& param, int page = 1);

    void requestPGCFilter();

    static inline bilibili::PGCIndexFilters INDEX_FILTERS;

protected:
    int requestIndex = 1;
};