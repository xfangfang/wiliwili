#pragma once

#include <string>
#ifdef _WIN32
#include <Windows.h>
#include <winuser.h>
#include <shellapi.h>
#endif

void openBrowser(const std::string&);


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



    void toClipboard(const std::string& link, const shareTarget target) {
        auto share_address = &link;
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
                ;
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
        openBrowser(*share_address);
        
    }
    

}

void openBrowser(const std::string& link){
    #ifdef _WIN32
        ShellExecute(0, 0, link.c_str(), 0, 0 , SW_SHOW );
    #endif
}
