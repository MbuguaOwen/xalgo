// =====================[ System Utilities ]===================== //
#include <iostream>          // Basic I/O (for logging/fallback)
#include <thread>            // For concurrency, thread pinning
#include <chrono>            // Timing benchmarks
#include <atomic>            // Lock-free flags and sync
#include <vector>            // Core container
#include <memory>            // Smart pointers
#include <string>            // For config paths & symbols
#include <cstdlib>           // For EXIT_SUCCESS / EXIT_FAILURE

// =====================[ Configuration & Logging ]===================== //
#include "utils/ConfigLoader.hpp"
#include "utils/Logger.hpp"
#include "utils/Metrics.hpp"

// =====================[ Data Models ]===================== //
#include "core/models/Order.hpp"
#include "core/models/Venue.hpp"
#include "core/models/Trade.hpp"
#include "core/models/MarketData.hpp"
#include "core/models/MarketRegime.hpp"
#include "core/models/Signal.hpp"
#include "core/models/Position.hpp"
#include "core/TradeLeg.hpp"

// =====================[ Interfaces (Inversion Layer)]===================== //
#include "core/interfaces/ISignalGenerator.hpp"
#include "core/interfaces/IFeatureExtractor.hpp"
#include "core/interfaces/IOrderRouter.hpp"
#include "core/interfaces/IVenueAdapter.hpp"
#include "core/interfaces/IExecutionManager.hpp"
#include "core/interfaces/IRedundancyManager.hpp"
#include "core/interfaces/ISimulator.hpp"

// =====================[ Signal Intelligence Layer ]===================== //
#include "core/SignalEngine.hpp"

// =====================[ Execution Layer ]===================== //
#include "core/execution/ExecutionManager.hpp"
#include "core/execution/ExecutionState.hpp"
#include "core/router/SmartOrderRouter.hpp"

// =====================[ Risk Management Layer ]===================== //
#include "core/Risk/RiskManager.hpp"

// =====================[ Infrastructure Layer ]===================== //
#include "core/messaging/ZMQPubSubHandler.hpp"
#include "core/interfaces/ZeroMQConnectionManager.hpp"
#include "core/interfaces/IHeartbeatMonitor.hpp"
#include "core/interfaces/ICircuitBreaker.hpp"

// =====================[ External Libraries ]===================== //
#include <Eigen/Dense> // For Johansen test and matrix operations


namespace TradingSystem {
    // Simulated stub; real version would be injected or fully implemented.
    struct IOrderRouter {
        virtual ~IOrderRouter() = default;
    };
}

using namespace TradingSystem;

int main() {
    // --------------------------------------------------------
    // Infrastructure Setup
    // --------------------------------------------------------
    OrderQueue orderQueue;
    ExecutionManager execManager(orderQueue);

    // --------------------------------------------------------
    // Signal & Statistical Edge Computation
    // --------------------------------------------------------
    SignalEngine signalEngine;
    JohansenTestEngine johansenEngine;

    std::vector<TickData> tickSeries;
    for (int i = 0; i < 1000; ++i) {
        TickData tick {
            1.1200 + i * 1e-5,
            1.3100 + i * 1e-5,
            0.8600 + i * 1e-5,
            std::chrono::steady_clock::now()
        };
        tickSeries.push_back(tick);
    }

    std::vector<double> spreadHistory;
    for (const auto& tick : tickSeries) {
        spreadHistory.push_back(signalEngine.computeSpread(tick));
    }

    double zScore = signalEngine.computeAdaptiveZScore(spreadHistory);
    std::cout << "Adaptive Z-Score: " << zScore << "\n";

    // --------------------------------------------------------
    // Cointegration Test
    // --------------------------------------------------------
    Eigen::VectorXd eurUsd(tickSeries.size()), gbpUsd(tickSeries.size()), eurGbp(tickSeries.size());
    for (size_t i = 0; i < tickSeries.size(); ++i) {
        eurUsd(i) = tickSeries[i].eurUsd;
        gbpUsd(i) = tickSeries[i].gbpUsd;
        eurGbp(i) = tickSeries[i].eurGbp;
    }
    bool cointegrated = johansenEngine.runTest({eurUsd, gbpUsd, eurGbp});
    std::cout << "Cointegration detected: " << (cointegrated ? "Yes" : "No") << "\n";

    // --------------------------------------------------------
    // Risk Management
    // --------------------------------------------------------
    RiskManager riskManager(100e6);
    if (!riskManager.evaluateOrderRisk(2e6, 1.2)) {
        std::cerr << "Order rejected due to risk limits.\n";
        return EXIT_FAILURE;
    }

    // --------------------------------------------------------
    // Order Execution Benchmark
    // --------------------------------------------------------
    const int numOrders = 10000;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numOrders; ++i) {
        Order order(static_cast<uint64_t>(i), Order::BUY, 100000, 1.1234);
        execManager.executeTrade(order);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto latency = std::chrono::duration<double, std::micro>(end - start).count();
    std::cout << "Avg order submission latency: " << (latency / numOrders) << " µs\n";

    // --------------------------------------------------------
    // Health Monitoring Simulation
    // --------------------------------------------------------
    std::thread monitor([&riskManager]() {
        while (true) {
            if (!riskManager.isStrategyAllowed()) {
                std::cerr << "Strategy disabled due to excessive drawdown. Alerting ops...\n";
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(3));
    monitor.detach();

    // --------------------------------------------------------
    // Multi-Leg Trade Execution
    // --------------------------------------------------------
    Order sampleOrder(1001, "EUR/USD", 1.1234, 1'000'000, OrderSide::BUY, OrderType::MARKET);
    std::cout << "[Order Info] ID: " << sampleOrder.getId()
              << ", Symbol: " << sampleOrder.getSymbol()
              << ", Side: " << (sampleOrder.getSide() == OrderSide::BUY ? "BUY" : "SELL")
              << ", Price: " << sampleOrder.getPrice()
              << ", Qty: " << sampleOrder.getQuantity() << "\n";

    TradeLeg leg1("EUR/USD", 1.1234, 1'000'000, "buy");
    TradeLeg leg2("USD/GBP", 0.7890, 1'000'000, "sell");
    TradeLeg leg3("GBP/EUR", 1.4210, 1'000'000, "buy");

    ExecutionManager multiLegManager;
    multiLegManager.setLegs(leg1, leg2, leg3);
    multiLegManager.execute();

    if (multiLegManager.getState() != TradeState::COMPLETE) {
        std::cerr << "Multi-leg trade execution failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "Multi-leg trade executed successfully.\n";
    std::cout << "System shutting down cleanly...\n";
    return EXIT_SUCCESS;
}
