// MarketRegime.hpp
#pragma once

#include <string>

namespace XAlgo::Signal {

enum class RegimeType {
    VOLATILE,
    CALM,
    TRENDING,
    MEAN_REVERTING,
    UNKNOWN
};

struct MarketRegime {
    RegimeType type = RegimeType::UNKNOWN;
    float volatility_score = 0.0f;
    float trend_strength = 0.0f;
    std::string derived_from_model; // e.g. "HMM", "BayesianSwitch"
};

} // namespace XAlgo::Signal
