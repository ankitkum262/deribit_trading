#include "auth.hpp"
#include "rest_client.hpp"
#include "tracker.hpp"  
#include <iostream>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>
#include "websocket_client.hpp"
#include "websocket_server.hpp"

WebSocketClient* g_ws_client = nullptr;
WebSocketServer* g_ws_server = nullptr;

using json = nlohmann::json;
using namespace std;
using namespace std::chrono;

int main() {
    std::string client_id = "0uXsIP-O";
    std::string client_secret = "pq2Hswet8WhwSnLcdaAhyiLCz3gnbDj05OMKWdpM4w0";

    try {
        std::string token = get_access_token(client_id, client_secret);
        std::cout << "Access Token: " << token << "\n";

        LatencyTracker& tracker = getLatencyTracker();

        // Place 10 market orders for BTC-PERPETUAL in a loop.
        for (int i = 0; i < 10; ++i) {
            std::string order_id = "order_" + std::to_string(i);

            // Start measuring latency for order placement.
            tracker.start_measurement(LatencyTracker::ORDER_PLACEMENT, order_id);

            std::string response = place_order(token, "BTC-PERPETUAL", 10, "buy", "market");
            
            // Stop the measurement once the response is received.
            tracker.stop_measurement(LatencyTracker::ORDER_PLACEMENT, order_id);

            std::cout << "Order " << i << " Response: " << response << "\n";

            std::this_thread::sleep_for(milliseconds(100));
        }

        std::cout << tracker.generate_report() << std::endl;

    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}
