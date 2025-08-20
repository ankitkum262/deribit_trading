#ifndef WEBSOCKET_SERVER_HPP
#define WEBSOCKET_SERVER_HPP

#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_STL_

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <asio.hpp>
#include <set>
#include <map>
#include <mutex>
#include <string>

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::connection_hdl connection_hdl;

class WebSocketServer {
public:
    void run(uint16_t port);
    void broadcast(const std::string& instrument, const std::string& message);

private:
    server m_server;
    std::mutex connection_mutex_;
    // Mapping: instrument -> set of connection handles (clients subscribed to that instrument)
    std::map<std::string, std::set<connection_hdl, std::owner_less<connection_hdl>>> subscriptions_;

    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, server::message_ptr msg);
};

#endif