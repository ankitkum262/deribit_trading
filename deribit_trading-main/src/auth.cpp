#include "auth.hpp"
#include "utils.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>

std::string get_access_token(const std::string& client_id, const std::string& client_secret) {
    std::string url = "https://test.deribit.com/api/v2/public/auth?client_id=" 
                    + client_id + "&client_secret=" + client_secret + "&grant_type=client_credentials";

    std::string response = http_get(url);

    auto json_response = nlohmann::json::parse(response);
    if (json_response.contains("result") && json_response["result"].contains("access_token")) {
        return json_response["result"]["access_token"];
    } else {
        throw std::runtime_error("Authentication failed: " + response);
    }
}