#pragma once

#include "IExecutionManager.hpp"
#include <functional>

namespace TradingSystem {

class ISimulator {
public:
    virtual ~ISimulator() noexcept = default;
    
    // Instead of std::function (which may incur heap allocations), you can template the callback
    // in performance-critical code. For an interface, we use std::function for clarity.
    virtual void simulateOrderExecution(
        const Order& order, 
        std::function<void(uint64_t, double, double)> callback
    ) noexcept = 0;
    
    // Advances simulation time. For high-performance simulators, consider using a monotonic clock.
    virtual void advanceTime(uint64_t microseconds) noexcept = 0;
};

} // namespace TradingSystem
