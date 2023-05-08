
#include "live/extract_messages.hpp"

#include <nlohmann/json.hpp>

std::vector<std::string> extract_danmu_messages(const std::vector<std::string>& messages) {
    std::vector<std::string> danmu_messages;

    for (const auto& message : messages) {
        try {
            auto json_message = nlohmann::json::parse(message);

            if (json_message.contains("cmd") && json_message["cmd"] == "DANMU_MSG") {
                if (json_message.contains("info") && json_message["info"].is_array() && json_message["info"].size() > 1) {
                    danmu_messages.push_back(json_message["info"][1].get<std::string>());
                }
            }
        } catch (const nlohmann::json::parse_error& e) {
        //   /std::cerr << "Failed to parse JSON message: " << e.what() << std::endl;
        }
    }

    return danmu_messages;
}
