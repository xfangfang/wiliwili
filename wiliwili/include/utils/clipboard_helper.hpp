#pragma once

#include <string>
#ifdef _WIN32
#include <Windows.h>
#include <winuser.h>
#endif

namespace wiliwili{

    void toClipboard(const std::string& link) {
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

    }

}