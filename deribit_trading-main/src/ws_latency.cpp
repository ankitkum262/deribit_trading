#include "auth.hpp"
#include "websocket_client.hpp"
#include "websocket_server.hpp"
#include "tracker.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>

using namespace std;
using namespace std::chrono;
using json = nlohmann::json;

WebSocketClient* g_ws_client = nullptr;
WebSocketServer* g_ws_server = nullptr; //not used though

int main() {
    // Obtain REST token for private WebSocket requests.
    std::string client_id = "0uXsIP-O";
    std::string client_secret = "pq2Hswet8WhwSnLcdaAhyiLCz3gnbDj05OMKWdpM4w0";
    std::string token = get_access_token(client_id, client_secret);
    cout << "Access Token: " << token << "\n";

    // Create WebSocket client instance and set the access token.
    WebSocketClient ws_client;
    ws_client.access_token_ = token;
    g_ws_client = &ws_client;

    std::thread deribitThread([&ws_client]() {
        ws_client.connect("wss://test.deribit.com/ws/api/v2");
    });

    std::this_thread::sleep_for(seconds(2));

    LatencyTracker& tracker = getLatencyTracker();

    for (int i = 0; i < 10; ++i) {
        std::string order_unique_id = "ws_order_" + std::to_string(i);
        tracker.start_measurement(LatencyTracker::ORDER_PLACEMENT, order_unique_id);

        ws_client.place_order("BTC-PERPETUAL", 10, "buy", "market");

        std::this_thread::sleep_for(milliseconds(200));

        tracker.stop_measurement(LatencyTracker::ORDER_PLACEMENT, order_unique_id);
    }

    cout << tracker.generate_report() << endl;

    deribitThread.join();
    return 0;
}
