// MarketData.hpp
#pragma once

#include <string>
#include <chrono>
#include <vector>

namespace XAlgo::Data {

struct PriceLevel {
    double price = 0.0;
    double volume = 0.0;
};

struct OrderBookSnapshot {
    std::vector<PriceLevel> bids;
    std::vector<PriceLevel> asks;
};

struct MarketData {
    std::string symbol; // e.g., "GBP/USD"
    double last_price = 0.0;
    double mid_price = 0.0;
    double bid_price = 0.0;
    double ask_price = 0.0;
    double spread = 0.0;
    double volume = 0.0;

    OrderBookSnapshot book;
    std::chrono::system_clock::time_point timestamp;
};

} // namespace XAlgo::Data
