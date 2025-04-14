#pragma once

#include <string>
#include <cstdint>
#include <chrono>

enum class OrderSide { BUY, SELL };
enum class OrderType { MARKET, LIMIT };

class Order final {
public:
    // Constructor: mark inline since it is trivial.
    inline Order(uint64_t id, const std::string& symbol, double price, double quantity,
                 OrderSide side, OrderType type) noexcept
        : id_(id), symbol_(symbol), price_(price), quantity_(quantity),
          side_(side), type_(type),
          timestamp_(std::chrono::high_resolution_clock::now()) {}

    [[nodiscard]] inline uint64_t getId() const noexcept { return id_; }
    [[nodiscard]] inline const std::string& getSymbol() const noexcept { return symbol_; }
    [[nodiscard]] inline double getPrice() const noexcept { return price_; }
    [[nodiscard]] inline double getQuantity() const noexcept { return quantity_; }
    [[nodiscard]] inline OrderSide getSide() const noexcept { return side_; }
    [[nodiscard]] inline OrderType getType() const noexcept { return type_; }
    [[nodiscard]] inline std::chrono::high_resolution_clock::time_point getTimestamp() const noexcept { return timestamp_; }

private:
    const uint64_t id_;
    const std::string symbol_;
    const double price_;
    const double quantity_;
    const OrderSide side_;
    const OrderType type_;
    const std::chrono::high_resolution_clock::time_point timestamp_;
};
