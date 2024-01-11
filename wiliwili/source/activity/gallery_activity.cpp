//
// Created by fang on 2023/4/26.
//

#include <borealis/views/image.hpp>

#include "activity/gallery_activity.hpp"
#include "view/gallery_view.hpp"
#include "utils/image_helper.hpp"

const std::string ImageGalleryItemXML = R"xml(
    <brls:Box
        width="100%"
        height="100%"
        axis="row"
        grow="1"
        justifyContent="center"
        alignItems="center">

        <brls:Image
                margin="40"
                id="gallery/image"/>
    </brls:Box>
)xml";

class NetImageGalleryItem : public GalleryItem {
public:
    explicit NetImageGalleryItem(const std::string& url) {
        this->inflateFromXMLString(ImageGalleryItemXML);
        this->image->setImageFromRes("icon/bilibili_video.png");
        ImageHelper::with(this->image)->load(url);
    }
    ~NetImageGalleryItem() override { ImageHelper::clear(this->image); }

private:
    BRLS_BIND(brls::Image, image, "gallery/image");
};

GalleryActivity::GalleryActivity(const std::vector<std::string>& data) : galleryData(data) {
    brls::Logger::debug("GalleryActivity: create");
}

void GalleryActivity::onContentAvailable() {
    brls::Logger::debug("GalleryActivity: onContentAvailable");
    gallery->setIndicatorPosition(0.97);
    for (auto& i : galleryData) {
        gallery->addCustomView(new NetImageGalleryItem(i));
    }
}

GalleryActivity::~GalleryActivity() { brls::Logger::debug("GalleryActivity: delete"); }

bool GalleryActivity::isTranslucent() { return true; }
