#include "utils/string_helper.hpp"


namespace wiliwili {

    std::string urlEncode(const std::string &in) {
        const char *input = in.c_str();

        std::string working;

        while (*input) {
            const unsigned char c = *input++;
            if (isNonSymbol(c)) {
                working += (char)c;
            } else {
                working += fmt::format("%{:02x}", c);
            }
        }

        return working;
    }
}