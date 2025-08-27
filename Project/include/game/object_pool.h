#pragma once

#include <vector>
#include <memory>
#include <queue>
#include <functional>
#include <algorithm>

template<typename T>
class ObjectPool {
public:
    using ObjectPtr = std::unique_ptr<T>;
    using Factory = std::function<ObjectPtr()>;
    using Resetter = std::function<void(T&)>;

private:
    std::queue<ObjectPtr> available_;
    std::vector<ObjectPtr> active_;
    Factory factory_;
    Resetter resetter_;
    size_t max_size_;

public:
    // Constructor with factory and resetter
    ObjectPool(size_t initial_size, size_t max_size, Factory factory, Resetter resetter = nullptr)
        : factory_(std::move(factory)), resetter_(std::move(resetter)), max_size_(max_size) {
        
        // Pre-allocate objects
        for (size_t i = 0; i < initial_size; ++i) {
            available_.push(factory_());
        }
    }
    
    // Simple constructor for predictive pool manager (objects created on-demand)
    explicit ObjectPool(size_t max_size)
        : max_size_(max_size) {
        // Factory and resetter will be set later if needed
        // Objects created on-demand via acquire()
    }

    // Get an object from the pool
    T* acquire() {
        ObjectPtr obj;
        
        if (!available_.empty()) {
            obj = std::move(available_.front());
            available_.pop();
        } else if (active_.size() < max_size_) {
            if (factory_) {
                obj = factory_();
            } else {
                // For default construction, return nullptr - must provide factory
                return nullptr;
            }
        } else {
            return nullptr; // Pool exhausted
        }

        T* raw_ptr = obj.get();
        active_.push_back(std::move(obj));
        return raw_ptr;
    }

    // Return an object to the pool
    void release(T* obj) {
        auto it = std::find_if(active_.begin(), active_.end(),
            [obj](const ObjectPtr& ptr) { return ptr.get() == obj; });
        
        if (it != active_.end()) {
            ObjectPtr released = std::move(*it);
            active_.erase(it);
            
            // Reset object state if resetter provided
            if (resetter_) {
                resetter_(*released);
            }
            
            available_.push(std::move(released));
        }
    }

    // Get all active objects
    const std::vector<ObjectPtr>& getActive() const { return active_; }
    std::vector<ObjectPtr>& getActive() { return active_; }

    // Remove inactive/dead objects from active list
    template<typename Predicate>
    void removeIf(Predicate predicate) {
        auto it = active_.begin();
        while (it != active_.end()) {
            if (predicate(*it->get())) {
                ObjectPtr obj = std::move(*it);
                it = active_.erase(it);
                
                // Reset and return to pool
                if (resetter_) {
                    resetter_(*obj);
                }
                available_.push(std::move(obj));
            } else {
                ++it;
            }
        }
    }

    // Clear all objects (for game reset)
    void clear() {
        // Move all active objects back to available
        while (!active_.empty()) {
            ObjectPtr obj = std::move(active_.back());
            active_.pop_back();
            
            if (resetter_) {
                resetter_(*obj);
            }
            available_.push(std::move(obj));
        }
    }

    // Statistics
    size_t getActiveCount() const { return active_.size(); }
    size_t getAvailableCount() const { return available_.size(); }
    size_t getTotalCount() const { return active_.size() + available_.size(); }
    size_t getCapacity() const { return max_size_; }
};