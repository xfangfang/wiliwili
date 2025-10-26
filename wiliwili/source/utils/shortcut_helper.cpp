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
    {"f13", brls::BRLS_KBD_KEY_F13},
    {"f14", brls::BRLS_KBD_KEY_F14},
    {"f15", brls::BRLS_KBD_KEY_F15},
    {"f16", brls::BRLS_KBD_KEY_F16},
    {"f17", brls::BRLS_KBD_KEY_F17},
    {"f18", brls::BRLS_KBD_KEY_F18},
    {"f19", brls::BRLS_KBD_KEY_F19},
    {"f20", brls::BRLS_KBD_KEY_F20},
    {"f21", brls::BRLS_KBD_KEY_F21},
    {"f22", brls::BRLS_KBD_KEY_F22},
    {"f23", brls::BRLS_KBD_KEY_F23},
    {"f24", brls::BRLS_KBD_KEY_F24},
    {"tab", brls::BRLS_KBD_KEY_TAB},
    {"backspace", brls::BRLS_KBD_KEY_BACKSPACE},
    {"insert", brls::BRLS_KBD_KEY_INSERT},
    {"delete", brls::BRLS_KBD_KEY_DELETE},
    {"pgup", brls::BRLS_KBD_KEY_PAGE_UP},
    {"pgdn", brls::BRLS_KBD_KEY_PAGE_DOWN},
    {"home", brls::BRLS_KBD_KEY_HOME},
    {"end", brls::BRLS_KBD_KEY_END},
    {"up", brls::BRLS_KBD_KEY_UP},
    {"down", brls::BRLS_KBD_KEY_DOWN},
    {"left", brls::BRLS_KBD_KEY_LEFT},
    {"right", brls::BRLS_KBD_KEY_RIGHT},
    {"pause", brls::BRLS_KBD_KEY_PAUSE},
    {"menu", brls::BRLS_KBD_KEY_MENU},
    {"space", brls::BRLS_KBD_KEY_SPACE},
    /* support literal names of some printable keys */
    {"apostrophe", brls::BRLS_KBD_KEY_APOSTROPHE}, /* ' */
    {"comma", brls::BRLS_KBD_KEY_COMMA}, /* , */
    {"minus", brls::BRLS_KBD_KEY_MINUS}, /* - */
    {"period", brls::BRLS_KBD_KEY_PERIOD}, /* . */
    {"slash", brls::BRLS_KBD_KEY_SLASH}, /* / */
    {"backslash", brls::BRLS_KBD_KEY_BACKSLASH}, /* \ */
    {"semicolon", brls::BRLS_KBD_KEY_SEMICOLON}, /* ; */
    {"equal", brls::BRLS_KBD_KEY_EQUAL}, /* = */
    {"grave", brls::BRLS_KBD_KEY_GRAVE_ACCENT}, /* ` */
    {"left_bracket", brls::BRLS_KBD_KEY_LEFT_BRACKET}, /* [ */
    {"right_bracket", brls::BRLS_KBD_KEY_RIGHT_BRACKET}, /* ] */
    {"'", brls::BRLS_KBD_KEY_APOSTROPHE},
    {",", brls::BRLS_KBD_KEY_COMMA},
    {".", brls::BRLS_KBD_KEY_PERIOD},
    {"/", brls::BRLS_KBD_KEY_SLASH},
    {"\\", brls::BRLS_KBD_KEY_BACKSLASH},
    {";", brls::BRLS_KBD_KEY_SEMICOLON},
    {"=", brls::BRLS_KBD_KEY_EQUAL},
    {"`", brls::BRLS_KBD_KEY_GRAVE_ACCENT},
    {"[", brls::BRLS_KBD_KEY_LEFT_BRACKET},
    {"]", brls::BRLS_KBD_KEY_RIGHT_BRACKET},
};

brls::BrlsKeyCombination ShortcutHelper::parseKey(const std::string& config) {
    std::vector<std::string> keys;
    if (pystring::endswith(config, "-")) {
        const std::string cfg = config.substr(0, config.size() - 1) + "minus";
        keys = pystring::split(cfg, "-");
    } else {
        keys = pystring::split(config, "-");
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
