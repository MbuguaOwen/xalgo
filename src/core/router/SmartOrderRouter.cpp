#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <future>
#include <chrono>
#include <stdexcept>
#include <algorithm>

// Order and Venue definitions
struct Order {
    std::string symbol;
    double price;
    double quantity;
    std::string side; // "buy" or "sell"
};

struct Venue {
    std::string name;
    double latency;       // in microseconds
    double reliability;   // reliability factor (closer to 1 is better)
    std::atomic<bool> available;

    // Constructor
    Venue(const std::string& n, double l, double r)
        : name(n), latency(l), reliability(r), available(true) {}

    // Delete copy constructor and assignment (because of std::atomic)
    Venue(const Venue&) = delete;
    Venue& operator=(const Venue&) = delete;

    // Allow move semantics
    Venue(Venue&& other) noexcept
        : name(std::move(other.name)),
          latency(other.latency),
          reliability(other.reliability),
          available(other.available.load()) {}

    Venue& operator=(Venue&& other) noexcept {
        if (this != &other) {
            name = std::move(other.name);
            latency = other.latency;
            reliability = other.reliability;
            available.store(other.available.load());
        }
        return *this;
    }
};

// SmartOrderRouter implementation
class SmartOrderRouter {
private:
    std::vector<Venue> venues;
    std::mutex venueMutex;

    // Order translation logic placeholder
    Order translateOrder(const Order& order) {
        return order; // placeholder for actual venue-specific transformation
    }

public:
    SmartOrderRouter(std::vector<Venue>&& v) : venues(std::move(v)) {}

    void rankVenues() {
        std::lock_guard<std::mutex> lock(venueMutex);
        std::sort(venues.begin(), venues.end(), [](const Venue& a, const Venue& b) {
            return (a.latency / a.reliability) < (b.latency / b.reliability);
        });
    }

    void sendOrderAsync(const Order& order) {
        std::vector<std::future<void>> futures;
        futures.reserve(venues.size());
        Order translatedOrder = translateOrder(order);

        for (auto& venue : venues) {
            if (!venue.available.load(std::memory_order_acquire))
                continue;

            futures.emplace_back(std::async(std::launch::async, [&translatedOrder, &venue]() {
                try {
                    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(venue.latency)));
                    std::cout << "Order for " << translatedOrder.symbol
                              << " routed to venue: " << venue.name << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Error routing order to venue " << venue.name
                              << ": " << e.what() << std::endl;
                }
            }));
        }

        for (auto& f : futures) {
            try {
                f.get();
            } catch (const std::exception& e) {
                std::cerr << "Async task error: " << e.what() << std::endl;
            }
        }
    }

    void routeOrder(const Order& order) {
        rankVenues();
        sendOrderAsync(order);
    }
};

// MAIN
int main() {
    try {
        std::vector<Venue> venues;
        venues.emplace_back("VenueA", 50.0, 0.99);
        venues.emplace_back("VenueB", 30.0, 0.97);
        venues.emplace_back("VenueC", 70.0, 0.995);

        SmartOrderRouter router(std::move(venues));
        Order order = { "EUR/USD", 1.1234, 1000000, "buy" };

        router.routeOrder(order);
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error in SmartOrderRouter: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
