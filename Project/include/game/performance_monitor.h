#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <iostream>

class PerformanceMonitor {
private:
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> start_times_;
    std::unordered_map<std::string, double> total_times_;
    std::unordered_map<std::string, int> call_counts_;

public:
    void startTiming(const std::string& name) {
        start_times_[name] = std::chrono::high_resolution_clock::now();
    }
    
    void endTiming(const std::string& name) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto it = start_times_.find(name);
        if (it != start_times_.end()) {
            auto duration = std::chrono::duration<double, std::milli>(end_time - it->second);
            total_times_[name] += duration.count();
            call_counts_[name]++;
        }
    }
    
    void printStats() {
        std::cout << "\n=== Performance Statistics ===" << std::endl;
        for (const auto& [name, total_time] : total_times_) {
            int calls = call_counts_[name];
            double avg_time = total_time / calls;
            std::cout << name << ": " << calls << " calls, "
                      << "avg: " << avg_time << "ms, "
                      << "total: " << total_time << "ms" << std::endl;
        }
    }
    
    void reset() {
        total_times_.clear();
        call_counts_.clear();
    }
    
    double getAverageTime(const std::string& name) {
        auto total_it = total_times_.find(name);
        auto count_it = call_counts_.find(name);
        if (total_it != total_times_.end() && count_it != call_counts_.end() && count_it->second > 0) {
            return total_it->second / count_it->second;
        }
        return 0.0;
    }
};

// Singleton instance
extern PerformanceMonitor g_PerformanceMonitor;

// RAII helper for automatic timing
class ScopedTimer {
private:
    std::string name_;
public:
    ScopedTimer(const std::string& name) : name_(name) {
        g_PerformanceMonitor.startTiming(name_);
    }
    
    ~ScopedTimer() {
        g_PerformanceMonitor.endTiming(name_);
    }
};

// Macro for easy timing
#define PROFILE_SCOPE(name) ScopedTimer _timer(name)