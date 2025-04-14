#pragma once

#include <string>
#include <atomic>

class Venue final {
public:
    inline explicit Venue(const std::string& name) noexcept
        : name_(name), isActive_(true) {}

    [[nodiscard]] inline const std::string& getName() const noexcept { return name_; }
    [[nodiscard]] inline bool isActive() const noexcept { return isActive_.load(std::memory_order_relaxed); }
    inline void setActive(bool active) noexcept { isActive_.store(active, std::memory_order_relaxed); }

private:
    const std::string name_;
    std::atomic<bool> isActive_;
};
