#include "tracker.hpp"
#include <iostream>

void LatencyTracker::start_measurement(LatencyType type, const std::string& unique_id) {
    std::lock_guard<std::mutex> lock(metrics_mutex);
    
    LatencyMetric metric;
    metric.start_time = std::chrono::high_resolution_clock::now();
    
    if (unique_id.empty()) {
        latency_metrics[type].push_back(metric);
    } else {
        active_measurements[unique_id] = metric;
    }
}

void LatencyTracker::stop_measurement(LatencyType type, const std::string& unique_id) {
    std::lock_guard<std::mutex> lock(metrics_mutex);
    auto end_time = std::chrono::high_resolution_clock::now();
    
    if (unique_id.empty()) {
        for (auto& metric : latency_metrics[type]) {
            if (!metric.completed) {
                metric.end_time = end_time;
                metric.duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    metric.end_time - metric.start_time
                );
                metric.completed = true;
                break;
            }
        }
    } else {
        auto it = active_measurements.find(unique_id);
        if (it != active_measurements.end()) {
            it->second.end_time = end_time;
            it->second.duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                it->second.end_time - it->second.start_time
            );
            it->second.completed = true;
            latency_metrics[type].push_back(it->second);
            active_measurements.erase(it);
        }
    }
}

std::string LatencyTracker::generate_report() {
    std::lock_guard<std::mutex> lock(metrics_mutex);
    
    std::ostringstream report;
    report << "Latency Benchmarking Report\n";
    report << "------------------------------\n\n";

    const char* type_names[] = {
        "Order Placement",
        "Market Data Processing", 
        "WebSocket Message Propagation", 
        "Trading Loop End-to-End"
    };

    for (int t = 0; t < 4; ++t) {
        auto type = static_cast<LatencyTracker::LatencyType>(t);
        auto metrics = latency_metrics[type];
        if (metrics.empty()) continue;
        
        std::vector<std::chrono::nanoseconds> durations;
        for (const auto& metric : metrics) {
            if (metric.completed) {
                durations.push_back(metric.duration);
            }
        }
        
        if (durations.empty()) {
            report << type_names[t] << " Latency: No completed measurements\n\n";
            continue;
        }
        
        std::sort(durations.begin(), durations.end());
        size_t count = durations.size();
        auto total = std::accumulate(durations.begin(), durations.end(), std::chrono::nanoseconds(0));
        auto mean = total / count;
        auto min_val = durations.front();
        auto max_val = durations.back();
        
        report << type_names[t] << ":\n";
        report << "  Total Measurements: " << count << "\n";
        report << "  Mean Latency: " << std::fixed << std::setprecision(3) << mean.count() / 1e6 << " ms\n";
        report << "  Min Latency: " << min_val.count() / 1e6 << " ms\n";
        report << "  Max Latency: " << max_val.count() / 1e6 << " ms\n";
    }
    
    return report.str();
}

std::map<LatencyTracker::LatencyType, std::vector<LatencyTracker::LatencyMetric>> LatencyTracker::get_raw_metrics() {
    std::lock_guard<std::mutex> lock(metrics_mutex);
    return latency_metrics;
}

void LatencyTracker::reset() {
    std::lock_guard<std::mutex> lock(metrics_mutex);
    latency_metrics.clear();
    active_measurements.clear();
    std::cout << "Latency metrics have been reset." << std::endl;
}

LatencyTracker& getLatencyTracker() {
    static LatencyTracker tracker;
    return tracker;
}
