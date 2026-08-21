#import <AppKit/AppKit.h>

#include "utils/macos_helper.hpp"

namespace MacOSHelper {

void disableWindowShadow() {
    if (@available(macOS 26.0, *)) {
        for (NSWindow* window in NSApplication.sharedApplication.windows) {
            window.hasShadow = NO;
        }
    }
}

}
