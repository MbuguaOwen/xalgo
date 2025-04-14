#pragma once

#include <string>
#include <mutex>

class Position final {
public:
    inline explicit Position(const std::string& symbol) noexcept
        : symbol_(symbol), netQuantity_(0.0), avgPrice_(0.0) {}

    // Update position: This multi‐variable update is protected by a mutex.
    inline void update(double quantity, double price) noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        const double prevQty = netQuantity_;
        const double newQty = netQuantity_ + quantity;
        double totalCost = avgPrice_ * prevQty + price * quantity;
        netQuantity_ = newQty;
        avgPrice_ = (newQty != 0.0) ? totalCost / newQty : 0.0;
    }

    // Alternatively, if you accept some eventual consistency, you might explore:
    /*
    inline void update(double quantity, double price) noexcept {
        // This requires that std::atomic<double> is lock-free on your platform.
        // NOTE: Such updates are not strictly atomic (combined update) and might result in minor inconsistencies.
        double currentQty = netQuantity_.load(std::memory_order_relaxed);
        double currentAvg = avgPrice_.load(std::memory_order_relaxed);
        double newQty = currentQty + quantity;
        double totalCost = currentAvg * currentQty + price * quantity;
        netQuantity_.store(newQty, std::memory_order_relaxed);
        avgPrice_.store((newQty != 0.0) ? totalCost / newQty : 0.0, std::memory_order_relaxed);
    }
    */

    [[nodiscard]] inline double getNetQuantity() const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        return netQuantity_;
    }

    [[nodiscard]] inline double getAveragePrice() const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        return avgPrice_;
    }

    [[nodiscard]] inline const std::string& getSymbol() const noexcept { return symbol_; }

private:
    const std::string symbol_;
    mutable std::mutex mutex_;
    double netQuantity_;
    double avgPrice_;
};
