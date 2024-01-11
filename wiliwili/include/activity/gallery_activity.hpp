//
// Created by fang on 2023/4/26.
//

#pragma once

#include <borealis/core/activity.hpp>
#include <borealis/core/bind.hpp>

class GalleryView;
class GalleryActivity : public brls::Activity {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/gallery_activity.xml");
    explicit GalleryActivity(const std::vector<std::string>& data);

    bool isTranslucent() override;

    void onContentAvailable() override;

    ~GalleryActivity() override;

private:
    BRLS_BIND(GalleryView, gallery, "hint/gallery");
    std::vector<std::string> galleryData;
};