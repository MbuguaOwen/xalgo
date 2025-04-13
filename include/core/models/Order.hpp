// Order.hpp
#pragma once

#include <string>
#include <atomic>
#include <cstdint>
#include <chrono>

enum class OrderSide { BUY, SELL };
enum class OrderType { MARKET, LIMIT };

class Order {
public:
    Order(uint64_t id, const std::string& symbol, double price, double quantity,
          OrderSide side, OrderType type)
        : id_(id), symbol_(symbol), price_(price), quantity_(quantity),
          side_(side), type_(type), timestamp_(std::chrono::high_resolution_clock::now()) {}

    uint64_t getId() const noexcept { return id_; }
    const std::string& getSymbol() const noexcept { return symbol_; }
    double getPrice() const noexcept { return price_; }
    double getQuantity() const noexcept { return quantity_; }
    OrderSide getSide() const noexcept { return side_; }
    OrderType getType() const noexcept { return type_; }
    std::chrono::high_resolution_clock::time_point getTimestamp() const noexcept { return timestamp_; }

private:
    const uint64_t id_;
    const std::string symbol_;
    const double price_;
    const double quantity_;
    const OrderSide side_;
    const OrderType type_;
    const std::chrono::high_resolution_clock::time_point timestamp_;
};

