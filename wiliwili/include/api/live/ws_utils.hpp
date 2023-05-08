
#include <vector>
#include <string>


std::vector<std::string> parse_packet(const std::vector<uint8_t>& data);

std::vector<uint8_t> encode_packet(uint16_t protocol_version, uint32_t operation, const std::string& body);