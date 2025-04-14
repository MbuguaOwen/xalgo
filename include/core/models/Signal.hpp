// Signal.hpp
#pragma once

#include <string>
#include <chrono>

namespace XAlgo::Signal {

enum class SignalDirection {
    LONG,
    SHORT,
    HOLD,
    NONE  // Failsafe state
};

struct SignalMetadata {
    float confidence_score = 0.0f;
    std::string source_model;
    std::string rationale; // e.g. "mean reversion triggered", "Kalman pair anomaly"
    std::string regime_context; // e.g. "low volatility", "mean reversion"
};

struct Signal {
    SignalDirection direction = SignalDirection::NONE;
    SignalMetadata metadata;
    std::chrono::system_clock::time_point timestamp;
};

} // namespace XAlgo::Signal
