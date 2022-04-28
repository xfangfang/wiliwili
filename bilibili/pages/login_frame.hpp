#pragma once

#include <borealis.hpp>
#include <bilibili.h>
#include <thread>
#include <chrono>
#include "views/qr_image.hpp"

class LoginManager {
    public:
        LoginManager(std::function<void(std::string)> urlCallback,
                    std::function<void(std::string)> hintCallback,
                    std::function<void()> successCallback,
                    std::function<void()> errorCallback)
            :onUrlChangeCallback(urlCallback),onHintChangeCallback(hintCallback),
            onSuccessCallback(successCallback),onErrorCallback(errorCallback){
                this->resume();
        }

        void getLoginUrl(){
            this->running = true;
            bilibili::BilibiliClient::get_login_url([this](std::string url, std::string oauthKey){
                brls::Logger::debug("url:{} oauth:{}",url,oauthKey);
                this->login_url = url;
                this->oauthKey = oauthKey;
                this->onUrlChangeCallback(url);
                this->checkLogin();
            });
        }

        void checkLogin(){
            brls::Logger::debug("check login");
            bilibili::BilibiliClient::get_login_info(this->oauthKey, [this](bilibili::LoginInfo info){
                brls::Logger::debug("return code:{}",info);
                switch(info){
                    case bilibili::LoginInfo::OAUTH_KEY_TIMEOUT:
                    case bilibili::LoginInfo::OAUTH_KEY_ERROR:
                            this->running = false;
                            this->onErrorCallback();
                            this->onHintChangeCallback("need refresh qrcode");
                    break;
                    case bilibili::LoginInfo::NEED_CONFIRM:
                        this->onHintChangeCallback("wait for confirm");
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        this->checkLogin();
                    break;
                    case bilibili::LoginInfo::NEED_SCAN:
                        this->onHintChangeCallback("wait for scan");
                        std::this_thread::sleep_for(std::chrono::seconds(3));
                        this->checkLogin();
                    break;
                    case bilibili::LoginInfo::SUCCESS:
                        this->onHintChangeCallback("success");
                        this->onSuccessCallback();
                        this->running = false;
                    break;
                    default:
                        brls::Logger::error("return unknown code:{}",info);
                        this->running = false;
                    break;
                }
            });
        }


        void resume(){
            if(!this->running) this->getLoginUrl();
        }
    
    private:
        std::string login_url;
        std::string oauthKey;
        std::function<void(std::string)> onUrlChangeCallback;
        std::function<void(std::string)> onHintChangeCallback;
        std::function<void()> onSuccessCallback;
        std::function<void()> onErrorCallback;
        bool stop = true;
        bool running = false;
};

class LoginFrame : public brls::AbsoluteLayout
{
    public:
        LoginFrame(std::function<void()> onLoginCallback){
            // Create views
            this->backgroudImage = new brls::Image(BOREALIS_ASSET("pictures/2233login.png"));
            this->addView(this->backgroudImage);
            this->backgroudTopImage = new brls::Image(BOREALIS_ASSET("pictures/rl_top.png"));
            this->addView(this->backgroudTopImage);

            this->qrTipImage = new brls::Image(BOREALIS_ASSET("pictures/qr_tip.png"));
            this->addView(this->qrTipImage);

            this->qrImage = new QrImage();
            this->addView(this->qrImage);

            this->loginManager = new LoginManager([this](std::string url){
                this->qrImage->setValid(true);
                this->qrImage->setImage(url);
            }, [this](std::string hint){
                this->hintText = hint;
            }, onLoginCallback ,[this](){
                this->qrImage->setValid(false);
            });

            
            this->qrImage->registerAction("Refresh", brls::Key::A,[this]{
                brls::Logger::debug("refresh");
                this->loginManager->resume();
                return true;
            }, false);

        }
        brls::View* getDefaultFocus() override {
            return this->qrImage;
        }
        brls::View* getNextFocus(brls::FocusDirection direction, brls::View* currentView) override{
            return this->navigationMap.getNextFocus(direction, currentView);
        }
        void layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash){
            int x = this->getX();
            int y = this->getY();
            int w = this->getWidth();
            int h = this->getHeight();

            this->qrImage->setBoundaries(x + w * 0.75 - 100, y + h/2 - 100, 200, 200);

            this->backgroudImage->setBoundaries(x + w * 0.75 - 240, y + h/2 + 100 ,480,155);

            this->backgroudTopImage->setBoundaries(x, y ,w,106);

            this->qrTipImage->setBoundaries(x + w * 0.25 - 240, y + h/2 - 96, 480, 192);

        }

        void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override {
            
            //background
            nvgBeginPath(vg);
            nvgRect(vg,x,y,width,86);
            nvgFillColor(vg, nvgRGB(0,160,216));
            nvgFill(vg);

            nvgBeginPath(vg);
            nvgRect(vg, x, y + 86, width, height - 86);
            nvgFillColor(vg, nvgRGB(255,255,255));
            nvgFill(vg);

            // lines
            int halfInterWidth = 60;
            int lineY = 130;
            NVGcolor lineColor = nvgRGB(221,221,221);
            nvgBeginPath(vg);
            nvgRect(vg, x + width * 1 / 8, y + lineY, width * 3 / 8 - halfInterWidth, 1);
            nvgFillColor(vg, lineColor);
            nvgFill(vg);

            nvgBeginPath(vg);
            nvgRect(vg, x + width / 2 + halfInterWidth, y + lineY, width * 3 / 8 - halfInterWidth, 1);
            nvgFillColor(vg, lineColor);
            nvgFill(vg);

            nvgBeginPath(vg);
            nvgRect(vg, x + width / 2 , y + halfInterWidth + lineY, 1, height - lineY - 2 * halfInterWidth);
            nvgFillColor(vg, lineColor);
            nvgFill(vg);


            brls::AbsoluteLayout::draw(vg,x,y,width,height,style,ctx);

            //title text
            nvgFillColor(vg, a(ctx->theme->textColor));
            nvgFontSize(vg, 32);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgBeginPath(vg);
            nvgText(vg, x + width / 2, y + lineY , "登录", nullptr);

            //hint text left
            nvgFillColor(vg, a(ctx->theme->textColor));
            nvgFontSize(vg, 18);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgBeginPath(vg);
            nvgText(vg, x + width / 4, y + height / 2 + 96 + 50 , this->hintText.c_str(), nullptr);

            //hint text right
            nvgFillColor(vg, nvgRGB(113,113,113));
            nvgFontSize(vg, 14);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgBeginPath(vg);
            nvgTextBox(vg, x + width * 0.75 - 80 , y + height / 2 + 96 + 50 ,160 , this->hintText2.c_str(), nullptr);
        }

        

        ~LoginFrame(){
            delete this->loginManager;
        }

    private:
        brls::NavigationMap navigationMap;
        QrImage* qrImage;
        brls::Image* backgroudImage;
        brls::Image* backgroudTopImage;
        brls::Image* qrTipImage;
        LoginManager* loginManager;
        std::string hintText;
        std::string hintText2 = "请使用哔哩哔哩客户端\n扫码登录\n或扫码下载APP";

};