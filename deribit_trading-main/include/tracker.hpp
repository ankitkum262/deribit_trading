#ifndef LATENCY_TRACKER_H
#define LATENCY_TRACKER_H

#include <chrono>
#include <map>
#include <vector>
#include <mutex>
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <iomanip>

class LatencyTracker {
public:
    enum LatencyType {
        ORDER_PLACEMENT,
        MARKET_DATA_PROCESSING,
        WEBSOCKET_MESSAGE_PROPAGATION,
        TRADING_LOOP_END_TO_END
    };

    struct LatencyMetric {
        std::chrono::high_resolution_clock::time_point start_time;
        std::chrono::high_resolution_clock::time_point end_time;
        std::chrono::nanoseconds duration{0};
        bool completed{false};
    };

    void start_measurement(LatencyType type, const std::string& unique_id = "");
    void stop_measurement(LatencyType type, const std::string& unique_id = "");
    std::string generate_report();
    std::map<LatencyType, std::vector<LatencyMetric>> get_raw_metrics();
    void reset();

private:
    std::mutex metrics_mutex;
    std::map<LatencyType, std::vector<LatencyMetric>> latency_metrics;
    std::map<std::string, LatencyMetric> active_measurements;
};

LatencyTracker& getLatencyTracker();

#endif // LATENCY_TRACKER_H
