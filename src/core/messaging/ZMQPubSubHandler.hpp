// ZMQPubSubHandler.hpp
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include <zmq.hpp>

#include "core/messaging/ZeroMQConnectionManager.hpp"
#include "core/Logger.hpp"
#include "core/Metrics.hpp"

namespace hft {
namespace core {
namespace messaging {

/**
 * @brief Simple pub/sub handler on top of ZeroMQConnectionManager
 */
class ZMQPubSubHandler {
public:
    /**
     * @brief Constructor
     *
     * Creates one PUB socket and one SUB socket,
     * wires up health monitoring, but does not start the receive loop.
     *
     * @param connectionManager shared ZMQ connection manager
     * @param logger            shared logger
     * @param metrics           shared metrics collector
     * @param pubName           identifier for this publisher
     * @param pubEndpoint       ZMQ endpoint to bind/connect publisher
     * @param subName           identifier for this subscriber
     * @param subEndpoint       ZMQ endpoint to bind/connect subscriber
     * @param topics            list of topics to SUBSCRIBE to
     */
    ZMQPubSubHandler(
        std::shared_ptr<ZeroMQConnectionManager> connectionManager,
        std::shared_ptr<Logger>                logger,
        std::shared_ptr<Metrics>               metrics,
        const std::string&                     pubName,
        const std::string&                     pubEndpoint,
        const std::string&                     subName,
        const std::string&                     subEndpoint,
        const std::vector<std::string>&        topics
    );

    ~ZMQPubSubHandler();

    /**
     * @brief Publish a single message (topic + payload).
     * @return true on success, false on ZMQ error.
     */
    bool publish(const std::string& topic, const std::string& message);

    /**
     * @brief Dynamically subscribe to an additional topic.
     * @return true on success, false on ZMQ error.
     */
    bool subscribe(const std::string& topic);

    /**
     * @brief Install your own in‐process handler for inbound messages.
     */
    void setMessageHandler(std::function<void(const std::string& topic,
                                              const std::string& payload)> handler);

    /**
     * @brief Launch the background receive loop.
     */
    void start();

    /**
     * @brief Stop the receive loop and join its thread.
     */
    void stop();

private:
    void listenLoop();

    std::shared_ptr<ZeroMQConnectionManager> manager_;
    std::shared_ptr<Logger>                 logger_;
    std::shared_ptr<Metrics>                metrics_;

    ConnectionHealth&                       pubHealth_;
    ConnectionHealth&                       subHealth_;

    std::shared_ptr<zmq::socket_t>          pubSocket_;
    std::shared_ptr<zmq::socket_t>          subSocket_;

    std::atomic<bool>                       running_{false};
    std::thread                             listenThread_;
    std::mutex                              handlerMutex_;
    std::function<void(const std::string&, const std::string&)> messageHandler_;
};

} // namespace messaging
} // namespace core
} // namespace hft
