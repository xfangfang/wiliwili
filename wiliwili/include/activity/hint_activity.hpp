//
// Created by fang on 2022/8/21.
//

#pragma once

#include <borealis/core/activity.hpp>
#include <borealis/core/bind.hpp>

class GalleryView;

class HintActivity : public brls::Activity {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/hint_activity.xml");

    HintActivity();

    void onContentAvailable() override;

    ~HintActivity();

private:
    BRLS_BIND(GalleryView, gallery, "hint/gallery");
};