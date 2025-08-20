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
    
    std::string client_id = "0uXsIP-O";
    std::string client_secret = "pq2Hswet8WhwSnLcdaAhyiLCz3gnbDj05OMKWdpM4w0";

    try {
        std::string token = get_access_token(client_id, client_secret);
        std::cout << "Access Token: " << token << "\n";

        std::string order_id = "38867646049";
        std::string response = cancel_order(token, order_id);

        std::cout << "Response: " << response << "\n";

    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}