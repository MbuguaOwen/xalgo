#pragma once

#include "Order.hpp"
#include "Venue.hpp"
#include <chrono>
#include <string>

class Trade final {
public:
    inline Trade(const Order& order, const Venue& venue, double fillPrice, double fillQuantity) noexcept
        : orderId_(order.getId()),
          venue_(venue.getName()),
          fillPrice_(fillPrice),
          fillQuantity_(fillQuantity),
          side_(order.getSide()),
          timestamp_(std::chrono::high_resolution_clock::now()) {}

    [[nodiscard]] inline uint64_t getOrderId() const noexcept { return orderId_; }
    [[nodiscard]] inline const std::string& getVenue() const noexcept { return venue_; }
    [[nodiscard]] inline double getFillPrice() const noexcept { return fillPrice_; }
    [[nodiscard]] inline double getFillQuantity() const noexcept { return fillQuantity_; }
    [[nodiscard]] inline OrderSide getSide() const noexcept { return side_; }
    [[nodiscard]] inline std::chrono::high_resolution_clock::time_point getTimestamp() const noexcept { return timestamp_; }

private:
    const uint64_t orderId_;
    const std::string venue_;
    const double fillPrice_;
    const double fillQuantity_;
    const OrderSide side_;
    const std::chrono::high_resolution_clock::time_point timestamp_;
};

#pragma once

#include <string>

struct TradeLeg {
    std::string symbol;
    double price;
    double quantity;
    std::string side; // "buy" or "sell"

    TradeLeg(const std::string& sym = "", double p = 0.0, double qty = 0.0, const std::string& s = "")
        : symbol(sym), price(p), quantity(qty), side(s) {}
};
