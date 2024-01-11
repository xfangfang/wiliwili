//
// Created by fang on 2023/4/20.
//

#pragma once

#include <borealis/core/activity.hpp>
#include <borealis/core/bind.hpp>

#include "presenter/presenter.h"

namespace brls {
class Label;
}
class RecyclingGrid;
class SearchHots;
class SearchHistory;
class SVGImage;

class TVSearchActivity : public brls::Activity, public Presenter {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/search_activity_tv.xml");
    TVSearchActivity();

    void onContentAvailable() override;

    void requestSearchSuggest();

    void updateInputLabel();

    void setCurrentSearch(const std::string& value);

    void search(const std::string& key);

    std::string getCurrentSearch();

    ~TVSearchActivity() override;

    void onResume() override;

    inline static bool TV_MODE = false;

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "tv/search/keyboard");
    BRLS_BIND(brls::Label, inputLabel, "tv/search/input");
    BRLS_BIND(brls::Label, clearLabel, "tv/search/clear");
    BRLS_BIND(brls::Label, deleteLabel, "tv/search/delete");
    BRLS_BIND(brls::Label, searchLabel, "tv/search/search");
    BRLS_BIND(brls::Label, hotsHeaderLabel, "tv/search/label/hots");
    BRLS_BIND(SearchHots, searchHots, "tv/search/hots");
    BRLS_BIND(SearchHistory, searchHistory, "tv/search/history");
    BRLS_BIND(SVGImage, searchSVG, "tv/search/svg");

    brls::Event<std::string> updateSearchEvent;
    std::wstring currentSearch;
};