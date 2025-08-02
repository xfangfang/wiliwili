//
// Created by fang on 2025/7/27.
//

#include "utils/shortcut_helper.hpp"

#include <unordered_map>
#include <pystring.h>

#include "borealis/core/logger.hpp"

static std::unordered_map<std::string, brls::BrlsKeyboardModifiers> modifierMap = {
    {"shift", brls::BRLS_KBD_MODIFIER_SHIFT},
    {"ctrl", brls::BRLS_KBD_MODIFIER_CTRL},
    {"alt", brls::BRLS_KBD_MODIFIER_ALT},
    {"meta", brls::BRLS_KBD_MODIFIER_META}
};

static std::unordered_map<std::string, brls::BrlsKeyboardScancode> functionMap = {
    {"f1", brls::BRLS_KBD_KEY_F1},
    {"f2", brls::BRLS_KBD_KEY_F2},
    {"f3", brls::BRLS_KBD_KEY_F3},
    {"f4", brls::BRLS_KBD_KEY_F4},
    {"f5", brls::BRLS_KBD_KEY_F5},
    {"f6", brls::BRLS_KBD_KEY_F6},
    {"f7", brls::BRLS_KBD_KEY_F7},
    {"f8", brls::BRLS_KBD_KEY_F8},
    {"f9", brls::BRLS_KBD_KEY_F9},
    {"f10", brls::BRLS_KBD_KEY_F10},
    {"f11", brls::BRLS_KBD_KEY_F11},
    {"f12", brls::BRLS_KBD_KEY_F12},
    {"pgup", brls::BRLS_KBD_KEY_PAGE_UP},
    {"pgdn", brls::BRLS_KBD_KEY_PAGE_DOWN},
    {"home", brls::BRLS_KBD_KEY_HOME},
    {"end", brls::BRLS_KBD_KEY_END},
    {"up", brls::BRLS_KBD_KEY_UP},
    {"down", brls::BRLS_KBD_KEY_DOWN},
    {"left", brls::BRLS_KBD_KEY_LEFT},
    {"right", brls::BRLS_KBD_KEY_RIGHT},
    {"space", brls::BRLS_KBD_KEY_SPACE},
    {"[", brls::BRLS_KBD_KEY_LEFT_BRACKET},
    {"]", brls::BRLS_KBD_KEY_RIGHT_BRACKET},
};

brls::BrlsKeyCombination ShortcutHelper::parseKey(const std::string& config) {
    auto keys = pystring::split(config, "-");
    if (keys.empty()) {
        brls::Logger::error("Invalid key configuration: {}", config);
        return {brls::BRLS_KBD_KEY_UNKNOWN};
    }
    brls::BrlsKeyCombination res = {brls::BRLS_KBD_KEY_UNKNOWN};
    for (auto& key : keys) {
        key = pystring::strip(key);
        key = pystring::lower(key);
        if (key.empty()) continue;

        if (modifierMap.count(key) > 0) {
            // If it's a modifier, we just set it in the result
            res.mod |= modifierMap[key];
            continue;
        }

        if (functionMap.count(key) > 0) {
            // If it's a function key, we just set it in the result
            res.code = functionMap[key];
            break;
        }

        if (key.length() == 1) {
            const auto keyChar = key[0];
            if (keyChar >= '0' && keyChar <= '9') {
                res.code = static_cast<brls::BrlsKeyboardScancode>(keyChar - '0' + brls::BRLS_KBD_KEY_0);
                break;
            }
            if (keyChar >= 'a' && keyChar <= 'z') {
                res.code = static_cast<brls::BrlsKeyboardScancode>(keyChar - 'a' + brls::BRLS_KBD_KEY_A);
                break;
            }

            brls::Logger::error("Invalid key configuration: {}", config);
            return {brls::BRLS_KBD_KEY_UNKNOWN};
        }

        brls::Logger::error("Invalid key configuration: {}", config);
        return {brls::BRLS_KBD_KEY_UNKNOWN};
    }

    return res;
}