#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <map>
#include <queue>
#include <future>
#include <chrono>
#include <stdexcept>
#include <algorithm>

struct Order {
    std::string symbol;
    double price;
    double quantity;
    std::string side; // "buy" or "sell"
};

struct Venue {
    std::string name;
    double latency;
    double reliability;
    std::atomic<bool> available;

    Venue(std::string n, double l, double r)
        : name(n), latency(l), reliability(r), available(true) {}
};

class SmartOrderRouter {
private:
    std::vector<Venue> venues;
    std::mutex venueMutex;

public:
    SmartOrderRouter(const std::vector<Venue>& v) : venues(v) {}

    void rankVenues() {
        std::sort(venues.begin(), venues.end(), [](const Venue& a, const Venue& b) {
            return (a.latency * (1.0 / a.reliability)) < (b.latency * (1.0 / b.reliability));
        });
    }

    void sendOrderAsync(const Order& order) {
        std::vector<std::future<void>> futures;
        for (auto& venue : venues) {
            if (!venue.available.load()) continue;
            futures.push_back(std::async(std::launch::async, [&order, &venue]() {
                try {
                    std::this_thread::sleep_for(std::chrono::microseconds((int)venue.latency));
                    std::cout << "Order sent to venue: " << venue.name << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Order failed at venue " << venue.name << ": " << e.what() << std::endl;
                }
            }));
        }
        for (auto& f : futures) f.get();
    }

    void routeOrder(const Order& order) {
        rankVenues();
        sendOrderAsync(order);
    }
};

int main() {
    std::vector<Venue> venues = {
        Venue("VenueA", 50.0, 0.99),
        Venue("VenueB", 30.0, 0.97),
        Venue("VenueC", 70.0, 0.995)
    };

    SmartOrderRouter router(venues);
    Order order = {"EUR/USD", 1.1234, 1000000, "buy"};
    router.routeOrder(order);

    return 0;
}