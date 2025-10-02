
# Deribit C++ Trading System

A high-performance, low-latency C++ trading system designed for cryptocurrency derivatives on the Deribit platform. This Order Management System (OMS) implements aggressive optimizations to minimize end-to-end latency, achieving up to a 66% reduction compared to standard HTTP requests.

## Features

- **Low-Latency Trading**: Optimized for minimal order placement latency (~220ms via WebSocket vs. ~657ms via HTTP).
- **Dual Protocol Support**: Uses HTTP/REST for secure authentication and WebSocket for high-speed trading operations.
- **Real-time Market Data**: Leverages persistent WebSocket streams for live market updates.
- **Multi-instrument Support**: Full compatibility with Spot, Futures, and Options trading.
- **Performance Monitoring**: Built-in latency tracking with microsecond precision for benchmarking.
- **Modular Architecture**: Clean separation of concerns with dedicated service libraries.

***

## Performance Benchmarks

| Protocol | Average Latency | Improvement |
|:---|:---|:---|
| HTTP/REST | ~657 ms | Baseline |
| WebSocket | ~220 ms | **66% reduction** |
| Physical Limit | ~200 ms (ping) | Near-optimal performance |

***

## Prerequisites

- **C++17** or later (GCC 9+ or Clang 12+)
- **CMake** (Build system)
- **libcurl** (REST API communication)
- **WebSocket++** (WebSocket communication)
- **nlohmann/json** (JSON parsing)
- **OpenSSL** (Secure connections)

***

## Installation

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install -y cmake g++ libcurl4-openssl-dev libssl-dev
````

### macOS (Homebrew)

```bash
brew install cmake curl openssl
```

### Windows (vcpkg)

```bash
git clone [https://github.com/microsoft/vcpkg.git](https://github.com/microsoft/vcpkg.git)
cd vcpkg
.\bootstrap-vcpkg.bat
vcpkg install curl openssl nlohmann-json websocketpp
```

*Note: You may need to manually download the header-only libraries for `WebSocket++` and `nlohmann/json`.*

-----

## Build Instructions

1.  **Clone and set up the project**:

    ```bash
    git clone [https://github.com/ankitkum262/deribit_trading.git](https://github.com/ankitkum262/deribit_trading.git)
    cd deribit_trading
    mkdir build && cd build
    ```

2.  **Configure and build with CMake**:

    ```bash
    cmake ..
    make -j$(nproc)
    ```

3.  **Run the application**:

    ```bash
    ./rest_order
    ```

-----

## Configuration

1.  Create a Deribit testnet account at `https://test.deribit.com`.
2.  Generate API credentials (**Account → API → Create New API Key**).
3.  Add your credentials to a `config.ini` file in the project's root directory:
    ```ini
    client_id=your_client_id
    client_secret=your_client_secret
    ```

-----

## System Architecture

### Core Components

  - **`rest_order`**: The main executable for the trading application.
  - **`WebSocketClient`**: The primary interface for all low-latency trading operations.
  - **`Authentication`**: Manages OAuth2 token generation and renewal.
  - **`LatencyTracker`**: A utility for precise performance monitoring and benchmarking.
  - **`RestClient`**: Handles initial HTTP-based authentication.

### Communication Flow

The system first authenticates via **HTTP** to get an access token and then establishes a persistent **WebSocket** connection for all subsequent trading operations.

-----

## Usage Examples

### Placing and Cancelling Orders

```cpp
// Place a market buy order for 0.01 BTC
place_order("BTC-PERPETUAL", "buy", 0.01, "market");

// Cancel a specific order
cancel_order("your_order_id");
```

### Streaming Data

```cpp
// Subscribe to the order book for BTC-PERPETUAL
stream_market_data("BTC-PERPETUAL");
```

-----

## Performance Optimizations

This system was designed from the ground up for low latency through several key optimizations:

  - **Protocol-Level**: Prioritizes persistent WebSocket connections over stateless HTTP requests to eliminate connection overhead.
  - **System-Level**: Utilizes compiler optimizations (`-O3`, `-march=native`) and efficient thread management to reduce CPU and context-switching overhead.
  - **Data-Processing**: Employs the highly optimized `nlohmann/json` library for fast JSON parsing and `std::map` for efficient order storage and retrieval.

<!-- end list -->

```
```
