#pragma once

#include <string>
#ifdef _WIN32
#include <Windows.h>
#include <winuser.h>
#include <shellapi.h>
#endif

void openBrowser(const std::string);


namespace brls{

    enum shareTarget{
        clipboard,
        dynamic,
        qq,
        qzone,
        tieba,
        wechat,
        weibo,
    };



    void toClipboard(std::string link, const shareTarget target) {
        std::string share_address = link;
        switch(target){
            
            case clipboard:
            {
                #ifdef _WIN32
                auto hwnd = GetDesktopWindow();
                auto len = link.c_str();
                auto size = link.size();
                OpenClipboard(hwnd);
                EmptyClipboard();
                HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, size+1);
                if (!hg) {
                    CloseClipboard();
                    return;
                }
                memcpy(GlobalLock(hg), len, size+1);
                GlobalUnlock(hg);
                SetClipboardData(CF_TEXT, hg);
                CloseClipboard();
                GlobalFree(hg);
                #endif
                return;
            }

            case dynamic:
            {
                return;
                break;
            }

            case qq:
            {
                share_address = "https://sns.qzone.qq.com/cgi-bin/qzshare/cgi_qzshare_onekey?url=https%3A%2F%2Fwww.bilibili.com%2Fvideo%2FBV1Dm4y1N7cx%2F%3Fvd_source%3Ddf644fa6a7b916844cbbce9dbe95bce4%26share_medium%3Dweb%26share_source%3Dqqzone%26bbid%3D9E61A6CC-7270-5EA4-C990-AC214C017D1B11023infoc%26ts%3D1696562051558&showcount=1&desc=%E3%80%8A%E9%AD%94%E5%85%BD%E4%BA%89%E9%9C%B83%E3%80%8B%EF%BC%8C%E4%B8%83%E9%A1%B9%E5%85%A8%E8%83%BD%E5%A4%A7%E6%AF%94%E6%8B%BC%EF%BC%8C%E4%BB%96%E4%BB%AC%E5%BD%93%E4%B8%AD%E8%B0%81%E6%9C%80%E5%BC%BA%EF%BC%9F%20UP%E4%B8%BB%EF%BC%9A%E5%85%AC%E5%AD%99%E9%9B%85%E9%87%8F&summary=%E9%AD%94%E5%85%BD%E4%BA%89%E9%9C%B83%E7%94%B5%E5%AD%90%E6%96%97%E8%9B%90%E8%9B%90&title=%E3%80%8A%E9%AD%94%E5%85%BD%E4%BA%89%E9%9C%B83%E3%80%8B%EF%BC%8C%E4%B8%83%E9%A1%B9%E5%85%A8%E8%83%BD%E5%A4%A7%E6%AF%94%E6%8B%BC%EF%BC%8C%E4%BB%96%E4%BB%AC%E5%BD%93%E4%B8%AD%E8%B0%81%E6%9C%80%E5%BC%BA%EF%BC%9F&site=%E5%93%94%E5%93%A9%E5%93%94%E5%93%A9&pics=http%3A%2F%2Fi0.hdslb.com%2Fbfs%2Farchive%2Fc68481c15a2f5eb5013748ef030aba9059fd4935.jpg&style=203&width=98&height=22";
                break;
            }

            case qzone:
            {
                return;
                break;
            }

            case tieba:
            {
                return;
                break;
            }

            case wechat:
            {
                return;
                break;
            }

            case weibo:
            {
                return;
                break;
            }
        }
        openBrowser(share_address);
        
    }
    

}

void openBrowser(const std::string link){
    #ifdef _WIN32
        ShellExecute(0, 0, link.c_str(), 0, 0 , SW_SHOW );
    #endif
}
