#pragma once

#include <borealis.hpp>
#include "net_image.hpp"

class HorizontalListContentView : public brls::BoxLayout
{
  public:
    static size_t shareFocusedIndex;
    HorizontalListContentView(brls::List* list, size_t defaultFocus)
        : BoxLayout(brls::BoxLayoutOrientation::HORIZONTAL, defaultFocus)
        , list(list){
        brls::Style* style = brls::Application::getStyle();
        this->setMargins(style->List.marginTopBottom, style->List.marginLeftRight, style->List.marginTopBottom, style->List.marginLeftRight);
        this->setSpacing(style->List.spacing);
        this->setRememberFocus(true);
    }

    void willDisappear(bool resetState){
        for (brls::BoxLayoutChild* child : this->children)
            child->view->willDisappear(resetState);

        // Reset default focus to original one if needed
        if (this->rememberFocus)
            HorizontalListContentView::shareFocusedIndex = this->originalDefaultFocus;
    }

    void onChildFocusGained(brls::View* child){
        this->childFocused = true;
        // Remember focus if needed
        if (this->rememberFocus)
        {
            size_t index              = *((size_t*)child->getParentUserData());
            HorizontalListContentView::shareFocusedIndex = index;
        }

        View::onChildFocusGained(child);
    }

    brls::View* getDefaultFocus(){
        // Focus default focus first
        if (HorizontalListContentView::shareFocusedIndex < this->children.size()){
            View* newFocus = this->children[HorizontalListContentView::shareFocusedIndex]->view->getDefaultFocus();

            if (newFocus)
                return newFocus;
        }

        // Fallback to finding the last focusable view
        for (size_t i = this->children.size(); i >0;  i--){
            View* newFocus = this->children[i-1]->view->getDefaultFocus();

            if (newFocus)
                return newFocus;
        }

        return nullptr;
    }

  private:
    brls::List* list;
};

size_t HorizontalListContentView::shareFocusedIndex = 0;

class VideoListItem : public brls::ListItem
{
    public:
        VideoListItem(std::string name, std::string title, std::string thumbnail, std::string avatar)
            :brls::ListItem(name){
            this->setTextSize(10);
            this->setThumbnail(thumbnail);
            this->setAvatar(avatar);
            this->titleView = new brls::Label(brls::LabelStyle::REGULAR, title, true);
            this->titleView->setParent(this);
            this->titleView->setLineHeight(1);
            this->titleView->setFontSize(14);
            #ifdef __PLATFORM_LINUX__
                this->titleView->setLineHeight(0.7);
            #endif
        }

        ~VideoListItem(){
            if (this->titleView)
                delete this->titleView;

            if (this->thumbnailView)
                delete this->thumbnailView;

            if (this->avatarView)
                delete this->avatarView;

            this->resetValueAnimation();
        }

        void setAvatar(std::string imagePath){
            if (this->avatarView)
                this->avatarView->setImage(imagePath);
            else
                this->avatarView = new NetImage(imagePath);

            this->avatarView->setParent(this);
            this->avatarView->setScaleType(brls::ImageScaleType::SCALE);
            this->avatarView->setCornerRadius(100);
            this->invalidate();
        }

        void setThumbnail(std::string imagePath){
            if (this->thumbnailView)
                this->thumbnailView->setImage(imagePath);
            else
                this->thumbnailView = new NetImage(imagePath);

            this->thumbnailView->setParent(this);
            this->thumbnailView->setCornerRadius(2);
            this->thumbnailView->setScaleType(brls::ImageScaleType::SCALE);
            this->invalidate();
        }

        void layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash) override {
            // Description
            if (this->titleView){
                this->titleView->setBoundaries(this->x + this->height * 0.3 , this->y + this->height * 0.7, this->width - this->height * 0.3, this->height * 0.3);
                this->titleView->invalidate(true); // we must call layout directly
            }

            // Thumbnail
            if (this->thumbnailView){
                unsigned thumbnailWidth = this->width;
                unsigned thumbnailHeight = this->height * 0.7;
                this->thumbnailView->setBoundaries(
                    x ,
                    y ,
                    thumbnailWidth,
                    thumbnailHeight);
                this->thumbnailView->invalidate();
            }

            // Avatar
            if (this->avatarView){
                unsigned avatarSize = this->height * 0.25;
                this->avatarView->setBoundaries(
                    x + this->height * 0.025,
                    y + this->height * 0.725,
                    avatarSize,
                    avatarSize);
                this->avatarView->invalidate();
            }
        }

        void getHighlightInsets(unsigned* top, unsigned* right, unsigned* bottom, unsigned* left) override {
            brls::View::getHighlightInsets(top, right, bottom, left);
        }
        void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override {
            unsigned baseHeight = this->height;
            bool hasSubLabel    = this->subLabel != "";

            if (this->indented)
            {
                x += style->List.Item.indent;
                width -= style->List.Item.indent;
            }

            // Description
            if (this->titleView)
            {
                // Don't count description as part of list item
                baseHeight -= this->titleView->getHeight() + style->List.Item.descriptionSpacing;
                this->titleView->frame(ctx);
            }

            // Value
            unsigned valueX = x + width - style->List.Item.padding;
            unsigned valueY = y + (hasSubLabel ? baseHeight - baseHeight / 3 : baseHeight / 2);

            nvgTextAlign(vg, NVG_ALIGN_RIGHT | (hasSubLabel ? NVG_ALIGN_TOP : NVG_ALIGN_MIDDLE));
            nvgFontFaceId(vg, ctx->fontStash->regular);
            if (this->valueAnimation != 0.0f)
            {
                //Old value
                NVGcolor valueColor = a(this->oldValueFaint ? ctx->theme->listItemFaintValueColor : ctx->theme->listItemValueColor);
                valueColor.a *= (1.0f - this->valueAnimation);
                nvgFillColor(vg, valueColor);
                nvgFontSize(vg, style->List.Item.valueSize * (1.0f - this->valueAnimation));
                nvgBeginPath(vg);
                nvgText(vg, valueX, valueY, this->oldValue.c_str(), nullptr);

                //New value
                valueColor = a(this->valueFaint ? ctx->theme->listItemFaintValueColor : ctx->theme->listItemValueColor);
                valueColor.a *= this->valueAnimation;
                nvgFillColor(vg, valueColor);
                nvgFontSize(vg, style->List.Item.valueSize * this->valueAnimation);
                nvgBeginPath(vg);
                nvgText(vg, valueX, valueY, this->value.c_str(), nullptr);
            }
            else
            {
                nvgFillColor(vg, a(this->valueFaint ? ctx->theme->listItemFaintValueColor : ctx->theme->listItemValueColor));
                nvgFontSize(vg, style->List.Item.valueSize);
                nvgFontFaceId(vg, ctx->fontStash->regular);
                nvgBeginPath(vg);
                nvgText(vg, valueX, valueY, this->value.c_str(), nullptr);
            }

            // Label
            nvgFillColor(vg, a(ctx->theme->textColor));
            nvgFontSize(vg, this->textSize);
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgBeginPath(vg);
            nvgText(vg, x + height * 0.3 , y + height * 0.95, this->label.c_str(), nullptr);

            // Thumbnail
            if (this->thumbnailView)
                this->thumbnailView->frame(ctx);

            // Avatar
            if (this->avatarView)
                this->avatarView->frame(ctx);

            // Separators
            // Offset by one to be hidden by highlight
            // nvgFillColor(vg, a(ctx->theme->listItemSeparatorColor));

            // Top
            // if (this->drawTopSeparator)
            // {
            //     nvgBeginPath(vg);
            //     nvgRect(vg, x, y - 1, width, 1);
            //     nvgFill(vg);
            // }

            // // Bottom
            // nvgBeginPath(vg);
            // nvgRect(vg, x, y + 1 + baseHeight, width, 1);
            // nvgFill(vg);
        }
        int getIndex(){
            return this->index;
        }
        void setIndex(int i){
            this->index = i;
        }
        bool onClick(){
            return this->clickEvent.fire(this);
        }

        brls::Event<VideoListItem *>* getClickEvent(){
            return &this->clickEvent;
        }
    private:
        NetImage* avatarView      = nullptr;
        NetImage* thumbnailView   = nullptr;
        brls::Label* titleView    = nullptr;
        int index = 0;
        brls::Event<VideoListItem *> clickEvent;


};

template <class itemType>
class GridList : public brls::List
{
  public:

    GridList(int columns,int itemWidth, int itemHeight, int spacing, std::function<VideoListItem*(itemType)> createItem){
        this->columns = columns;
        this->itemWidth = itemWidth;
        this->itemHeight = itemHeight;
        this->spacing = spacing;
        this->createItem = createItem;
        
        this->setSpacing(spacing);
        this->setMargins(0,0,0,0);

    }

    void addListData(std::vector<itemType> list){
        for(auto item: list){
            this->itemsNeededAdd.push(item);
        }
        this->listData.insert(this->listData.end(), list.begin(), list.end());
    }

    void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override {
        if(!this->itemsNeededAdd.empty()){
            while(!this->itemsNeededAdd.empty()){
                this->addItem(this->createItem(this->itemsNeededAdd.front()));
                this->itemsNeededAdd.pop();
            }
            this->invalidate(true);
        }
        ScrollView::draw(vg, x, y, width, height, style, ctx);
    }

    void addItem(VideoListItem * view){
        if(!view) return;
        if (this->itemNums % this->columns == 0){
            HorizontalListContentView *h = new HorizontalListContentView(this, 0);
            h->setSpacing(this->spacing);
            h->setMargins(0,0,0,0);
            h->setHeight(this->itemHeight);
            listRows.push_back(h);
            this->addView(h);
            // this->invalidate(true);
        }
        HorizontalListContentView *row = listRows[listRows.size()-1];
        view->getClickEvent()->subscribe([this](VideoListItem* view){
            this->clickEvent.fire(this->listData[view->getIndex()],view);
        });
        view->setIndex(this->itemNums);
        row->addView(view);
        this->itemNums++;
    }

    brls::Event<itemType, VideoListItem*>* getClickEvent(){
        return &this->clickEvent;
    }

    private:
        int columns;
        int itemHeight;
        int itemWidth;
        int itemNums = 0;
        int spacing;
        std::vector<HorizontalListContentView*> listRows;
        std::queue<itemType> itemsNeededAdd;
        std::vector<itemType> listData;
        std::function<VideoListItem*(itemType)> createItem;
        brls::Event<itemType, VideoListItem*> clickEvent;
};