#include "ZMQPubSubHandler.hpp"
#include <iostream>
#include <thread>

ZMQPubSubHandler::ZMQPubSubHandler(ZeroMQConnectionManager& manager, const std::string& pubEndpoint, const std::string& subEndpoint)
    : manager_(manager)
{
    pubSocket_ = manager_.createSocket(ZMQ_PUB, pubEndpoint, true);
    subSocket_ = manager_.createSocket(ZMQ_SUB, subEndpoint, false);
}

ZMQPubSubHandler::~ZMQPubSubHandler() {
    // Sockets are automatically cleaned up.
}

void ZMQPubSubHandler::publish(const std::string& topic, const std::string& message) {
    zmq::message_t topicMsg(topic.begin(), topic.end());
    zmq::message_t messageMsg(message.begin(), message.end());
    pubSocket_->send(topicMsg, zmq::send_flags::sndmore);
    pubSocket_->send(messageMsg, zmq::send_flags::none);
}

void ZMQPubSubHandler::subscribe(const std::string& topic) {
    subSocket_->set(zmq::sockopt::subscribe, topic);
}

void ZMQPubSubHandler::setMessageHandler(std::function<void(const std::string&, const std::string&)> handler) {
    messageHandler_ = handler;
}

void ZMQPubSubHandler::startListening() {
    std::thread listener([this]() {
        while (true) {
            zmq::message_t topicMsg;
            zmq::message_t messageMsg;
            subSocket_->recv(topicMsg);
            subSocket_->recv(messageMsg);
            std::string topic(static_cast<char*>(topicMsg.data()), topicMsg.size());
            std::string message(static_cast<char*>(messageMsg.data()), messageMsg.size());
            if (messageHandler_) {
                messageHandler_(topic, message);
            } else {
                std::cout << "Received on topic [" << topic << "]: " << message << std::endl;
            }
        }
    });
    listener.detach();
}
