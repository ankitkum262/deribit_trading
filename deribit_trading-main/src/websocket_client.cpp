#include "websocket_client.hpp"
#include <asio.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include "websocket_server.hpp" 

using json = nlohmann::json;

extern WebSocketServer* g_ws_server;

websocketpp::lib::shared_ptr<asio::ssl::context> WebSocketClient::on_tls_init() {
    websocketpp::lib::shared_ptr<asio::ssl::context> ctx =
        websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::tlsv12);
    ctx->set_verify_mode(asio::ssl::verify_none);
    return ctx;
}

WebSocketClient::WebSocketClient() {
    ws_client.init_asio();

    ws_client.set_message_handler([this](websocketpp::connection_hdl hdl, client::message_ptr msg) {
        std::string payload = msg->get_payload();
        std::cout << "[Deribit] Received: " << payload << std::endl;
        
        try {
            json j = json::parse(payload);
            if (j.contains("params") && j["params"].contains("channel")) {
                std::string channel = j["params"]["channel"];
                // Expect channel format: "book.<instrument>.100ms"
                if (channel.size() > 11) {
                    std::string instrument = channel.substr(5, channel.size() - 5 - 6);
                    std::cout << "[Deribit] Broadcasting to " << instrument << ": " << payload << std::endl;
                    if (g_ws_server) {
                        g_ws_server->broadcast(instrument, payload);
                    }
                }
            } else {
            // This is a private response (e.g., for order management).
            std::cout << "[Deribit] Private response: " << payload << std::endl;
            // Broadcast private responses on a dedicated "private" channel.
            if (g_ws_server) {
                g_ws_server->broadcast("private", payload);
            }
         }
        } catch (const std::exception& e) {
            std::cerr << "[Deribit] Error parsing message: " << e.what() << std::endl;
        }
    });

    ws_client.set_tls_init_handler(std::bind(&WebSocketClient::on_tls_init, this));
    ws_client.set_open_handler([this](websocketpp::connection_hdl hdl){
        hdl_ = hdl;
        std::cout << "[Client] Connected clearly to Deribit." << std::endl;
    });
}

WebSocketClient::~WebSocketClient() {
    ws_client.stop();
}

void WebSocketClient::send(const std::string& message) {
    ws_client.send(hdl_, message, websocketpp::frame::opcode::text);
}

std::string WebSocketClient::receive() {
    std::unique_lock<std::mutex> lock(msg_mutex_);
    while (messages_.empty()) {
        msg_cond_.wait(lock);
    }
    std::string msg = messages_.front();
    messages_.pop();
    return msg;
}

void WebSocketClient::connect(const std::string& uri) {
    websocketpp::lib::error_code ec;
    auto con = ws_client.get_connection(uri, ec);
    if (ec) {
        throw std::runtime_error(ec.message());
    }
    hdl_ = con->get_handle();
    ws_client.connect(con);
    ws_client.run();
}

void WebSocketClient::subscribe_orderbook(const std::string& instrument) {
    nlohmann::json subscribe_message = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "public/subscribe"},
        {"params", {
            {"channels", {"book." + instrument + ".100ms"}}
        }}
    };

    ws_client.send(hdl_, subscribe_message.dump(), websocketpp::frame::opcode::text);
    std::cout << "[Client] Subscribed explicitly to " << instrument << " orderbook.\n";
}

void WebSocketClient::unsubscribe_orderbook(const std::string& instrument) {
    nlohmann::json unsubscribe_message = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "public/unsubscribe"},
        {"params", {
            {"channels", {"book." + instrument + ".100ms"}}
        }}
    };

    ws_client.send(hdl_, unsubscribe_message.dump(), websocketpp::frame::opcode::text);
    std::cout << "[Client] Unsubscribed explicitly from " << instrument << " orderbook.\n";
}

void WebSocketClient::place_order(const std::string& instrument, double amount, const std::string& direction,
                                    const std::string& order_type, double price) {
    json request = {
        {"jsonrpc", "2.0"},
        {"id", 2},
        {"method", (direction == "buy" ? "private/buy" : "private/sell")},
        {"params", {
            {"instrument_name", instrument},
            {"amount", amount},
            {"type", order_type},
            {"access_token", access_token_}
        }}
    };
    if (order_type == "limit") {
        request["params"]["price"] = price;
    }
    send(request.dump());
    std::cout << "[Client] Place order request sent: " << request.dump() << std::endl;
}

void WebSocketClient::cancel_order(const std::string& order_id) {
    json request = {
        {"jsonrpc", "2.0"},
        {"id", 3},
        {"method", "private/cancel"},
        {"params", {
            {"order_id", order_id},
            {"access_token", access_token_}
        }}
    };
    send(request.dump());
    std::cout << "[Client] Cancel order request sent: " << request.dump() << std::endl;
}

void WebSocketClient::modify_order(const std::string& order_id, double new_amount, double new_price) {
    json request = {
        {"jsonrpc", "2.0"},
        {"id", 4},
        {"method", "private/edit"},
        {"params", {
            {"order_id", order_id},
            {"amount", new_amount},
            {"price", new_price},
            {"access_token", access_token_}
        }}
    };
    send(request.dump());
    std::cout << "[Client] Modify order request sent: " << request.dump() << std::endl;
}

void WebSocketClient::get_positions() {
    json request = {
        {"jsonrpc", "2.0"},
        {"id", 5},
        {"method", "private/get_positions"},
        {"params", {
            {"currency", "BTC"},
            {"access_token", access_token_}
        }}
    };
    send(request.dump());
    std::cout << "[Client] Get positions request sent: " << request.dump() << std::endl;
}

void WebSocketClient::get_order_book(const std::string& instrument, int depth) {
    json request = {
        {"jsonrpc", "2.0"},
        {"id", 6},
        {"method", "public/get_order_book"},
        {"params", {
            {"instrument_name", instrument},
            {"depth", depth}
        }}
    };
    send(request.dump());
    std::cout << "[Client] Get orderbook request sent for " << instrument << " with depth " << depth << "\n";
}

#include <nlohmann/json.hpp>
using json = nlohmann::json;

void WebSocketClient::authenticate(const std::string& client_id, const std::string& client_secret) { //Not working though.
    json auth_request = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "public/auth"},
        {"params", {
            {"grant_type", "client_credentials"},
            {"client_id", client_id},
            {"client_secret", client_secret}
        }}
    };

    send(auth_request.dump());
    std::cout << "[Client] Sent authentication request: " << auth_request.dump() << std::endl;

    std::string response = receive();
    std::cout << "[Client] Received auth response: " << response << std::endl;
    
    try {
        json j = json::parse(response);
        if (j.contains("result") && j["result"].contains("access_token")) {
            access_token_ = j["result"]["access_token"];
            std::cout << "[Client] Authentication successful. Access token stored." << std::endl;
        } else {
            std::cerr << "[Client] Authentication failed: " << response << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[Client] Error parsing auth response: " << e.what() << std::endl;
    }
}
