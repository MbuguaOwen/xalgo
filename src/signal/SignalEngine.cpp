// SignalEngine.cpp

#include "core/interfaces/ISignalGenerator.hpp"
#include "core/models/MarketData.hpp"
#include "core/models/Signal.hpp"
#include "core/models/MarketRegime.hpp"

#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include <algorithm>
#include <iostream>

namespace XAlgo::Signal {

class SignalEngine {
public:
    explicit SignalEngine(std::vector<ISignalGeneratorPtr> generators)
        : signal_generators_(std::move(generators)) {}

    Signal run(const XAlgo::Data::MarketData& data) {
        std::vector<Signal> raw_signals;

        // Collect signals from each model
        for (const auto& generator : signal_generators_) {
            Signal s = generator->generate(data);
            s.metadata.source_model = generator->name();
            s.timestamp = std::chrono::system_clock::now();
            raw_signals.push_back(s);
        }

        // Aggregate and return final signal
        return aggregate(raw_signals);
    }

    void updateRegime(const MarketRegime& regime) {
        for (const auto& generator : signal_generators_) {
            generator->updateParameters(regime);
        }
    }

private:
    std::vector<ISignalGeneratorPtr> signal_generators_;

    Signal aggregate(const std::vector<Signal>& signals) {
        // Simple majority vote logic (for production, replace with Bayesian ensemble)
        int long_votes = 0, short_votes = 0, hold_votes = 0;

        for (const auto& s : signals) {
            switch (s.direction) {
                case SignalDirection::LONG: long_votes++; break;
                case SignalDirection::SHORT: short_votes++; break;
                case SignalDirection::HOLD: hold_votes++; break;
                default: break;
            }
        }

        Signal final_signal;
        final_signal.timestamp = std::chrono::system_clock::now();
        final_signal.metadata.rationale = "Ensemble Vote Aggregation";

        if (long_votes > short_votes && long_votes > hold_votes) {
            final_signal.direction = SignalDirection::LONG;
        } else if (short_votes > long_votes && short_votes > hold_votes) {
            final_signal.direction = SignalDirection::SHORT;
        } else {
            final_signal.direction = SignalDirection::HOLD;
        }

        final_signal.metadata.confidence_score =
            static_cast<float>(std::max({long_votes, short_votes, hold_votes})) /
            static_cast<float>(signals.size());

        return final_signal;
    }
};

} // namespace XAlgo::Signal
