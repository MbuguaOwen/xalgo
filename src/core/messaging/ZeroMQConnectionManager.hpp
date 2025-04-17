// ZeroMQConnectionManager.hpp
#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <zmq.hpp>

#include "core/Logger.hpp"
#include "core/Metrics.hpp"
#include "core/ConfigLoader.hpp"

namespace hft {
namespace core {
namespace messaging {

/**
 * @brief Connection health status information
 */
struct ConnectionHealth {
    std::atomic<bool> isConnected{false};
    std::atomic<uint64_t> lastHeartbeatTime{0};
    std::atomic<uint64_t> messagesSent{0};
    std::atomic<uint64_t> messagesReceived{0};
    std::atomic<uint64_t> errorCount{0};
    std::atomic<uint64_t> reconnectCount{0};
    std::string lastError{""};
};

/**
 * @brief ZeroMQConnectionManager handles connections, reconnects and health monitoring
 * for all ZMQ sockets in the system.
 */
class ZeroMQConnectionManager {
public:
    /**
     * @brief Primary constructor
     * 
     * @param context ZMQ context to use
     * @param config Configuration object containing connection settings
     * @param logger Logger instance
     * @param metrics Metrics collection instance
     */
    ZeroMQConnectionManager(
        zmq::context_t& context,
        const ConfigLoader& config,
        std::shared_ptr<Logger> logger,
        std::shared_ptr<Metrics> metrics
    );

    /**
     * @brief Convenience constructor that loads config from a file path
     */
    explicit ZeroMQConnectionManager(const std::string& configFile);

    ~ZeroMQConnectionManager();

    /**
     * @brief Access underlying ZMQ context (useful if you need raw sockets)
     */
    zmq::context_t& getContext();

    /**
     * @brief Low‑level socket factory
     */
    std::unique_ptr<zmq::socket_t> createSocket(int type, const std::string& endpoint, bool bind = false);

    /**
     * @brief Creates a publisher socket
     */
    std::shared_ptr<zmq::socket_t> createPublisher(const std::string& name, const std::string& endpoint);

    /**
     * @brief Creates a subscriber socket
     */
    std::shared_ptr<zmq::socket_t> createSubscriber(
        const std::string& name, 
        const std::string& endpoint,
        const std::vector<std::string>& topics = {""}
    );

    /**
     * @brief Starts health monitoring for all sockets
     */
    void startHealthMonitoring();

    /**
     * @brief Gets the health status for a specific connection
     */
    const ConnectionHealth& getConnectionHealth(const std::string& name) const;

    /**
     * @brief Gets the list of all managed connections
     */
    std::vector<std::string> getConnectionNames() const;

private:
    // either points at external context _or_ ownedContext_.get()
    zmq::context_t* zmqContextPtr_;               
    std::unique_ptr<zmq::context_t> ownedContext_; 
    ConfigLoader                configLoader_;
    std::unordered_map<std::string, std::shared_ptr<zmq::socket_t>> sockets_;
    std::unordered_map<std::string, ConnectionHealth>               healthStatus_;
    std::unordered_map<std::string, std::shared_ptr<zmq::socket_t>> monitorSockets_;
    mutable std::mutex           socketMutex_;
    std::atomic<bool>            monitorRunning_{false};
    std::thread                  monitorThread_;

    std::shared_ptr<Logger>      logger_;
    std::shared_ptr<Metrics>     metrics_;
    int                          reconnectIntervalMs_;
    int                          heartbeatIntervalMs_;
    int                          monitoringIntervalMs_;

    void setupSocketMonitoring(const std::shared_ptr<zmq::socket_t>& socket, const std::string& name);
    void monitorThreadFunction();
    void processMonitorEvents(zmq::socket_t& monitorSocket, const std::string& name);
    void updateMetrics();
    void reconnectDisconnectedSockets();
};

} // namespace messaging
} // namespace core
} // namespace hft
