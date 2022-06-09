/**
 * Created by fang on 2022/6/9.
 */

#pragma once

#include <borealis.hpp>

class SearchActivity : public brls::Activity {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/search_activity.xml");

    SearchActivity();

    void onContentAvailable() override;

    ~SearchActivity();
};