/**
 * Created by fang on 2022/6/9.
 */

#pragma once

#include <borealis.hpp>

class SearchTab;

typedef brls::Event<std::string> UpdateSearchEvent;

class SearchActivity : public brls::Activity {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/search_activity.xml");

    SearchActivity(const std::string &key = "");

    void onContentAvailable() override;

    ~SearchActivity();

    void search(const std::string &key);

    UpdateSearchEvent *getUpdateSearchEvent();

    inline static std::string currentKey = "";
private:

    BRLS_BIND(brls::Label, labelSearchKey, "search/label/key");
    BRLS_BIND(SearchTab, searchTab, "search/tab");
    UpdateSearchEvent updateSearchEvent;

};