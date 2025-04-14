// ISignalGenerator.hpp
#pragma once

#include <memory>
#include "models/MarketData.hpp"
#include "models/Signal.hpp"
#include "models/MarketRegime.hpp"

namespace XAlgo::Signal {

class ISignalGenerator {
public:
    virtual ~ISignalGenerator() = default;

    // Generate trading signal from current market state
    virtual Signal generate(const MarketData& data) noexcept = 0;

    // Update signal generator parameters based on current regime
    virtual void updateParameters(const MarketRegime& regime) noexcept = 0;

    // Optional: For model-specific introspection or monitoring
    virtual const std::string& name() const noexcept = 0;
};

using ISignalGeneratorPtr = std::shared_ptr<ISignalGenerator>;

} // namespace XAlgo::Signal
