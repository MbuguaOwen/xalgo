#pragma once

#include "core/Logger.hpp"
#include "core/Metrics.hpp"
#include "messaging/ZeroMQConnectionManager.hpp"
#include <zmq.hpp>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

namespace hft {
namespace core {
namespace messaging {

/**
 * @brief Handler for PUB/SUB messaging over ZeroMQ
 */
class ZMQPubSubHandler {
public:
    /**
     * @brief Constructor
     * 
     * @param manager   Reference to the ZeroMQ connection manager
     * @param logger    Logger instance
     * @param metrics   Metrics collector
     * @param pubName   Identifier for the publisher socket
     * @param pubEndpoint   Endpoint for publisher to bind/connect
     * @param subName   Identifier for the subscriber socket
     * @param subEndpoint   Endpoint for subscriber to bind/connect
     * @param topics    Initial list of topics to subscribe to
     */
    ZMQPubSubHandler(
        ZeroMQConnectionManager& manager,
        std::shared_ptr<Logger> logger,
        std::shared_ptr<Metrics> metrics,
        const std::string& pubName,
        const std::string& pubEndpoint,
        const std::string& subName,
        const std::string& subEndpoint,
        const std::vector<std::string>& topics = {}
    );

    ~ZMQPubSubHandler();

    /**
     * @brief Publish a message under a topic
     */
    void publish(const std::string& topic, const std::string& message);

    /**
     * @brief Subscribe to a new topic at runtime
     */
    void subscribe(const std::string& topic);

    /**
     * @brief Set the callback for incoming messages
     */
    void setMessageHandler(std::function<void(const std::string&, const std::string&)> handler);

    /**
     * @brief Start background listener thread
     */
    void start();

    /**
     * @brief Stop background listener thread
     */
    void stop();

private:
    void listenLoop();

    ZeroMQConnectionManager&                   manager_;
    std::shared_ptr<Logger>                   logger_;
    std::shared_ptr<Metrics>                  metrics_;
    std::shared_ptr<zmq::socket_t>            pubSocket_;
    std::shared_ptr<zmq::socket_t>            subSocket_;
    ConnectionHealth&                         pubHealth_;
    ConnectionHealth&                         subHealth_;
    std::atomic<bool>                         running_{false};
    std::thread                               listenThread_;
    std::mutex                                handlerMutex_;
    std::function<void(const std::string&, const std::string&)> messageHandler_;
};

} // namespace messaging
} // namespace core
} // namespace hft
