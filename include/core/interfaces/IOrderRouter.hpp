#pragma once

#include <string>
// Use a fixed‑width string type or fixed‑size char array for symbol fields in ultra‑low latency contexts.
// For now we use std::string for ease of use.
#include <vector>
#include <memory>
#include <cstdint>

namespace TradingSystem {

// Include an order identifier inside Order for consistency across the system.
// In a production system, you might want to use a union or a fixed‑buffer for the symbol.
struct Order {
    uint64_t orderId;      // Unique identifier for the order
    std::string symbol;    // Could be replaced with a fixed-length string or string_view with careful lifetime management
    double price;
    double quantity;
    bool isBuy;
    uint64_t timestamp;
};

class IOrderRouter {
public:
    virtual ~IOrderRouter() noexcept = default;
    
    // Route order should be lock‑free or minimal overhead and non‑blocking.
    virtual void routeOrder(const Order& order) noexcept = 0;
    
    // Using [[nodiscard]] to help ensure that cancellation failures aren’t silently ignored.
    [[nodiscard]] virtual bool cancelOrder(uint64_t orderId) noexcept = 0;
    
    // flush might be used to push out any buffered messages; ensure it’s non‑blocking.
    virtual void flush() noexcept = 0;
};

} // namespace TradingSystem
