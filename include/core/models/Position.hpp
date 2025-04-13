
// Position.hpp
#pragma once

#include <string>
#include <atomic>
#include <mutex>

class Position {
public:
    explicit Position(const std::string& symbol)
        : symbol_(symbol), netQuantity_(0.0), avgPrice_(0.0) {}

    void update(double quantity, double price) {
        std::lock_guard<std::mutex> lock(mutex_);
        double totalCost = avgPrice_ * netQuantity_ + price * quantity;
        netQuantity_ += quantity;
        if (netQuantity_ != 0.0) {
            avgPrice_ = totalCost / netQuantity_;
        } else {
            avgPrice_ = 0.0;
        }
    }

    double getNetQuantity() const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        return netQuantity_;
    }

    double getAveragePrice() const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        return avgPrice_;
    }

    const std::string& getSymbol() const noexcept { return symbol_; }

private:
    const std::string symbol_;
    mutable std::mutex mutex_;
    double netQuantity_;
    double avgPrice_;
};
