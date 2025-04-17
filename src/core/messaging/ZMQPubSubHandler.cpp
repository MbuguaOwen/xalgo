// ZMQPubSubHandler.cpp
#include "messaging/ZMQPubSubHandler.hpp"
#include <iostream>

using namespace hft::core::messaging;

ZMQPubSubHandler::ZMQPubSubHandler(
    std::shared_ptr<ZeroMQConnectionManager> connectionManager,
    std::shared_ptr<Logger>                logger,
    std::shared_ptr<Metrics>               metrics,
    const std::string&                     pubName,
    const std::string&                     pubEndpoint,
    const std::string&                     subName,
    const std::string&                     subEndpoint,
    const std::vector<std::string>&        topics
)
    : manager_(std::move(connectionManager))
    , logger_(std::move(logger))
    , metrics_(std::move(metrics))
    , pubHealth_(manager_->getConnectionHealth(pubName))
    , subHealth_(manager_->getConnectionHealth(subName))
{
    // Create the two sockets
    pubSocket_ = manager_->createPublisher(pubName, pubEndpoint);
    subSocket_ = manager_->createSubscriber(subName, subEndpoint, topics);

    // Ensure health monitoring is running
    manager_->startHealthMonitoring();

    logger_->info("ZMQPubSubHandler initialized: pub='{}'->{}, sub='{}'->{}",
                  pubName, pubEndpoint, subName, subEndpoint);
}

ZMQPubSubHandler::~ZMQPubSubHandler() {
    stop();
    logger_->info("ZMQPubSubHandler shutdown complete");
}

bool ZMQPubSubHandler::publish(const std::string& topic, const std::string& message) {
    try {
        zmq::message_t t(topic.data(), topic.size());
        zmq::message_t p(message.data(), message.size());

        pubSocket_->send(t, zmq::send_flags::sndmore);
        pubSocket_->send(p, zmq::send_flags::none);

        ++pubHealth_.messagesSent;
        metrics_->increment("zmq.pub.messages_sent");
        return true;
    } catch (const zmq::error_t& e) {
        ++pubHealth_.errorCount;
        pubHealth_.lastError = e.what();
        logger_->error("Publish error on topic '{}': {}", topic, e.what());
        return false;
    }
}

bool ZMQPubSubHandler::subscribe(const std::string& topic) {
    try {
        subSocket_->setsockopt(ZMQ_SUBSCRIBE, topic.data(), topic.size());
        logger_->info("Subscribed to topic '{}'", topic);
        return true;
    } catch (const zmq::error_t& e) {
        ++subHealth_.errorCount;
        subHealth_.lastError = e.what();
        logger_->error("Subscribe error on topic '{}': {}", topic, e.what());
        return false;
    }
}

void ZMQPubSubHandler::setMessageHandler(
    std::function<void(const std::string&, const std::string&)> handler
) {
    std::lock_guard<std::mutex> lk(handlerMutex_);
    messageHandler_ = std::move(handler);
}

void ZMQPubSubHandler::start() {
    if (running_.exchange(true)) return;  // already running
    listenThread_ = std::thread(&ZMQPubSubHandler::listenLoop, this);
    logger_->info("Listener thread started");
}

void ZMQPubSubHandler::stop() {
    if (!running_.exchange(false)) return;  // already stopped
    if (listenThread_.joinable()) {
        listenThread_.join();
        logger_->info("Listener thread stopped");
    }
}

void ZMQPubSubHandler::listenLoop() {
    while (running_) {
        try {
            zmq::message_t t, p;
            // blocking recv
            if (!subSocket_->recv(t, zmq::recv_flags::none)) continue;
            subSocket_->recv(p, zmq::recv_flags::none);

            std::string topic(static_cast<char*>(t.data()), t.size());
            std::string payload(static_cast<char*>(p.data()), p.size());

            ++subHealth_.messagesReceived;
            metrics_->increment("zmq.sub.messages_received");

            std::lock_guard<std::mutex> lk(handlerMutex_);
            if (messageHandler_) {
                messageHandler_(topic, payload);
            } else {
                std::cout << "[ZMQSub] " << topic << " -> " << payload << std::endl;
            }
        } catch (const zmq::error_t& e) {
            ++subHealth_.errorCount;
            subHealth_.lastError = e.what();
            logger_->error("Error in listenLoop: {}", e.what());
        }
    }
}
