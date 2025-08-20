cmake_minimum_required(VERSION 3.10)
project(DeribitTradingSystem)

set(CMAKE_CXX_STANDARD 14)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)
find_path(ASIO_INCLUDE_DIR asio.hpp REQUIRED)

include_directories(include ${ASIO_INCLUDE_DIR} /opt/homebrew/include)

add_executable(rest_order     
    src/rest_order.cpp
    src/tracker.cpp
    src/auth.cpp
    src/rest_client.cpp
    src/utils.cpp
    src/websocket_client.cpp
    src/websocket_server.cpp
)

target_link_libraries(rest_order    
    PRIVATE CURL::libcurl nlohmann_json::nlohmann_json OpenSSL::SSL OpenSSL::Crypto
)