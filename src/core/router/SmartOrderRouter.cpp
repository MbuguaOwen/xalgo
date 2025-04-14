// SmartOrderRouter.cpp
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

    Venue(const std::string& n, double l, double r)
        : name(n), latency(l), reliability(r), available(true) {}
};

// SmartOrderRouter implementation
class SmartOrderRouter {
private:
    std::vector<Venue> venues;
    // Using a mutex only in ranking; order routing is handled concurrently.
    std::mutex venueMutex;

    // Minimal order translation: in production this might convert orders to a venue-specific format.
    Order translateOrder(const Order& order) {
        // Here we simply return a copy. Additional translation logic can be added.
        return order;
    }

public:
    SmartOrderRouter(const std::vector<Venue>& v) : venues(v) {}

    // Rank venues based on (latency/reliability)
    void rankVenues() {
        std::lock_guard<std::mutex> lock(venueMutex);
        std::sort(venues.begin(), venues.end(), [](const Venue& a, const Venue& b) {
            return (a.latency / a.reliability) < (b.latency / b.reliability);
        });
    }

    // Asynchronously send the order to each available venue.
    void sendOrderAsync(const Order& order) {
        // Prepare a vector for futures with reserved space to minimize dynamic allocations.
        std::vector<std::future<void>> futures;
        futures.reserve(venues.size());
        Order translatedOrder = translateOrder(order);

        for (auto& venue : venues) {
            // Only send to available venues.
            if (!venue.available.load(std::memory_order_acquire))
                continue;

            // Launch asynchronous tasks using std::async.
            futures.push_back(std::async(std::launch::async, [&translatedOrder, &venue]() {
                try {
                    // Simulate minimal network latency.
                    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(venue.latency)));
                    // In production, this would be a network order submission call.
                    std::cout << "Order for " << translatedOrder.symbol
                              << " routed to venue: " << venue.name << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Error routing order to venue " << venue.name << ": " << e.what() << std::endl;
                }
            }));
        }
        // Ensure all asynchronous tasks complete.
        for (auto& f : futures) {
            try {
                f.get();
            } catch (const std::exception& e) {
                std::cerr << "Async task error: " << e.what() << std::endl;
            }
        }
    }

    // High-level function to route an order.
    void routeOrder(const Order& order) {
        rankVenues();
        sendOrderAsync(order);
    }
};

int main() {
    try {
        // Define a few venues with simulated latencies (in μs) and reliability factors.
        std::vector<Venue> venues = {
            Venue("VenueA", 50.0, 0.99),
            Venue("VenueB", 30.0, 0.97),
            Venue("VenueC", 70.0, 0.995)
        };

        SmartOrderRouter router(venues);
        Order order = {"EUR/USD", 1.1234, 1000000, "buy"};

        // Route the order; expect sub-100μs (per venue) plus overhead
        router.routeOrder(order);
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error in SmartOrderRouter: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
