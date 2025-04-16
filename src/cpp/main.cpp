#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <cstdlib>      // For EXIT_FAILURE and EXIT_SUCCESS
#include "SignalEngine.hpp"
#include "ExecutionManager.hpp"
#include "RiskManager.hpp"
#include "TradeLeg.hpp"
#include "Order.hpp"

// For cointegration test using Eigen vectors.
#include <Eigen/Dense>

namespace TradingSystem {
    // Simulated IOrderRouter stub for compatibility; production would be richer.
    struct IOrderRouter {
        virtual ~IOrderRouter() = default;
    };
}

using namespace TradingSystem;

int main() {
    // --------------------------------------------------------
    // Setup Infrastructure Layer & Order Processing Interface
    // --------------------------------------------------------
    // Create a simple order queue (in production, this might be a high-performance lock-free queue)
    OrderQueue orderQueue;
    
    // Instantiate Execution Manager with order queue for individual orders.
    ExecutionManager execManager(orderQueue);
    
    // --------------------------------------------------------
    // Setup Signal Intelligence & Backtesting Layer
    // --------------------------------------------------------
    SignalEngine signalEngine;
    JohansenTestEngine johansenEngine;
    
    // Simulate a series of tick data for backtesting (in a real system, this data would be live)
    std::vector<TickData> tickSeries;
    for (int i = 0; i < 1000; ++i) {
        TickData tick {
            1.1200 + i * 1e-5,  // eurUsd
            1.3100 + i * 1e-5,  // gbpUsd
            0.8600 + i * 1e-5,  // eurGbp
            std::chrono::steady_clock::now() // timestamp
        };
        tickSeries.push_back(tick);
    }
    
    // Compute spreads and the adaptive Z-score over the sliding window
    std::vector<double> spreadHistory;
    for (const auto &tick : tickSeries) {
        double spread = signalEngine.computeSpread(tick);
        spreadHistory.push_back(spread);
    }
    double zScore = signalEngine.computeAdaptiveZScore(spreadHistory);
    std::cout << "Adaptive Z-Score: " << zScore << "\n";
    
    // --------------------------------------------------------
    // Run Johansen Cointegration Test for Signal Validation
    // --------------------------------------------------------
    // Create three time series from the tick data using Eigen vectors.
    Eigen::VectorXd eurUsd(tickSeries.size()), gbpUsd(tickSeries.size()), eurGbp(tickSeries.size());
    for (size_t i = 0; i < tickSeries.size(); ++i) {
        eurUsd(i) = tickSeries[i].eurUsd;
        gbpUsd(i) = tickSeries[i].gbpUsd;
        eurGbp(i) = tickSeries[i].eurGbp;
    }
    std::vector<Eigen::VectorXd> timeSeries = { eurUsd, gbpUsd, eurGbp };
    bool cointegrated = johansenEngine.runTest(timeSeries);
    std::cout << "Cointegration detected: " << (cointegrated ? "Yes" : "No") << "\n";
    
    // --------------------------------------------------------
    // Setup Risk Management Layer
    // --------------------------------------------------------
    // Assume an allocated capital of $100M for risk evaluation.
    RiskManager riskManager(100e6);
    double simulatedOrderSize = 2e6;
    double volatilityFactor = 1.2;  // Simulated volatility impact
    if (!riskManager.evaluateOrderRisk(simulatedOrderSize, volatilityFactor)) {
        std::cerr << "Order rejected due to risk limits.\n";
        return EXIT_FAILURE;
    }
    
    // --------------------------------------------------------
    // Execution & Benchmarking of Single-Leg Orders
    // --------------------------------------------------------
    const int numOrders = 10000;
    auto startTime = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numOrders; ++i) {
        // Construct an order; here we assume Order supports a constructor with:
        // (order_id, side, quantity, price)
        Order order { static_cast<uint64_t>(i), Order::BUY, 100000, 1.1234 };
        execManager.executeTrade(order);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> execDuration = endTime - startTime;
    std::cout << "Average order submission latency: " 
              << (execDuration.count() / numOrders) << " µs\n";

    // --------------------------------------------------------
    // Monitoring & Error Recovery Simulation
    // --------------------------------------------------------
    std::thread monitor([&riskManager]() {
        while (true) {
            if (!riskManager.isStrategyAllowed()) {
                std::cerr << "Strategy disabled due to excessive drawdown. Alerting ops...\n";
                break;  // In production, trigger an automated recovery or alert procedure.
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    
    // Let the monitoring thread run briefly for simulation.
    std::this_thread::sleep_for(std::chrono::seconds(3));
    monitor.detach();
    
    // --------------------------------------------------------
    // Order Information & Multi–Leg Trade Execution (from Code B)
    // --------------------------------------------------------
    // Create and display a single sample order.
    Order sampleOrder(1001, "EUR/USD", 1.1234, 1000000, OrderSide::BUY, OrderType::MARKET);
    std::cout << "[Order Info] ID: " << sampleOrder.getId() 
              << ", Symbol: " << sampleOrder.getSymbol()
              << ", Side: " << (sampleOrder.getSide() == OrderSide::BUY ? "BUY" : "SELL")
              << ", Price: " << sampleOrder.getPrice()
              << ", Qty: " << sampleOrder.getQuantity() << std::endl;
    
    // Setup multi-leg trade parameters using TradeLegs.
    TradeLeg leg1("EUR/USD", 1.1234, 1000000, "buy");
    TradeLeg leg2("USD/GBP", 0.7890, 1000000, "sell");
    TradeLeg leg3("GBP/EUR", 1.4210, 1000000, "buy");
    
    // For multi-leg execution, we create a separate instance of ExecutionManager.
    // Depending on your design, this may reuse the same infrastructure or use a different instance.
    ExecutionManager multiLegManager; // Assuming an overload with no parameters exists.
    multiLegManager.setLegs(leg1, leg2, leg3);
    multiLegManager.execute();
    
    if (multiLegManager.getState() != TradeState::COMPLETE) {
        std::cerr << "Multi-leg trade execution did not complete successfully." << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Multi-leg trade executed successfully." << std::endl;
    
    // --------------------------------------------------------
    // Clean Shutdown
    // --------------------------------------------------------
    std::cout << "System shutting down cleanly..." << std::endl;
    return EXIT_SUCCESS;
}
