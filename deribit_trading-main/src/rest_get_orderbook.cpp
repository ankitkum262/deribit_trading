#include "auth.hpp"
#include "rest_client.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include "websocket_client.hpp"
#include "websocket_server.hpp"
#include <thread>
#include <chrono>

using json = nlohmann::json;

WebSocketServer* g_ws_server = nullptr;
WebSocketClient* g_ws_client = nullptr;

int main() {

    try {

        std::string instrument = "BTC-PERPETUAL";
        int depth = 10;
        std::string response = get_order_book(instrument, depth);
        std::cout << "Response: " << response << "\n";

    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}