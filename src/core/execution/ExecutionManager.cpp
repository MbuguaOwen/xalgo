// ExecutionManager.cpp
#include <iostream>
#include <array>
#include <atomic>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <exception>

// Define the possible states for the triangular arbitrage trade
enum class TradeState : uint8_t {
    INIT,
    LEG1_SENT,
    LEG2_SENT,
    LEG3_SENT,
    COMPLETE,
    ERROR
};

// Trade leg details: Using a fixed-size container avoids dynamic allocations.
struct TradeLeg {
    std::string symbol;
    double price;
    double quantity;
    std::string side; // "buy" or "sell"

    // Constructor for convenience
    TradeLeg(const std::string& sym = "", double p = 0.0, double qty = 0.0, const std::string& s = "")
        : symbol(sym), price(p), quantity(qty), side(s) {}
};

class ExecutionManager {
public:
    // Constructor: state initialized to INIT and legs are default-initialized.
    ExecutionManager() : state_(TradeState::INIT) {}

    // Disable copy and move semantics for thread safety
    ExecutionManager(const ExecutionManager&) = delete;
    ExecutionManager& operator=(const ExecutionManager&) = delete;

    // Set the three legs of the triangular trade. Must be called prior to execution.
    void setLegs(const TradeLeg& leg1, const TradeLeg& leg2, const TradeLeg& leg3) {
        // Since legs are only written once and then read-only, no lock is required.
        legs_[0] = leg1;
        legs_[1] = leg2;
        legs_[2] = leg3;
    }

    // Execute the trade using an atomic state machine.
    void execute() noexcept {
        try {
            // Execute leg 1
            updateState(TradeState::LEG1_SENT);
            sendLeg(legs_[0]);

            // Execute leg 2
            updateState(TradeState::LEG2_SENT);
            sendLeg(legs_[1]);

            // Execute leg 3
            updateState(TradeState::LEG3_SENT);
            sendLeg(legs_[2]);

            updateState(TradeState::COMPLETE);
            std::cout << "Triangular arbitrage complete." << std::endl;
        }
        catch (const std::exception& ex) {
            updateState(TradeState::ERROR);
            std::cerr << "Execution error: " << ex.what() << std::endl;
        }
        catch (...) {
            updateState(TradeState::ERROR);
            std::cerr << "Unknown execution error." << std::endl;
        }
    }

    // Retrieve the current state for monitoring
    TradeState getState() const noexcept {
        return state_.load(std::memory_order_acquire);
    }

private:
    // Use an std::array for fixed-size, contiguous storage of legs.
    std::array<TradeLeg, 3> legs_;

    // Atomic trade state for low-latency, lock-free state transitions.
    std::atomic<TradeState> state_;

    // Update the state atomically with relaxed ordering where applicable.
    void updateState(TradeState newState) noexcept {
        state_.store(newState, std::memory_order_release);
    }

    // Simulated trade execution function. For production, replace with actual market connectivity.
    inline void sendLeg(const TradeLeg& leg) {
        // Simulate execution latency of 10 microseconds; use steady_clock for low overhead.
        auto start = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::microseconds(10));

        // In a production system, detailed error checking and confirmation receipt would be implemented.
        if (leg.quantity <= 0.0 || leg.price <= 0.0) {
            throw std::runtime_error("Invalid trade leg parameters");
        }

        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::micro> execTime = end - start;
        if (execTime.count() > 100.0) {
            // Log a warning if latency exceeds the 100μs target.
            std::cerr << "Warning: Leg execution latency " << execTime.count() << "μs exceeds threshold." << std::endl;
        }

        // Output execution message with minimal overhead.
        std::cout << "Executed " << leg.side << " on " << leg.symbol 
                  << " at " << leg.price << " for " << leg.quantity << " (Latency: "
                  << execTime.count() << "μs)" << std::endl;
    }
};

int main() {
    // Construct trade legs. In production, these values would be dynamically determined.
    TradeLeg leg1("EUR/USD", 1.1234, 1000000, "buy");
    TradeLeg leg2("USD/GBP", 0.7890, 1000000, "sell");
    TradeLeg leg3("GBP/EUR", 1.4210, 1000000, "buy");

    // Instantiate the execution manager
    ExecutionManager manager;

    // Set the trade legs (immutable after this point for thread safety)
    manager.setLegs(leg1, leg2, leg3);

    // Execute the trade orchestrating all legs in sequence.
    manager.execute();

    // Optional: Check final state
    TradeState finalState = manager.getState();
    if (finalState != TradeState::COMPLETE) {
        std::cerr << "Trade execution did not complete successfully." << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
