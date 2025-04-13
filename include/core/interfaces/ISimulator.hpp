// ISimulator.hpp
#ifndef ISIMULATOR_HPP
#define ISIMULATOR_HPP

#include "IExecutionManager.hpp"
#include <functional>

namespace TradingSystem {

class ISimulator {
public:
    virtual ~ISimulator() = default;
    virtual void simulateOrderExecution(const Order& order, std::function<void(uint64_t, double, double)> callback) noexcept = 0;
    virtual void advanceTime(uint64_t microseconds) noexcept = 0;
};

} // namespace TradingSystem

#endif // ISIMULATOR_HPP
