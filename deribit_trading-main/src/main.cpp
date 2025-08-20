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

    std::string token = get_access_token(client_id, client_secret);
    std::cout << "Access Token: " << token << "\n";

    WebSocketClient ws_client;
    WebSocketServer ws_server;
    g_ws_client = &ws_client;
    g_ws_server = &ws_server;

    // Connect the WS client to Deribit
    std::thread deribitThread([&ws_client]() {
        ws_client.connect("wss://test.deribit.com/ws/api/v2");
    });
    ws_client.access_token_ = token;
    // Create and run your WS server (for external clients)
    std::thread serverThread([&ws_server]() {
        ws_server.run(9002);  // Your server will listen on port 9002.
    });

    deribitThread.join();
    serverThread.join();

    return 0;
}

/*
For Subscribing to Orderbook - 

{
  "action": "subscribe",
  "instrument": "BTC-PERPETUAL"
}


For Unsubscribing to Orderbook - 

{
  "action": "unsubscribe",
  "instrument": "BTC-PERPETUAL"
}


For Placing an Order - 

{
  "action": "place_order",
  "instrument": "BTC-PERPETUAL",
  "amount": 10,
  "direction": "buy",
  "order_type": "market"
}


For Cancelling an Order - 

{
  "action": "cancel_order",
  "order_id": "order_id_here"
}


For Modifying an Order - 

{
  "action": "modify_order",
  "order_id": "order_id_here",
  "new_amount": 20,
  "new_price": 31000.0
}


For Getting Positions - 

{
  "action": "get_positions"
}


*/