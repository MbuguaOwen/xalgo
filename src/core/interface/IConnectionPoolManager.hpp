// File: include/core/interfaces/IConnectionPoolManager.hpp
#pragma once
#include <string>
#include <vector>
#include <memory>

class IConnectionPoolManager {
public:
    virtual void addConnection(const std::string& brokerId) = 0;
    virtual void removeConnection(const std::string& brokerId) = 0;
    virtual bool hasAvailableConnection(const std::string& brokerId) const = 0;
    virtual size_t getPoolSize(const std::string& brokerId) const = 0;
    virtual void clearPool() = 0;
    virtual ~IConnectionPoolManager() = default;
};

