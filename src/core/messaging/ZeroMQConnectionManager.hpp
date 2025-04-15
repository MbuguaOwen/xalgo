#ifndef ZEROMQ_CONNECTION_MANAGER_HPP
#define ZEROMQ_CONNECTION_MANAGER_HPP

#include <zmq.hpp>
#include <string>
#include <memory>

/**
 * @brief Manages the ZeroMQ context and provides helper functions for creating sockets.
 */
class ZeroMQConnectionManager {
public:
    /**
     * @brief Constructor that initializes the ZeroMQ context using a given configuration file.
     * @param configFile Path to the ZeroMQ configuration file (JSON).
     */
    ZeroMQConnectionManager(const std::string& configFile);
    
    /**
     * @brief Destructor.
     */
    ~ZeroMQConnectionManager();

    /**
     * @brief Returns the underlying ZeroMQ context.
     * @return Reference to the zmq::context_t.
     */
    zmq::context_t& getContext();

    /**
     * @brief Create a new ZeroMQ socket of specified type and bind or connect it to an endpoint.
     * @param type Socket type (e.g., ZMQ_PUB, ZMQ_SUB).
     * @param endpoint The endpoint to bind or connect.
     * @param bind If true, the socket is bound; otherwise, it is connected.
     * @return Unique pointer to the created zmq::socket_t.
     */
    std::unique_ptr<zmq::socket_t> createSocket(int type, const std::string& endpoint, bool bind = true);

private:
    zmq::context_t context_;
};

#endif  // ZEROMQ_CONNECTION_MANAGER_HPP
