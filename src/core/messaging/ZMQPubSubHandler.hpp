#ifndef ZMQ_PUBSUB_HANDLER_HPP
#define ZMQ_PUBSUB_HANDLER_HPP

#include "ZeroMQConnectionManager.hpp"
#include <zmq.hpp>
#include <string>
#include <functional>

/**
 * @brief Provides a basic publisher/subscriber interface using ZeroMQ.
 */
class ZMQPubSubHandler {
public:
    /**
     * @brief Constructor that sets up publishing and subscribing endpoints.
     * @param manager Reference to an existing ZeroMQConnectionManager.
     * @param pubEndpoint The endpoint for publishing messages.
     * @param subEndpoint The endpoint for subscribing to messages.
     */
    ZMQPubSubHandler(ZeroMQConnectionManager& manager, const std::string& pubEndpoint, const std::string& subEndpoint);
    
    /**
     * @brief Destructor.
     */
    ~ZMQPubSubHandler();

    /**
     * @brief Publish a message on a given topic.
     * @param topic The topic to publish.
     * @param message The message content.
     */
    void publish(const std::string& topic, const std::string& message);

    /**
     * @brief Subscribe to a topic.
     * @param topic The topic to subscribe to.
     */
    void subscribe(const std::string& topic);

    /**
     * @brief Set a callback function to handle received messages.
     * @param handler A function taking the topic and message.
     */
    void setMessageHandler(std::function<void(const std::string&, const std::string&)> handler);

    /**
     * @brief Start the subscriber loop in a separate thread.
     */
    void startListening();

private:
    ZeroMQConnectionManager& manager_;
    std::unique_ptr<zmq::socket_t> pubSocket_;
    std::unique_ptr<zmq::socket_t> subSocket_;
    std::function<void(const std::string&, const std::string&)> messageHandler_;
};

#endif  // ZMQ_PUBSUB_HANDLER_HPP
