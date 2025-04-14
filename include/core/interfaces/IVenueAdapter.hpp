#pragma once

#include "IOrderRouter.hpp"
#include <string>

namespace TradingSystem {

class IVenueAdapter {
public:
    virtual ~IVenueAdapter() noexcept = default;
    
    // Non‑blocking order send for low latency
    virtual void sendOrder(const Order& order) noexcept = 0;
    
    [[nodiscard]] virtual bool cancelOrder(uint64_t orderId) noexcept = 0;
    
    // Acknowledgement callback ideally should be minimal overhead and asynchronous.
    virtual void onOrderAcknowledgement(uint64_t orderId) noexcept = 0;
    
    // Returns the venue name – consider caching this value if it’s used frequently.
    virtual std::string venueName() const noexcept = 0;
};

} // namespace TradingSystem
