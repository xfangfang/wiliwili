//
// Created by maye174 on 2023/4/6.
//

#pragma once
#include <vector>
#include <string>
#include <cstdint>

std::vector<std::string> parse_packet(const std::vector<uint8_t>& data);

std::vector<uint8_t> encode_packet(uint16_t protocol_version, uint32_t operation, const std::string& body);