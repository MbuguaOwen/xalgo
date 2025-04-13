// IOrderRouter.hpp
#ifndef IORDERROUTER_HPP
#define IORDERROUTER_HPP

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace TradingSystem {

struct Order {
    std::string symbol;
    double price;
    double quantity;
    bool isBuy;
    uint64_t timestamp;
};

class IOrderRouter {
public:
    virtual ~IOrderRouter() = default;
    virtual void routeOrder(const Order& order) noexcept = 0;
    virtual bool cancelOrder(uint64_t orderId) noexcept = 0;
    virtual void flush() noexcept = 0;
};

} // namespace TradingSystem

#endif // IORDERROUTER_HPP