// File: include/core/interfaces/IMultiBrokerAPIManager.hpp
#pragma once
#include <string>
#include "Order.hpp"

class IMultiBrokerAPIManager {
public:
    virtual void initializeConnections() = 0;
    virtual void sendOrder(const std::string& brokerId, const Order& order) = 0;
    virtual bool isConnectionHealthy(const std::string& brokerId) const = 0;
    virtual void reconnect(const std::string& brokerId) = 0;
    virtual void shutdown() = 0;
    virtual ~IMultiBrokerAPIManager() = default;
};