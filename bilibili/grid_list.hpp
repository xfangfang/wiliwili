#pragma once

#include <borealis.hpp>
#include "views/net_image.hpp"

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

class GridListItem : public brls::ListItem
{
    public:
        GridListItem(std::string title):brls::ListItem(title){

        }

        void getHighlightInsets(unsigned* top, unsigned* right, unsigned* bottom, unsigned* left) override {
            brls::View::getHighlightInsets(top, right, bottom, left);
        }

        bool onClick(){
            return this->clickEvent.fire(this);
        }

        brls::Event<GridListItem *>* getClickEvent(){
            return &this->clickEvent;
        }

        size_t getIndex(){
            return this->index;
        }
        void setIndex(int i){
            this->index = i;
        }

        void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override {
            // Label
            nvgFillColor(vg, a(ctx->theme->textColor));
            nvgFontSize(vg, this->textSize);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgBeginPath(vg);
            nvgText(vg, x + width / 2 , y + height / 2, this->label.c_str(), nullptr);
        }

    protected:
        brls::Event<GridListItem *> clickEvent;
        size_t index = 0;
};

class VideoListItem : public GridListItem
{
    public:
        VideoListItem(std::string name, std::string title, std::string thumbnail, std::string avatar)
            :GridListItem(name){
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
                this->avatarView = new NetImage(imagePath, BOREALIS_ASSET("icon/akari.jpg"));

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
        }

    private:
        NetImage* avatarView      = nullptr;
        NetImage* thumbnailView   = nullptr;
        brls::Label* titleView    = nullptr;

};

class MoreListItem : public GridListItem
{
    public:
        MoreListItem():GridListItem("..."){

        }
};

template <class itemType>
class GridList : public brls::List
{
  public:

    GridList(int columns,int itemWidth, int itemHeight, int spacing, std::function<GridListItem*(itemType)> createItem){
        this->columns = columns;
        this->itemWidth = itemWidth;
        this->itemHeight = itemHeight;
        this->spacing = spacing;
        this->createItem = createItem;
        
        this->setSpacing(spacing);
        this->setMargins(0,0,0,0);

    }

    void addListData(std::vector<itemType> list){
        RunOnUIThread([this, list](){
            for(auto item: list){
                GridListItem* tempView = this->createItem(item);
                this->addItem(tempView);
            }
            if(this->moreAction){
                this->addItem(this->moreListItem);
            }
            this->invalidate(true);
        });
        this->listData.insert(this->listData.end(), list.begin(), list.end());
        this->removeMoreAction(false);
    }

    void addItem(GridListItem * view){
        if(!view) return;
        if (this->itemNums % this->columns == 0 ){
            HorizontalListContentView *h = new HorizontalListContentView(this, 0);
            h->setSpacing(this->spacing);
            h->setMargins(0,0,0,0);
            h->setHeight(this->itemHeight);
            listRows.push_back(h);
            this->addView(h);
        }
        HorizontalListContentView *row = listRows[listRows.size()-1];
        view->getClickEvent()->subscribe([this](GridListItem* view){
            if (view->getIndex() < this->listData.size()) {
                this->clickEvent.fire(this->listData[view->getIndex()],view);
            }
        });
        view->setIndex(this->itemNums);
        row->addView(view);
        if(!brls::Application::getCurrentFocus()) brls::Application::giveFocus(view);
        this->itemNums++;
    }

    void addMoreAction(std::function<void()> callback){
        this->moreAction = true;
        this->moreListItem = new MoreListItem();
        moreListItem->getClickEvent()->subscribe([callback](GridListItem* view){
            callback();
        });
        this->moreListItem->setWidth(this->itemWidth);
        this->moreListItem->setHeight(this->itemHeight);
    }

    void removeMoreAction(bool free = true){
        brls::Logger::error("remove More Action:{} {}",this->moreAction, this->itemNums);
        if (this->moreAction && this->itemNums > 0){
            HorizontalListContentView *row = listRows[listRows.size()-1];
            row->removeView(row->getViewsCount() - 1, false);
            this->itemNums--;
            brls::Logger::error("last line row count: {} {} {}",row->getViewsCount(),this->getViewsCount(), listRows.size());
            if (row->getViewsCount() == 0){
                this->removeView(this->getViewsCount()-1);
                listRows.pop_back();
            }
        }
        if (!free) return;

        this->moreAction = false;
        if (this->moreListItem){
            delete this->moreListItem;
            this->moreListItem = nullptr;
        }
    }

    brls::Event<itemType, GridListItem*>* getClickEvent(){
        return &this->clickEvent;
    }

    private:
        int columns;
        int itemHeight;
        int itemWidth;
        int itemNums = 0;
        int spacing;
        bool moreAction = false;
        std::vector<HorizontalListContentView*> listRows;
        // std::queue<itemType> itemsNeededAdd;
        std::vector<itemType> listData;
        std::function<GridListItem*(itemType)> createItem;
        brls::Event<itemType, GridListItem*> clickEvent;
        MoreListItem* moreListItem = nullptr;
};