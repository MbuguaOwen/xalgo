// IVenueAdapter.hpp
#ifndef IVENUEADAPTER_HPP
#define IVENUEADAPTER_HPP

#include "IOrderRouter.hpp"
#include <string>

namespace TradingSystem {

class IVenueAdapter {
public:
    virtual ~IVenueAdapter() = default;
    virtual void sendOrder(const Order& order) noexcept = 0;
    virtual bool cancelOrder(uint64_t orderId) noexcept = 0;
    virtual void onOrderAcknowledgement(uint64_t orderId) noexcept = 0;
    virtual std::string venueName() const noexcept = 0;
};

} // namespace TradingSystem

#endif // IVENUEADAPTER_HPP