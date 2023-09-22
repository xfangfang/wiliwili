#pragma once

#include <string>
#ifdef _WIN32
#include <Windows.h>
#include <winuser.h>
#endif




namespace wiliwili{

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
                break;
            }

            case dynamic:
            {
                return;
                break;
            }

            case qq:
            {
                return;
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
    }
        
        

}