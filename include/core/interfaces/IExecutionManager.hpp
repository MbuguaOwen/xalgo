#pragma once

#include "IOrderRouter.hpp"
#include <cstdint>

namespace TradingSystem {

class IExecutionManager {
public:
    virtual ~IExecutionManager() noexcept = default;
    
    // Execute trade should quickly interface with downstream systems.
    virtual void executeTrade(const Order& order) noexcept = 0;
    
    // Execution reports must be handled swiftly; consider using lock‑free queues for real systems.
    virtual void onExecutionReport(uint64_t orderId, double fillPrice, double fillQty) noexcept = 0;
};

} // namespace TradingSystem
