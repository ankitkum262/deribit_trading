#include "websocket_server.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include "websocket_client.hpp"

using json = nlohmann::json;

extern WebSocketClient* g_ws_client;

void WebSocketServer::on_open(connection_hdl hdl) {
    std::cout << "[Server] Client connected clearly." << std::endl;
}

void WebSocketServer::on_close(connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(connection_mutex_);
    for (auto& pair : subscriptions_) {
        pair.second.erase(hdl);
    }
    std::cout << "[Server] Client disconnected clearly." << std::endl;
}

void WebSocketServer::on_message(connection_hdl hdl, server::message_ptr msg) {
    json received = json::parse(msg->get_payload());
    std::lock_guard<std::mutex> lock(connection_mutex_);
    
    if (received["action"] == "subscribe") {
        std::string instrument = received["instrument"];
        subscriptions_[instrument].insert(hdl);
        std::cout << "[Server] Client subscribed clearly to: " << instrument << std::endl;
        
        if (g_ws_client && subscriptions_[instrument].size() == 1) {
            g_ws_client->subscribe_orderbook(instrument);
        }
    } else if (received["action"] == "unsubscribe") {
        std::string instrument = received["instrument"];
        subscriptions_[instrument].erase(hdl);
        std::cout << "[Server] Client unsubscribed from " << instrument << " clearly." << std::endl;
        if (subscriptions_[instrument].empty() && g_ws_client) {
            g_ws_client->unsubscribe_orderbook(instrument);
        }
    } else if (received["action"] == "place_order") {
        std::string instrument = received["instrument"];
        subscriptions_["private"].insert(hdl);
        double amount = received["amount"];
        std::string direction = received["direction"];
        std::string order_type = received["order_type"];
        double price = received.contains("price") ? received["price"].get<double>() : 0.0;
        std::cout << "[Server] Received place_order request for " << instrument << std::endl;
        if (g_ws_client) {
            g_ws_client->place_order(instrument, amount, direction, order_type, price);
        }
    } else if (received["action"] == "cancel_order") {
        subscriptions_["private"].insert(hdl);
        std::string order_id = received["order_id"];
        std::cout << "[Server] Received cancel_order request for order " << order_id << std::endl;
        if (g_ws_client) {
            g_ws_client->cancel_order(order_id);
        }
    } else if (received["action"] == "modify_order") {
        subscriptions_["private"].insert(hdl);
        std::string order_id = received["order_id"];
        double new_amount = received["new_amount"];
        double new_price = received["new_price"];
        std::cout << "[Server] Received modify_order request for order " << order_id << std::endl;
        if (g_ws_client) {
            g_ws_client->modify_order(order_id, new_amount, new_price);
        }
    } else if (received["action"] == "get_positions") {
        subscriptions_["private"].insert(hdl);
        std::cout << "[Server] Received get_positions request." << std::endl;
        if (g_ws_client) {
            g_ws_client->get_positions();
        }
    } else if(received["action"] == "get_orderbook") {
        subscriptions_["private"].insert(hdl);
        std::string instrument = received["instrument"];
        int depth = received.contains("depth") ? received["depth"].get<double>() : 10;
        std::cout << "[Server] Received get_orderbook request for " << instrument << " with depth " << depth << std::endl;
        if (g_ws_client) {
            g_ws_client->get_order_book(instrument, depth);
        }
    }
}

void WebSocketServer::run(uint16_t port) {
    m_server.init_asio();

    m_server.set_open_handler(std::bind(&WebSocketServer::on_open, this, std::placeholders::_1));
    m_server.set_close_handler(std::bind(&WebSocketServer::on_close, this, std::placeholders::_1));
    m_server.set_message_handler(std::bind(&WebSocketServer::on_message, this, std::placeholders::_1, std::placeholders::_2));

    m_server.listen(port);
    m_server.start_accept();

    std::cout << "[Server] WebSocket running clearly on port " << port << std::endl;
    m_server.run();
}

void WebSocketServer::broadcast(const std::string& instrument, const std::string& message) {
    std::lock_guard<std::mutex> lock(connection_mutex_);
    std::cout << "[Server] Broadcasting to instrument " << instrument << ": " << message << std::endl;
    if (subscriptions_.count(instrument)) {
        for (auto hdl : subscriptions_[instrument]) {
            m_server.send(hdl, message, websocketpp::frame::opcode::text);
        }
    } else {
        std::cout << "[Server] No subscribers for instrument " << instrument << std::endl;
    }
}
