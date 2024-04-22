//
// Created by fang on 2023/4/25.
//

#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <zlib.h>
#include <cpr/cpr.h>
#include <mongoose.h>

#include "utils/string_helper.hpp"

namespace wiliwili {

std::string urlEncode(const std::string &in) { return cpr::util::urlEncode(in); }

std::string base64Encode(const std::string &data) {
    size_t in_len  = data.size();
    size_t out_len = ((in_len / 3) + (in_len % 3 ? 1 : 0)) * 4 + 1;
    std::unique_ptr<char[]> buf(new char[out_len]);
    const auto *in = reinterpret_cast<const unsigned char*>(data.c_str());
    size_t encoded_len = mg_base64_encode(in, in_len, buf.get(), out_len);
    std::string out;
    if (encoded_len > 0) {
        out.assign(buf.get(), encoded_len);
    }
    return out;
}

int base64Decode(const std::string &input, std::string &out) {
    out.clear();
    size_t in_len = input.size();
    if (in_len % 4 != 0) { // "Input data size is not a multiple of 4"
        return -1;
    }
    size_t out_len = in_len / 4 * 3 + 1;
    std::unique_ptr<char[]> buf(new char[out_len]);
    size_t decoded_len = mg_base64_decode(input.c_str(), input.size(), buf.get(), out_len);
    if (decoded_len > 0) {
        out.assign(buf.get(), decoded_len);
        return 0;
    }
    return -1;
}

std::string decompressGzipData(const std::string &compressedData) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (inflateInit2(&zs, 16 + MAX_WBITS) != Z_OK) {
        throw(std::runtime_error("inflateInit failed while decompressing."));
    }

    zs.next_in  = (Bytef *)compressedData.data();
    zs.avail_in = compressedData.size();

    int ret;
    char outbuffer[4096];
    std::string decompressedData;

    do {
        zs.next_out  = reinterpret_cast<Bytef *>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = inflate(&zs, 0);

        if (decompressedData.size() < zs.total_out) {
            decompressedData.append(outbuffer, zs.total_out - decompressedData.size());
        }

    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        throw(std::runtime_error(fmt::format("Exception during zlib decompression: ({}) {}", ret, zs.msg)));
    }

    return decompressedData;
}

std::string toUpper( const std::string & str, std::string::size_type length )
{
    std::string::size_type len = std::min(str.size(), length), i;
    std::string s = str.substr(0, len);

    for ( i = 0; i < len; ++i )
    {
        if ( ::islower( s[i] ) ) s[i] = (char) ::toupper( s[i] );
    }

    return s;
}

};  // namespace wiliwili
