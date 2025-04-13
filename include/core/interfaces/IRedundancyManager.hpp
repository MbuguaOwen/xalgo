// IRedundancyManager.hpp
#ifndef IREDUNDANCYMANAGER_HPP
#define IREDUNDANCYMANAGER_HPP

#include <memory>
#include <vector>
#include "IVenueAdapter.hpp"

namespace TradingSystem {

class IRedundancyManager {
public:
    virtual ~IRedundancyManager() = default;
    virtual void registerVenueAdapter(std::shared_ptr<IVenueAdapter> adapter) noexcept = 0;
    virtual void broadcastOrder(const Order& order) noexcept = 0;
    virtual bool cancelOrderAcrossVenues(uint64_t orderId) noexcept = 0;
};

} // namespace TradingSystem

#endif // IREDUNDANCYMANAGER_HPP