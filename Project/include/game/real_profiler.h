#pragma once

#include <chrono>
#include <unordered_map>
#include <string>
#include <iostream>
#include <vector>

class RealProfiler {
private:
    struct ProfileData {
        std::chrono::high_resolution_clock::time_point start_time;
        double total_time = 0.0;
        size_t call_count = 0;
        double min_time = 999999.0;
        double max_time = 0.0;
        bool active = false;
    };
    
    std::unordered_map<std::string, ProfileData> profiles_;
    
public:
    static RealProfiler& getInstance() {
        static RealProfiler instance;
        return instance;
    }
    
    void startTimer(const std::string& name) {
        auto& data = profiles_[name];
        data.start_time = std::chrono::high_resolution_clock::now();
        data.active = true;
    }
    
    void endTimer(const std::string& name) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto& data = profiles_[name];
        
        if (data.active) {
            auto duration = std::chrono::duration<double, std::milli>(end_time - data.start_time).count();
            data.total_time += duration;
            data.call_count++;
            data.min_time = std::min(data.min_time, duration);
            data.max_time = std::max(data.max_time, duration);
            data.active = false;
            
            // Alert on expensive operations (>16ms for 60fps)
            if (duration > 16.0) {
                std::cout << "EXPENSIVE: " << name << " took " << duration << "ms" << std::endl;
            }
        }
    }
    
    void printReport() const {
        std::cout << "\n=== REAL PERFORMANCE PROFILE ===" << std::endl;
        
        // Sort by total time
        std::vector<std::pair<std::string, const ProfileData*>> sorted;
        for (const auto& [name, data] : profiles_) {
            if (data.call_count > 0) {
                sorted.emplace_back(name, &data);
            }
        }
        
        std::sort(sorted.begin(), sorted.end(), 
            [](const auto& a, const auto& b) {
                return a.second->total_time > b.second->total_time;
            });
        
        for (const auto& [name, data] : sorted) {
            double avg = data->total_time / data->call_count;
            std::cout << name << ":" << std::endl;
            std::cout << "  Total: " << data->total_time << "ms" << std::endl;
            std::cout << "  Calls: " << data->call_count << std::endl;
            std::cout << "  Avg: " << avg << "ms" << std::endl;
            std::cout << "  Min: " << data->min_time << "ms" << std::endl; 
            std::cout << "  Max: " << data->max_time << "ms" << std::endl;
            
            if (avg > 5.0) {
                std::cout << "  *** BOTTLENECK DETECTED ***" << std::endl;
            }
            std::cout << std::endl;
        }
    }
    
    void reset() {
        profiles_.clear();
    }
};

// RAII timer class
class ScopedProfiler {
private:
    std::string name_;
public:
    ScopedProfiler(const std::string& name) : name_(name) {
        RealProfiler::getInstance().startTimer(name_);
    }
    
    ~ScopedProfiler() {
        RealProfiler::getInstance().endTimer(name_);
    }
};

#define PROFILE_FUNCTION() ScopedProfiler _prof(__FUNCTION__)
#define PROFILE_SCOPE(name) ScopedProfiler _prof(name)