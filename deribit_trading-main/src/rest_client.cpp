#include "rest_client.hpp"
#include "utils.hpp"
#include <nlohmann/json.hpp>

std::string place_order(const std::string& access_token,
                        const std::string& instrument,
                        double amount,
                        const std::string& side,
                        const std::string& order_type,
                        double price) {

    std::string url = "https://test.deribit.com/api/v2/private/" + side;
    
    url += "?amount=" + std::to_string(amount);
    url += "&instrument_name=" + instrument;
    url += "&type=" + order_type;

    if (order_type == "limit") {
        url += "&price=" + std::to_string(price);
    }

    std::string response = http_post(url, "", access_token);

    return response;
}

std::string cancel_order(const std::string& access_token, const std::string& order_id) {
    std::string url = "https://test.deribit.com/api/v2/private/cancel?order_id=" + order_id;

    std::string response = http_post(url, "", access_token);

    return response;
}

std::string modify_order(const std::string& access_token, const std::string& order_id, double new_amount, double new_price) {
    std::string url = "https://test.deribit.com/api/v2/private/edit?order_id=" 
                    + order_id + "&amount=" + std::to_string(new_amount)
                    + "&price=" + std::to_string(new_price);

    std::string response = http_post(url, "", access_token);
    return response;
}

std::string get_order_book(const std::string& instrument, int depth) {
    std::string url = "https://test.deribit.com/api/v2/public/get_order_book?instrument_name="
                    + instrument + "&depth=" + std::to_string(depth);

    std::string response = http_get(url);
    return response;
}

std::string get_positions(const std::string& access_token, const std::string& currency) {
    std::string url = "https://test.deribit.com/api/v2/private/get_positions?currency=" + currency;

    std::string response = http_get_with_auth(url, access_token);
    return response;
}