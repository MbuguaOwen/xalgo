#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <atomic>
#include <thread>
#include <chrono>
#include <stdexcept>

enum class TradeState {
    INIT,
    LEG1_SENT,
    LEG2_SENT,
    LEG3_SENT,
    COMPLETE,
    ERROR
};

struct TradeLeg {
    std::string symbol;
    double price;
    double quantity;
    std::string side; // "buy" or "sell"
};

class ExecutionManager {
private:
    std::mutex stateMutex;
    std::atomic<TradeState> state;
    std::unordered_map<std::string, TradeLeg> legs;

    void sendLeg(const TradeLeg& leg) {
        std::this_thread::sleep_for(std::chrono::microseconds(10)); // Simulate execution latency
        std::cout << "Executed " << leg.side << " on " << leg.symbol << " at " << leg.price << " for " << leg.quantity << std::endl;
    }

public:
    ExecutionManager() : state(TradeState::INIT) {}

    void setLegs(const TradeLeg& leg1, const TradeLeg& leg2, const TradeLeg& leg3) {
        std::lock_guard<std::mutex> lock(stateMutex);
        legs["leg1"] = leg1;
        legs["leg2"] = leg2;
        legs["leg3"] = leg3;
    }

    void execute() {
        try {
            state = TradeState::LEG1_SENT;
            sendLeg(legs.at("leg1"));

            state = TradeState::LEG2_SENT;
            sendLeg(legs.at("leg2"));

            state = TradeState::LEG3_SENT;
            sendLeg(legs.at("leg3"));

            state = TradeState::COMPLETE;
            std::cout << "Triangular arbitrage complete." << std::endl;
        } catch (const std::exception& e) {
            state = TradeState::ERROR;
            std::cerr << "Execution error: " << e.what() << std::endl;
        }
    }
};

int main() {
    TradeLeg leg1 = {"EUR/USD", 1.1234, 1000000, "buy"};
    TradeLeg leg2 = {"USD/GBP", 0.7890, 1000000, "sell"};
    TradeLeg leg3 = {"GBP/EUR", 1.4210, 1000000, "buy"};

    ExecutionManager manager;
    manager.setLegs(leg1, leg2, leg3);
    manager.execute();

    return 0;
}
