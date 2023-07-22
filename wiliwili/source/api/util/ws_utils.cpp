#include "live/ws_utils.hpp"

#include <utility>
#include <iostream>
#include <cstring>
#include <memory>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <zlib.h>
//#include <brotli/decode.h>

//buffer
static uint8_t buffer[1024 * 256];
// 解析数据包
std::vector<std::string> parse_packet(const std::vector<uint8_t>& data) {
    std::vector<std::string> messages;
    messages.reserve(128);

    std::vector<uint8_t> decompressed;
    decompressed.reserve(1024 * 256);

    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    size_t data_len = data.size();
    size_t offset = 0;

    while (offset < data_len) {

        uint32_t packet_length = ntohl(*reinterpret_cast<const uint32_t*>(data.data() + offset));
        uint16_t header_length = ntohs(*reinterpret_cast<const uint16_t*>(data.data() + offset + 4));
        uint16_t protocol_version = ntohs(*reinterpret_cast<const uint16_t*>(data.data() + offset + 6));  
        uint32_t operation = ntohl(*reinterpret_cast<const uint32_t*>(data.data() + offset + 8));

        offset += header_length;

        //| 3 | 服务器 | 数据类型为Int 32 Big Endian | 心跳回应 | Body 内容为房间人气值 |
        if (operation == 3){
            uint32_t body = ntohl(*reinterpret_cast<const uint32_t*>(data.data() + offset));
            messages.emplace_back("heartbeat reply: " + std::to_string(body)); 
            break;
        }
        //| 5 | 服务器 | 数据类型为JSON纯文本 | 通知 | 弹幕、广播等全部信息 |
        else if (operation == 5) {
            std::string body(
                    reinterpret_cast<const char*>(data.data() + offset), 
                    packet_length - header_length);

            if (protocol_version == 0) {
                messages.emplace_back(std::move(body));
                break;
            }
            else if (protocol_version == 2){
                strm.avail_in = body.size();
                strm.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(body.data()));
                if (inflateInit(&strm) != Z_OK) {
                    //std::cerr << "Failed to initialize zlib" << std::endl;
                    break;
                }
                do {
                    strm.avail_out = sizeof(buffer);
                    strm.next_out = buffer;
                    if (inflate(&strm, Z_NO_FLUSH) == Z_STREAM_ERROR) {
                        //std::cerr << "Failed to inflate zlib stream" << std::endl;
                        break;
                    }
                    decompressed.insert(decompressed.end(), buffer, buffer + sizeof(buffer) - strm.avail_out);
                } while (strm.avail_out == 0);
                inflateEnd(&strm);
                auto nested_messages = parse_packet(decompressed);
                messages.insert(messages.end(), nested_messages.begin(), nested_messages.end());
            }
        }
        offset += (packet_length - header_length);
    }

    return messages;
}

// 编码数据包
std::vector<uint8_t> encode_packet(uint16_t protocol_version, uint32_t operation, const std::string& body) {
    std::vector<uint8_t> packet;
    uint32_t packet_length = 16 + body.size();
    uint16_t header_length = 16;
    uint32_t sequence_id = 1;

    // Packet Length
    uint32_t packet_length_n = htonl(packet_length);
    packet.insert(packet.end(), reinterpret_cast<uint8_t*>(&packet_length_n), reinterpret_cast<uint8_t*>(&packet_length_n) + 4);

    // Header Length
    uint16_t header_length_n = htons(header_length);
    packet.insert(packet.end(), reinterpret_cast<uint8_t*>(&header_length_n), reinterpret_cast<uint8_t*>(&header_length_n) + 2);

    // Protocol Version
    uint16_t protocol_version_n = htons(protocol_version);
    packet.insert(packet.end(), reinterpret_cast<uint8_t*>(&protocol_version_n), reinterpret_cast<uint8_t*>(&protocol_version_n) + 2);

    // Operation
    uint32_t operation_n = htonl(operation);
    packet.insert(packet.end(), reinterpret_cast<uint8_t*>(&operation_n), reinterpret_cast<uint8_t*>(&operation_n) + 4);

    // Sequence Id
    uint32_t sequence_id_n = htonl(sequence_id);
    packet.insert(packet.end(), reinterpret_cast<uint8_t*>(&sequence_id_n), reinterpret_cast<uint8_t*>(&sequence_id_n) + 4);

    // Body
    packet.insert(packet.end(), body.begin(), body.end());

    return packet;
}

