#pragma once

#include <memory>
#include <vector>
#include "IVenueAdapter.hpp"

namespace TradingSystem {

class IRedundancyManager {
public:
    virtual ~IRedundancyManager() noexcept = default;
    
    // Use shared ownership for venue adapters; if ownership is exclusive consider std::unique_ptr.
    virtual void registerVenueAdapter(std::shared_ptr<IVenueAdapter> adapter) noexcept = 0;
    
    // Broadcast the order to multiple venues with minimal overhead.
    virtual void broadcastOrder(const Order& order) noexcept = 0;
    
    // Cancelling across venues – ensure you have minimal latency here.
    [[nodiscard]] virtual bool cancelOrderAcrossVenues(uint64_t orderId) noexcept = 0;
};

} // namespace TradingSystem
