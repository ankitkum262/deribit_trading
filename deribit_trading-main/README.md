# Deribit C++ Trading System – Setup Guide

This application is a low-latency trading system developed in C++ that interfaces with the Deribit Testnet API. It provides both REST and WebSocket-based interfaces for order management and real-time market data streaming. The system is designed to support Spot, Futures, and Options instruments, with full support for Deribit’s public and private APIs.

The main aim of this Order Mangement System (OMS) will not be to simply send, recieve or stream communicable data with Deribit's server but to actually implement optimisations to reduce latency.

I would recommend using VS Code due to it's feature rich interface.

You can have a look at video explanation of the codebase here : https://drive.google.com/drive/folders/1j9_-WpCXGKoM0Dp7XO3bgiLewI-P7mbH?usp=sharing

## Prerequisites

Before building and running the application, ensure the following dependencies are installed on your system.

### Required Tools and Libraries

- C++17 or later (Tested with GCC 9+ or Clang 12+)
- CMake (Build system)
- libcurl (For REST API communication)
- WebSocket++ (For WebSocket client/server communication, latency reduction)
- nlohmann/json (For JSON parsing)

Skip to the end of (4) if you are more interested in the implementation choices, optimisations and benchmarking.
---

## 1. Install Dependencies

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install -y cmake g++ libcurl4-openssl-dev libssl-dev
```

You will also need to manually install the following header-only libraries (if not already present):

- WebSocket++: https://github.com/zaphoyd/websocketpp
- nlohmann/json: https://github.com/nlohmann/json

You can place these headers in an `external/` directory and include them manually in your project.

### macOS (with Homebrew)

```bash
brew install cmake curl openssl
```

Download and include header-only dependencies manually as above.

### Windows (with vcpkg)

```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
vcpkg install curl openssl nlohmann-json websocketpp
```

Configure CMake to use vcpkg by adding:

```bash
-DCMAKE_TOOLCHAIN_FILE=path_to_vcpkg/scripts/buildsystems/vcpkg.cmake
```

---

## 2. CMake Build Configuration

Create a `CMakeLists.txt` file in the root directory of your project:

```cmake_minimum_required(VERSION 3.10)
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

```

Note: Modify the source file paths according to your actual project structure.

---

## 3. Build Instructions

### Step 1: Create Build Directory

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

(or) select the following in VS Code
```bash
CMake: Delete Cache and Reconfigure
```

### Step 2: Run the Executable

```bash
./OMS (or the equivalen name of your project)
```

---

## 4. Configure API Credentials

Before running the system, configure your Deribit Testnet credentials.

### Obtain Credentials:

1. Create a test account at: https://test.deribit.com
2. Navigate to **Account → API → Create New API Key**
3. Copy the `client_id` and `client_secret` into `config.ini`

**Setup ends here.**

**The following is the actual explanation of the code and reasoning.**
---

**1. The General Goal of the Project:**
- Reduce latency between request and response. For example, if an order is placed at a certain time, the response from the server (filled, open, or invalid) should be received as fast as possible.
- Stream current open orders, display your open orders and positions.
- Measure latency in order placement (and execution), processing the received message, and round trip time (RTT).
- Optimizations via CPU usage, communication protocol, thread management, memory, and selection of optimal data structures for parsing and storing data.

**2. Heavy Usage of WebSockets for the Project:**

WebSocket is the primary communication protocol used in this project. It is preferred due to its persistent connection by design—something HTTP (REST) lacks.

HTTP is used only to obtain the access_token and refresh_token. WebSockets are used not only for streaming market or private order updates but also for sending all requests. Once the WebSocket connection is established, sending requests through it is cheaper compared to HTTP.

**3. How is Latency Reduced?**

- **Server-to-Device Latency:**

  The first realization is that it is physically impossible for end-to-end latency to go below the lower bound set by a simple ping.
  ```bash
  ping test.deribit.com
  ```
  This represents the physical latency limit—determined by speed-of-light delay and some device/program overhead. In my case, this is around 
    ```bash
    199-200 ms on average.
  ```
  The goal is to match this as closely as possible, and WebSocket is critical in achieving that. Note: establishing a WebSocket connection is marginally slower than a single HTTP request, but all subsequent communication is significantly faster.

- **Data Processing Latency:**

  This refers to the time between receiving a server message and processing it. Optimization here depends heavily on appropriate data structures and memory management.

**4. Benchmarking**

- **Bottlenecks:**
  - Physical distance between the server and client. My program no matter how optimised cannot overcome the physical limitation as discussed in (3).
  - Thread crowding. If user interaction is handled through crowded threads (e.g., placing buy orders, streaming, sending heartbeats via the same WebSocket handle), latency increases due to sychronisation latency.

- **Benchmarking Methodology:**
  A dedicated file `latency_tracker.cpp` and its header define a LatencyTracker class which starts and stops timers. Latencies are stored in an `unordered_map` (label → latency) and printed at the end.

  A timer is placed at the endpoints of the function whose latency we want to measure. For example, when measuring latency in Order Placement, a timer is started just before hitting `.send()` and stopped upon receiving a corresponding response.

  Only buy-request latency is benchmarked, assuming all order types follow similar trends.

- **Order Placement Latency:**
    
    Massive Improvements can be achieved here by making very basic changes.
  Latency is minimized by using non-threaded code and WebSocket instead of HTTP. The following benchmarks are displayed for only around 80-100 calls, while the actual trend using calls upto 10000 are more accurate but similar the findings below.

  A small sample latency report for an HTTP buy order:
  ```bash
  [671106, 631415, 614714, 642678, 643342, 657112, 631272, 714742, 710213, 715799,
  634858, 648824, 655482, 656395, 661727, 610720, 626688, 797461, 634589, 614215,
  688485, 626897, 642197, 653309, 698922, 640313, 628391, 624543, 659525, 651187,
  622577, 666604, 611552, 634537, 637794, 661128, 634949, 676620, 683105, 756421,
  713330, 664415, 675836, 638807, 640529, 645047, 626042, 642068, 703654, 727676,
  714848, 636120, 657566, 656333, 698220, 641095, 706608, 712799, 677520, 627728,
  629842, 636165, 615696, 648191, 722373, 715105, 648503, 615713, 639235, 631649, 
  617998, 637400, 622409, 638211, 728042, 664435, 629795, 652474, 641295, 636558,
  734459, 623258, 637786, 659434, 632332, 648125, 652207, 615250, 640487, 615696]
  ```
  With an average of: **657225 μs**, which is extremely bad.

  A small sample for threaded WebSocket calls:
  ```bash
  [209057, 210066, 210833, 211453, 216798, 217296, 231186, 232028, 213233, 213347,
  213665, 213708, 213918, 214216, 225803, 226224, 224970, 225216, 225328, 225410,
  225500, 225608, 233675, 233918, 215660, 215418, 215927, 216404, 217028, 217542,
  230705, 231156, 215275, 216923, 218478, 219564, 224057, 225268, 237481, 237943,
  214121, 213740, 214072, 215677, 220047, 220259, 234151, 234423, 231317, 232030,
  232793, 233247, 233831, 234415, 235022, 235561, 213849, 214002, 214316, 214461,
  214741, 214864, 225547, 226413, 205759, 205867, 207058, 208417, 213949, 214334,
  227346, 227696, 225375, 225930, 226320, 226650, 227094, 227472, 227842, 228102]
  ```
  With an average of: **221817 μs**, which is very close to the **physical limit of ~200ms** — showing a significant reduction in latency. Still about 2000 μs above the limit, but this is a major improvement over HTML with minimal effort.

  A small sample for non-threaded WebSocket call:
  ```bash
  [213063, 213843, 214910, 216185, 221214, 221393, 235046, 235219, 213130, 213861,
  214303, 215563, 221288, 221753, 236084, 236569, 206794, 207814, 214413, 218948,
  222098, 222548, 234665, 234917, 213773, 215383, 216261, 216946, 220027, 220910,
  234267, 234883, 211187, 212656, 213536, 214329, 220103, 221053, 234761, 235209,
  204740, 206147, 206949, 207614, 213531, 214397, 225376, 225887, 228927, 229323,
  229423, 229464, 229750, 229341, 229689, 230042, 211609, 212953, 213406, 213644,
  219159, 219854, 234116, 235085, 211549, 212467, 213035, 213480, 214103, 214627,
  228804, 229653, 211731, 212788, 213425, 213985, 217080, 217574, 230305, 230678,
  215432, 216906, 217729, 218466, 219230, 219876, 232015, 232701, 211202, 212575,
  213205, 213756, 216351, 216982, 212278, 213880, 214643, 215403, 216160, 216815]
  ```
  With an average of: **219881 μs**, The small difference of just ~2ms shows that threading does not significantly impact latency in a system with a light threads. It also indicates that other areas (like network propagation, server response time, and system-level delays) dominate the latency profile, rather than the threading model used within the application. But I guess a win is a win.


- **Market Data Processing Latency:**
    
    The choices for this is discussed below. In a gist, using efficient purpose-specific data strucutres (like a map with order_id as keys to store a few important information about the order) is an essential step. Using modular code is beneficial everywhere, including here.

A sample latency report of parsing and storing relevant information from a WS buy order:
  ```bash
  [75, 79, 72, 71, 68, 67, 74, 68, 67, 80, 72, 79, 73, 76, 73, 67, 67, 65, 73, 77,
  70, 69, 67, 71, 66, 68, 64, 73, 76, 70, 68, 65, 66, 67, 78, 66, 81, 76, 69, 81,
  67, 90, 80, 86, 129, 109, 108, 92, 71, 81, 71, 98, 71, 134, 112, 110, 84, 72,
  73, 79, 125, 110, 90, 88, 82, 83, 82, 105, 69, 91, 80, 119, 124, 111, 113, 108,
  97, 91, 84, 69, 73, 77, 73, 67, 67, 98, 67, 128, 106, 75, 73, 68, 72, 76, 84,
  73, 77, 69, 105, 97, 93, 91, 92, 72, 76, 89, 79]
  ```
  With an average of: **82.33 μs** indicating that the latency in parsing responses is **not a bottleneck**, however there is a lot of scope to improve this as well. Some methods will be mentioned in the last section.
  
- **Market Data Processing Latency:**

    This is the round trip latency, roughly this should be the sum of Data Processing Latency and Order placement latency and some functional overload.
    
A sample latency report of RTT for a WS response message:
  ```bash
[221917, 222358, 222728, 223054, 224451, 239286, 244184, 249445, 249660, 263871,
213317, 214035, 214784, 215480, 216179, 216696, 229103, 229428, 242999, 243376,
221010, 221605, 221987, 222330, 222754, 223177, 237944, 238383, 251457, 251894,
209200, 209328, 209454, 210642, 217522, 217606, 229192, 229307, 243137, 243266,
225443, 226101, 226644, 227018, 227421, 227765, 240131, 240378, 254316, 254445,
215214, 216002, 216420, 216852, 217343, 217706, 228535, 228793, 242607, 242881,
235388, 235589, 235773, 235948, 236096, 236254, 236393, 236538, 242203, 242507,
211789, 212520, 213067, 213584, 216007, 216505, 230427, 230948, 244912, 245782,
222460, 222742, 222959, 223120, 223308, 223467, 231730, 232092, 246382, 246720,
215558, 215853, 216145, 216368, 218863, 219237, 234461, 234712, 247561, 247706,
217909, 218349, 218705, 219009, 219401, 219812, 232319, 232853, 247320, 247484]
  ```
  With an average of: **228749 μs**, the difference of this with the sum of latencies of data processing and response recieving is in the order of **1000 μs**, which indicates the functional overhead is a potential bottleneck, because ideally the difference should've been in orders of only 10s or 100s.

**5. Data-Processing Optimizations:**

  Further improvements can be made beyond a few milliseconds. Some approaches include:

  **CPU Optimizations:**
  Avoiding long redundant functions, modularizing code and eliminating redundancy across and within files/functions is the first steps towards CPU optimizations. The code I have written is aware of this and redundancy is removed as much as possible. 
  Frequently used functions are modularised into a separate file for enabling internal CPU optimisations and increasing modularity.
  We can use `-O3` optimizations and `-march=native` to let the compiler auto-optimize for our CPU. 
  
  **Selecting the correct DS/libraries for storing and parsing data:**
  nlohmann::json is already optimised for parsing json string responses decently enough. Hence this library is good enough for our purposes. The parsed data is then stored in a map (order_id -> json object storing important information) so that future modifications are accessed and modified as fast as possible. 
Since all of these are already thought of and implemented from the start there is no before/after benchmarking necessary for this. I will review this in the video explanation in more details.


### Discussion of potential further improvements :

I have discussed how to improve with some working professionals and the following suggestions were most common. 
There is additional latency introduced by system calls, context switching, kernel and driver overhead, and the network stack. In some cases, techniques like enabling page swapping instead of memory copying or using kernel bypass (e.g., DPDK, RDMA) can help reduce this overhead. Hardware-level tuning is also possible but highly platform-specific.

However, even if the application and system are optimized, a significant portion of total latency can come from the external environment—your home network, including the switch, router, modem, and everything else along the path to the exchange server. This has been thoroughly discussed before.

**CPU-Level Optimizations**
Avoid complex `if-else` chains and write code that’s predictable for the CPU. Reuse memory instead of creating new objects every time. Keeping data structures aligned with CPU cache lines can also make things faster.

**Threading Techniques**
We should run WebSocket I/O in a separate thread so it doesn’t block order processing. Using lock-free queues helps avoid delays when passing data between threads. Assigning threads to specific CPU cores (CPU pinning) and increasing thread priority can reduce context switching and improve performance. Also, having a balanced thread pool helps spread work evenly. But I could not implement this as this is a little advanced at this stage and I lack time as well.

**Communication Protocol Enhancements**
If the exchange allows, switching from JSON to a binary format like MessagePack will reduce message size and parsing time. We should also avoid sending or reading unnecessary fields to save time and CPU cycles.

**Parsing and Data Handling**
Using a faster parser like RapidJSON in streaming mode is better than full JSON parsing. Only extract fields we actually use. Pre-allocate objects where possible and avoid copying data unnecessarily.
