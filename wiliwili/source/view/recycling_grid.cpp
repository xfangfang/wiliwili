//
// Created by fang on 2022/6/15.
//

#include "view/recycling_grid.hpp"

RecyclingGridItem::RecyclingGridItem() {
    this->setFocusable(true);
    this->registerClickAction([this](View* view) {
        RecyclingGrid* recycler = dynamic_cast<RecyclingGrid*>(getParent()->getParent());
        if (recycler)
            recycler->getDataSource()->onItemSelected(recycler, index);
        return true;
    });
    this->addGestureRecognizer(new brls::TapGestureRecognizer(this));
}

size_t RecyclingGridItem::getIndex() const {
    return this->index;
}

void RecyclingGridItem::setIndex(size_t value){
    this->index = value;
}

RecyclingGridItem::~RecyclingGridItem() {
}


RecyclingGrid::RecyclingGrid() {
    brls::Logger::debug("View RecyclingGrid: create");

    this->setFocusable(false);

    this->setScrollingBehavior(brls::ScrollingBehavior::CENTERED);
    // Create content box
    this->contentBox = new RecyclingGridContentBox(this);
    this->setContentView(this->contentBox);

    this->registerFloatXMLAttribute("itemHeight", [this](float value){
        this->estimatedRowHeight = value + this->estimatedRowSpace;
        this->reloadData();
    });

    this->registerFloatXMLAttribute("spanCount", [this](float value){
        this->spanCount = value;
        this->reloadData();
    });

    this->registerFloatXMLAttribute("itemSpace", [this](float value){
        this->estimatedRowSpace = value;
        this->reloadData();
    });

    this->registerFloatXMLAttribute("preFetchLine", [this](float value){
        this->preFetchLine = value;
        this->reloadData();
    });

}

RecyclingGrid::~RecyclingGrid() {
    brls::Logger::debug("View RecyclingGridActivity: delete");
    if(this->dataSource)
        delete this->dataSource;
    for (auto it : queueMap)
    {
        for (auto item : *it.second)
            delete item;
        delete it.second;
    }
}

void RecyclingGrid::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style, brls::FrameContext* ctx)
{
    // 触摸或鼠标滑动时会导致屏幕元素位置变更
    // 简单地在draw函数中调用itemsRecyclingLoop 实现动态的增删元素
    // todo：只在滑动过程中调用 itemsRecyclingLoop 以节省静止时的计算消耗
    itemsRecyclingLoop();

    ScrollingFrame::draw(vg, x, y, width, height, style, ctx);
}

void RecyclingGrid::registerCell(std::string identifier, std::function<RecyclingGridItem*()> allocation)
{
    queueMap.insert(std::make_pair(identifier, new std::vector<RecyclingGridItem*>()));
    allocationMap.insert(std::make_pair(identifier, allocation));
}

void RecyclingGrid::addCellAt(size_t index, int downSide)
{
    RecyclingGridItem* cell;
    //获取到一个填充好数据的cell
    cell = dataSource->cellForRow(this, index);
    cell->setHeight(estimatedRowHeight - estimatedRowSpace);
    cell->setWidth((renderedFrame.getWidth() - paddingLeft - paddingRight) / spanCount - estimatedRowSpace);
    cell->setDetachedPositionX(renderedFrame.getMinX() + paddingLeft
                               + (renderedFrame.getWidth() - paddingLeft - paddingRight) / spanCount * (index % spanCount));
    cell->setDetachedPositionY(estimatedRowHeight * (int)(index / spanCount) + paddingTop);
    cell->setIndex(index);

    this->contentBox->getChildren().insert(this->contentBox->getChildren().end(), cell);

    // Allocate and set parent userdata
    size_t* userdata = (size_t*)malloc(sizeof(size_t));
    *userdata        = index;

    cell->setParent(this->contentBox, userdata);

    // Layout and events
    this->contentBox->invalidate();
    cell->View::willAppear();

    if (index < visibleMin)
        visibleMin = index;

    if (index > visibleMax)
        visibleMax = index;

    if (!downSide)
        renderedFrame.origin.y -= index % spanCount == 0 ? estimatedRowHeight: 0;

    renderedFrame.size.height += index % spanCount == 0 ? estimatedRowHeight: 0;

    brls::Logger::debug("Cell #" + std::to_string(index) + " - added");
}

void RecyclingGrid::setDataSource(RecyclingGridDataSource* source)
{
    if (this->dataSource)
        delete this->dataSource;

    this->dataSource = source;
    if (layouted)
        reloadData();
}

void RecyclingGrid::reloadData()
{
    if (!layouted)
        return;

    // 将所有节点从屏幕上移除放入重复利用的列表中
    auto children = this->contentBox->getChildren();
    for (auto const& child : children)
    {
        queueReusableCell((RecyclingGridItem*)child);
        this->contentBox->removeView(child, false);
    }

    visibleMin = UINT_MAX;
    visibleMax = 0;

    renderedFrame            = brls::Rect();
    renderedFrame.size.width = getWidth();

    setContentOffsetY(0, false);

    if (dataSource)
    {
        contentBox->setHeight(estimatedRowHeight * this->getRowCount() + paddingTop + paddingBottom);
        brls::Rect frame  = getLocalFrame();
        for (auto row = 0; row < dataSource->getItemCount(); row++)
        {
            this->addCellAt(row, true);
            if (renderedFrame.getMaxY() - preFetchLine * estimatedRowHeight > frame.getMaxY() && (row + 1) % spanCount == 0)
                break;
        }
        selectRowAt(this->defaultCellFocus, false);
    }
}

void RecyclingGrid::notifyDataChanged() {
    // todo: 目前仅能处理data在原本的基础上增加的情况，需要考虑data减少或更换时的情况
    if (!layouted)
        return;

    if (dataSource)
    {
        contentBox->setHeight(estimatedRowHeight * this->getRowCount() + paddingTop + paddingBottom);
    }
}

void RecyclingGrid::clearData() {
    if (!layouted)
        return;

    if (dataSource)
    {
        dataSource->clearData();
        this->reloadData();
    }
}

void RecyclingGrid::setDefaultCellFocus(size_t index){
    this->defaultCellFocus = index;
}

size_t RecyclingGrid::getDefaultCellFocus() const{
    return this->defaultCellFocus;
}

size_t RecyclingGrid::getItemCount(){
    return this->dataSource->getItemCount();
}

size_t RecyclingGrid::getRowCount(){
    return (this->dataSource->getItemCount() - 1) / this->spanCount + 1;
}

void RecyclingGrid::onNextPage(const std::function<void()>& callback){
    this->nextPageCallback = callback;
}

void RecyclingGrid::itemsRecyclingLoop()
{
    if(!dataSource)
        return;

    brls::Rect visibleFrame = getVisibleFrame();

    // 上方元素自动销毁
    while (true)
    {
        RecyclingGridItem* minCell = nullptr;
        for (auto it : contentBox->getChildren())
            if (*((size_t*)it->getParentUserData()) == visibleMin)
                minCell = (RecyclingGridItem*)it;

        if (!minCell || (minCell->getDetachedPosition().y + (1 + preFetchLine) * estimatedRowHeight >= visibleFrame.getMinY()))
            break;

        renderedFrame.origin.y += minCell->getIndex() % spanCount == 0 ? estimatedRowHeight: 0;
        renderedFrame.size.height -= minCell->getIndex() % spanCount == 0 ? estimatedRowHeight: 0;

        queueReusableCell(minCell);
        this->contentBox->removeView(minCell, false);

        brls::Logger::debug("Cell #" + std::to_string(visibleMin) + " - destroyed");

        visibleMin++;
    }

    // 下方元素自动销毁
    while (true)
    {
        RecyclingGridItem* maxCell = nullptr;
        for (auto it : contentBox->getChildren())
            if (*((size_t*)it->getParentUserData()) == visibleMax)
                maxCell = (RecyclingGridItem*)it;

        if (!maxCell || (maxCell->getDetachedPosition().y - (1 + preFetchLine) * estimatedRowHeight <= visibleFrame.getMaxY()))
            break;

        renderedFrame.size.height -= maxCell->getIndex() % spanCount == 0 ? estimatedRowHeight: 0;

        queueReusableCell(maxCell);
        this->contentBox->removeView(maxCell, false);

        brls::Logger::debug("Cell #" + std::to_string(visibleMax) + " - destroyed");

        visibleMax--;
    }

    // 上方元素自动添加
    while (visibleMin - 1 < dataSource->getItemCount() )
    {
        if(renderedFrame.getMinY() + preFetchLine * estimatedRowHeight < visibleFrame.getMinY() - paddingTop){
            if( (visibleMin) % spanCount == 0)
                break;
        }
        int i = visibleMin - 1;
        addCellAt(i, false);
    }

    // 下方元素自动添加
    while (visibleMax + 1 < dataSource->getItemCount())
    {
        if(renderedFrame.getMaxY() - preFetchLine * estimatedRowHeight > visibleFrame.getMaxY() - paddingBottom){
            if( (visibleMax + 1) % spanCount == 0)
                break;
        }
        int i = visibleMax + 1;
        addCellAt(i, true);
    }
}

RecyclingGridDataSource* RecyclingGrid::getDataSource() const
{
    return this->dataSource;
}

void RecyclingGrid::selectRowAt(size_t index, bool animated)
{
    this->setContentOffsetY(this->estimatedRowHeight * index, animated);
    this->itemsRecyclingLoop();

    for (View* view : contentBox->getChildren())
    {
        if (*((size_t*)view->getParentUserData()) == index)
        {
            contentBox->setLastFocusedView(view);
            break;
        }
    }
}

brls::View* RecyclingGrid::getNextCellFocus(brls::FocusDirection direction, brls::View* currentView)
{
    void* parentUserData = currentView->getParentUserData();

    // Allow up and down when axis is ROW
    if ((this->contentBox->getAxis() == brls::Axis::ROW && direction != brls::FocusDirection::LEFT && direction != brls::FocusDirection::RIGHT)) {
        int row_offset = spanCount;
        if (direction == brls::FocusDirection::UP) row_offset = -spanCount;
        View* row_currentFocus       = nullptr;
        size_t row_currentFocusIndex = *((size_t*)parentUserData) + row_offset;

        if (row_currentFocusIndex >= this->dataSource->getItemCount()) {
            row_currentFocusIndex -= *((size_t*)parentUserData) % spanCount;
        }

        while (!row_currentFocus && row_currentFocusIndex >= 0 && row_currentFocusIndex <
                                                                          this->dataSource->getItemCount())
        {
            for (auto it : this->contentBox->getChildren())
            {
                if (*((size_t*)it->getParentUserData()) == row_currentFocusIndex)
                {
                    row_currentFocus = it->getDefaultFocus();
                    break;
                }
            }
            row_currentFocusIndex += row_offset;
        }
        if (row_currentFocus) {
            // 按键(上或下)可以导航过去的情况
            itemsRecyclingLoop();

            // 到达页面尾部
            if ( direction == brls::FocusDirection::DOWN && visibleMax + 1 >= this->getItemCount()){
                if(this->nextPageCallback){
                    this->nextPageCallback();
                }
            }
            return row_currentFocus;
        }
    }

    if (this->contentBox->getAxis() == brls::Axis::ROW) {
        int position = *((size_t*)parentUserData) % spanCount;
        if((direction == brls::FocusDirection::LEFT && position == 0) || (direction == brls::FocusDirection::RIGHT && position == (spanCount-1))) {
            View* next = getParentNavigationDecision(this, nullptr, direction);
            if (!next && hasParent())
                next = getParent()->getNextFocus(direction, this);
            return next;
        }
    }


    // Return nullptr immediately if focus direction mismatches the box axis (clang-format refuses to split it in multiple lines...)
    if ((this->contentBox->getAxis() == brls::Axis::ROW && direction != brls::FocusDirection::LEFT && direction != brls::FocusDirection::RIGHT) || (this->contentBox->getAxis() == brls::Axis::COLUMN && direction != brls::FocusDirection::UP && direction != brls::FocusDirection::DOWN))
    {
        View* next = getParentNavigationDecision(this, nullptr, direction);
        if (!next && hasParent())
            next = getParent()->getNextFocus(direction, this);
        return next;
    }

    // Traverse the children
    size_t offset = 1; // which way we are going in the children list

    if ((this->contentBox->getAxis() == brls::Axis::ROW && direction == brls::FocusDirection::LEFT) || (this->contentBox->getAxis() == brls::Axis::COLUMN && direction == brls::FocusDirection::UP))
    {
        offset = -1;
    }

    size_t currentFocusIndex = *((size_t*)parentUserData) + offset;
    View* currentFocus       = nullptr;

    while (!currentFocus && currentFocusIndex >= 0 && currentFocusIndex < this->dataSource->getItemCount())
    {
        for (auto it : this->contentBox->getChildren())
        {
            if (*((size_t*)it->getParentUserData()) == currentFocusIndex)
            {
                currentFocus = it->getDefaultFocus();
                break;
            }
        }
        currentFocusIndex += offset;
    }

    currentFocus = getParentNavigationDecision(this, currentFocus, direction);
    if (!currentFocus && hasParent())
        currentFocus = getParent()->getNextFocus(direction, this);
    return currentFocus;
}

void RecyclingGrid::onLayout()
{
    ScrollingFrame::onLayout();
    float width = this->getWidth();
    // check NAN
    if(width != width)
        return;

    this->contentBox->setWidth(width);
    if (checkWidth())
    {
        layouted = true;
        reloadData();
    }
}

bool RecyclingGrid::checkWidth(){
    float width           = getWidth();
    static float oldWidth = width;
    if ((int)oldWidth != (int)width && width != 0)
    {
        oldWidth = width;
        return true;
    }
    oldWidth = width;
    return false;
}

void RecyclingGrid::queueReusableCell(RecyclingGridItem* cell)
{
    queueMap.at(cell->reuseIdentifier)->push_back(cell);
    cell->cacheForReuse();
}

void RecyclingGrid::setPadding(float padding)
{
    this->setPadding(padding, padding, padding, padding);
}

void RecyclingGrid::setPadding(float top, float right, float bottom, float left)
{
    paddingTop    = top;
    paddingRight  = right;
    paddingBottom = bottom;
    paddingLeft   = left;

    this->reloadData();
}

void RecyclingGrid::setPaddingTop(float top)
{
    paddingTop = top;
    this->reloadData();
}

void RecyclingGrid::setPaddingRight(float right)
{
    paddingRight = right;
    this->reloadData();
}

void RecyclingGrid::setPaddingBottom(float bottom)
{
    paddingBottom = bottom;
    this->reloadData();
}

void RecyclingGrid::setPaddingLeft(float left)
{
    paddingLeft = left;
    this->reloadData();
}

brls::View* RecyclingGrid::create() {
    return new RecyclingGrid();
}

RecyclingGridItem* RecyclingGrid::dequeueReusableCell(std::string identifier)
{
    brls::Logger::debug("RecyclingGrid::dequeueReusableCell: {}", identifier);
    RecyclingGridItem* cell = nullptr;
    auto it            = queueMap.find(identifier);

    if (it != queueMap.end())
    {
        std::vector<RecyclingGridItem*>* vector = it->second;
        if (!vector->empty())
        {
            cell = vector->back();
            vector->pop_back();
        }
        else
        {
            cell                  = allocationMap.at(identifier)();
            cell->reuseIdentifier = identifier;
            cell->detach();
        }
    }

    if (cell)
        cell->prepareForReuse();

    return cell;
}

RecyclingGridContentBox::RecyclingGridContentBox(RecyclingGrid* recycler): Box(brls::Axis::ROW), recycler(recycler){}

brls::View* RecyclingGridContentBox::getNextFocus(brls::FocusDirection direction, brls::View* currentView) {
    return this->recycler->getNextCellFocus(direction, currentView);
}