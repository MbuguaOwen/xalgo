// IExecutionManager.hpp
#ifndef IEXECUTIONMANAGER_HPP
#define IEXECUTIONMANAGER_HPP

#include "IOrderRouter.hpp"
#include <cstdint>

namespace TradingSystem {

class IExecutionManager {
public:
    virtual ~IExecutionManager() = default;
    virtual void executeTrade(const Order& order) noexcept = 0;
    virtual void onExecutionReport(uint64_t orderId, double fillPrice, double fillQty) noexcept = 0;
};

} // namespace TradingSystem

#endif // IEXECUTIONMANAGER_HPP