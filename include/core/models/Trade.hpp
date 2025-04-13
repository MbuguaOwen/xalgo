// Trade.hpp
#pragma once

#include "Order.hpp"
#include "Venue.hpp"
#include <chrono>

class Trade {
public:
    Trade(const Order& order, const Venue& venue, double fillPrice, double fillQuantity)
        : orderId_(order.getId()),
          venue_(venue.getName()),
          fillPrice_(fillPrice),
          fillQuantity_(fillQuantity),
          side_(order.getSide()),
          timestamp_(std::chrono::high_resolution_clock::now()) {}

    uint64_t getOrderId() const noexcept { return orderId_; }
    const std::string& getVenue() const noexcept { return venue_; }
    double getFillPrice() const noexcept { return fillPrice_; }
    double getFillQuantity() const noexcept { return fillQuantity_; }
    OrderSide getSide() const noexcept { return side_; }
    std::chrono::high_resolution_clock::time_point getTimestamp() const noexcept { return timestamp_; }

private:
    const uint64_t orderId_;
    const std::string venue_;
    const double fillPrice_;
    const double fillQuantity_;
    const OrderSide side_;
    const std::chrono::high_resolution_clock::time_point timestamp_;
};
