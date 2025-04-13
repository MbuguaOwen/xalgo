// Venue.hpp
#pragma once

#include <string>
#include <atomic>
#include <mutex>

class Venue {
public:
    explicit Venue(const std::string& name) : name_(name), isActive_(true) {}

    const std::string& getName() const noexcept { return name_; }
    bool isActive() const noexcept { return isActive_.load(std::memory_order_relaxed); }
    void setActive(bool active) noexcept { isActive_.store(active, std::memory_order_relaxed); }

private:
    const std::string name_;
    std::atomic<bool> isActive_;
};
