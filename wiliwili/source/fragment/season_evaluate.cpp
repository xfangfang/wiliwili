//
// Created by fang on 2023/1/16.
//

#include <borealis/core/application.hpp>
#include <borealis/views/button.hpp>

#include "analytics.h"
#include "fragment/season_evaluate.hpp"
#include "utils/string_helper.hpp"
#include "view/button_close.hpp"
#include "view/mpv_core.hpp"

SeasonEvaluate::SeasonEvaluate() {
    this->inflateFromXMLRes("xml/fragment/season_evaluate.xml");

    btnDouban->registerClickAction([this](...) {
        GA("open_douban")
        MPVCore::instance().pause();
        brls::Application::getPlatform()->openBrowser("https://search.douban.com/movie/subject_search?search_text=" +
                                                      wiliwili::urlEncode(this->keyword));
        return true;
    });

    btnZhihu->registerClickAction([this](...) {
        GA("open_zhihu")
        MPVCore::instance().pause();
        brls::Application::getPlatform()->openBrowser("https://www.zhihu.com/search?type=content&q=" +
                                                      wiliwili::urlEncode(this->keyword));
        return true;
    });

    btnBaidu->registerClickAction([this](...) {
        GA("open_baidu")
        MPVCore::instance().pause();
        brls::Application::getPlatform()->openBrowser("https://www.baidu.com/s?wd=" +
                                                      wiliwili::urlEncode(this->keyword));
        return true;
    });

    btnBing->registerClickAction([this](...) {
        GA("open_bing")
        MPVCore::instance().pause();
        brls::Application::getPlatform()->openBrowser("https://cn.bing.com/search?q=" +
                                                      wiliwili::urlEncode(this->keyword));

        return true;
    });

#ifdef __PSV__
    btnDouban->getParent()->setVisibility(brls::Visibility::GONE);
    btnZhihu->setFocusable(false);
    btnBaidu->setFocusable(false);
    btnBing->setFocusable(false);
    btnDouban->setFocusable(false);
    btnClose->setFocusable(true);
#endif
}

void SeasonEvaluate::setKeyword(const std::string& value) { this->keyword = value; }

void SeasonEvaluate::setContent(const std::string& value) { this->label->setText(value); }

SeasonEvaluate::~SeasonEvaluate() { brls::Logger::debug("Fragment SeasonEvaluate: delete"); }

brls::View* SeasonEvaluate::create() { return new SeasonEvaluate(); }