#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_STL_

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <asio.hpp>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

class WebSocketClient {
public:
    WebSocketClient();
    ~WebSocketClient();
    std::string access_token_;

    void connect(const std::string& uri);
    void send(const std::string& message);
    void subscribe_orderbook(const std::string& instrument);
    void unsubscribe_orderbook(const std::string& instrument);
    std::string receive();  // Blocking call, clearly waits for new messages
    void place_order(const std::string& instrument, double amount, const std::string& direction,
                     const std::string& order_type, double price = 0.0);
    void cancel_order(const std::string& order_id);
    void modify_order(const std::string& order_id, double new_amount, double new_price);
    void get_positions();
    void get_order_book(const std::string& instrument, int depth = 10);
    void authenticate(const std::string& client_id, const std::string& client_secret);

private:
    typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
    client ws_client;
    websocketpp::connection_hdl hdl_;

    std::queue<std::string> messages_;
    std::mutex msg_mutex_;
    std::condition_variable msg_cond_;

    void on_open(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg);
    websocketpp::lib::shared_ptr<asio::ssl::context> on_tls_init();
};

#endif
