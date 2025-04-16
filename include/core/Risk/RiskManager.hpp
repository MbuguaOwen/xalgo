#pragma once

#include <atomic>
#include <mutex>
#include <iostream>
#include <cmath>
#include <chrono>

namespace TradingSystem {

/// @brief RiskManager monitors positions and ensures risk controls.
class RiskManager {
public:
    RiskManager(double capital)
        : allocatedCapital_(capital), currentDrawdown_(0.0), isStrategyActive_(true) {}

    /// @brief Pre-trade risk control: evaluates if a new order is permissible.
    bool evaluateOrderRisk(double potentialOrderSize, double volatilityFactor) {
        std::lock_guard<std::mutex> lock(riskMutex_);
        // Example condition: no order should exceed 5% of allocated capital when volatility is high.
        if (potentialOrderSize > allocatedCapital_ * 0.05 * volatilityFactor) {
            std::cerr << "RiskManager: Order size exceeds risk limits.\n";
            return false;
        }
        return true;
    }

    /// @brief Real-time risk engine: updates positions with fill executions.
    void updatePosition(double pnlChange) {
        std::lock_guard<std::mutex> lock(riskMutex_);
        currentPnL_ += pnlChange;
        updateDrawdown();
    }
    
    /// @brief Checks if strategy should be disabled due to drawdown.
    bool isStrategyAllowed() {
        std::lock_guard<std::mutex> lock(riskMutex_);
        if (currentDrawdown_ > allocatedCapital_ * 0.03) {  // 3% max drawdown
            isStrategyActive_ = false;
        }
        return isStrategyActive_;
    }

private:
    /// @brief Updates maximum drawdown based on current PnL.
    void updateDrawdown() {
        // For demonstration, we assume currentPnL_ is negative when drawing down.
        if (currentPnL_ < minPnL_) {
            minPnL_ = currentPnL_;
            currentDrawdown_ = allocatedCapital_ - (allocatedCapital_ + minPnL_);
            std::cerr << "RiskManager: Updated drawdown: " << currentDrawdown_ << "\n";
        }
    }

    double allocatedCapital_;
    double currentPnL_ {0.0};
    double minPnL_ {0.0};
    double currentDrawdown_;
    bool isStrategyActive_;
    std::mutex riskMutex_;
};

} // namespace TradingSystem
