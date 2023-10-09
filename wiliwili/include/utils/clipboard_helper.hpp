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



    void toClipboard(std::string link, const shareTarget target, const bilibili::VideoDetailResult &videoDetail) {
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
                return;
                // pics=http%3A%2F%2Fi0.hdslb.com%2Fbfs%2Farchive%2Fc68481c15a2f5eb5013748ef030aba9059fd4935.jpg&
                break;
            }

            case qzone:
            {
                share_address = "https://sns.qzone.qq.com/cgi-bin/qzshare/cgi_qzshare_onekey?url=" + link + "&hare_medium%3Dweb%26share_source%3Dqqzone%26bbid%3D9E61A6CC-7270-5EA4-C990-AC214C017D1B11023infoc%26ts%3D1696562051558&showcount=1&desc="+ videoDetail.desc.substr(0,20) + " UP主：" + videoDetail.owner.name + "&summary=" + videoDetail.desc.substr(0,20)  + "&title="+videoDetail.title+ "&site=%E5%93%94%E5%93%A9%E5%93%94%E5%93%A9&style=203&width=98&height=22&pics="+videoDetail.pic;
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

void openBrowser(const std::string url){
    #ifdef _WIN32
        ShellExecute(0, 0, url.c_str(), 0, 0 , SW_SHOW );
    #endif
}
