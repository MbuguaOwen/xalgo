#include "ZeroMQConnectionManager.hpp"
#include <iostream>

ZeroMQConnectionManager::ZeroMQConnectionManager(const std::string& configFile)
    : context_(1)  // Using one I/O thread.
{
    // TODO: Load configuration from configFile (e.g., using a JSON parser).
    std::cout << "ZeroMQConnectionManager initialized with config: " << configFile << std::endl;
}

ZeroMQConnectionManager::~ZeroMQConnectionManager() {
    // The context destructor cleans up sockets.
}

zmq::context_t& ZeroMQConnectionManager::getContext() {
    return context_;
}

std::unique_ptr<zmq::socket_t> ZeroMQConnectionManager::createSocket(int type, const std::string& endpoint, bool bind) {
    auto socket = std::make_unique<zmq::socket_t>(context_, type);
    if (bind) {
        socket->bind(endpoint);
    } else {
        socket->connect(endpoint);
    }
    return socket;
}
