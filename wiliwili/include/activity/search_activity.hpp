/**
 * Created by fang on 2022/6/9.
 */

#pragma once

#include <borealis/core/activity.hpp>
#include <borealis/core/bind.hpp>

#include "fragment/search_interface.hpp"

namespace brls {
class Label;
class Box;
}  // namespace brls
class SearchTab;

typedef brls::Event<std::string> UpdateSearchEvent;

class SearchActivity : public brls::Activity, public SearchEventInterface {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/search_activity.xml");

    explicit SearchActivity(const std::string &key = "");

    void onContentAvailable() override;

    ~SearchActivity() override;

    void requestSearch(const std::string &key) override;

    UpdateSearchEvent *getUpdateSearchEvent();

    inline static std::string currentKey;

private:
    BRLS_BIND(brls::Label, labelSearchKey, "search/label/key");
    BRLS_BIND(SearchTab, searchTab, "search/tab");
    BRLS_BIND(brls::Box, searchBox, "search/box");

    void search(const std::string &key);

    UpdateSearchEvent updateSearchEvent;
};