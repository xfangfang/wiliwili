#pragma once

#include <borealis.hpp>

class HorizontalListContentView : public brls::BoxLayout
{
  public:
    HorizontalListContentView(brls::List* list, size_t defaultFocus)
        : BoxLayout(brls::BoxLayoutOrientation::HORIZONTAL, defaultFocus)
        , list(list){
        brls::Style* style = brls::Application::getStyle();
        this->setMargins(style->List.marginTopBottom, style->List.marginLeftRight, style->List.marginTopBottom, style->List.marginLeftRight);
        this->setSpacing(style->List.spacing);
        this->setRememberFocus(true);
    }

  protected:
    void customSpacing(brls::View* current, brls::View* next, int* spacing){}

  private:
    brls::List* list;
};

class GridList : public brls::List
{
  private:
    int nums;
    int height;

  public:
    brls::ListItem * create(std::string title){
        brls::ListItem *l1 = new brls::ListItem(title);
        brls::Image * image = new brls::Image(BOREALIS_ASSET("icon/bilibili_128x128.jpg"));
        l1->setThumbnail(image);
        l1->setWidth(280);
        l1->setHeight(200);
        return l1;
    }
    HorizontalListContentView * createRow(){
        HorizontalListContentView *h = new HorizontalListContentView(this, 0);
        h->setSpacing(0);
        h->setMargins(0,0,0,0);
        h->setHeight(200);
        h->addView(this->create("p1"));
        h->addView(this->create("p2"));
        h->addView(this->create("p3"));
        h->addView(this->create("p4"));
        return h;
    }
    GridList(int nums,int height):nums(nums),height(height){
        this->setSpacing(0);
        this->setMargins(0,0,0,0);
        
        this->addView(createRow());
        this->addView(createRow());
        this->addView(createRow());

    }
};