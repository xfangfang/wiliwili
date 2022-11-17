//
// Created by fang on 2022/7/22.
//

// Thanks to XITRIX's patch to borealis
// https://github.com/XITRIX/borealis/commit/c309f22bd451aaf59fbbb15c5f2345104582e8a8

#pragma once

// 检查异步返回时组件是否已经被销毁
#define ASYNC_RETAIN                               \
    if (!deletionToken && !deletionTokenCounter) { \
        deletionToken        = new bool(false);    \
        deletionTokenCounter = new int(0);         \
    }                                              \
    (*deletionTokenCounter)++;                     \
    bool* token       = deletionToken;             \
    int* tokenCounter = deletionTokenCounter;

#define ASYNC_RELEASE                           \
    bool release = *token;                      \
    int counter  = *tokenCounter;               \
    if (counter > 0) {                          \
        (*tokenCounter)--;                      \
        if (*tokenCounter == 0) {               \
            delete token;                       \
            delete tokenCounter;                \
            if (!release) {                     \
                deletionToken        = nullptr; \
                deletionTokenCounter = nullptr; \
            }                                   \
        }                                       \
    }                                           \
    if (release) return;

#define ASYNC_TOKEN this, token, tokenCounter

// 避免多次请求API
#define CHECK_REQUEST \
    if (requesting) return;
#define SET_REQUEST requesting = true;
#define CHECK_AND_SET_REQUEST CHECK_REQUEST SET_REQUEST
#define UNSET_REQUEST requesting = false;

class Presenter {
public:
    virtual ~Presenter() {
        if (deletionToken) *deletionToken = true;
    }

protected:
    bool* deletionToken       = nullptr;
    int* deletionTokenCounter = nullptr;
    bool requesting           = false;
};