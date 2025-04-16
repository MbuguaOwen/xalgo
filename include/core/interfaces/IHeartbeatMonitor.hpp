// File: include/core/interfaces/IHeartbeatMonitor.hpp
#pragma once
#include <string>

class IHeartbeatMonitor {
public:
    virtual void startMonitoring() = 0;
    virtual void stopMonitoring() = 0;
    virtual bool isAlive(const std::string& componentId) const = 0;
    virtual void onHeartbeatTimeout(const std::string& componentId) = 0;
    virtual void registerComponent(const std::string& componentId) = 0;
    virtual void deregisterComponent(const std::string& componentId) = 0;
    virtual ~IHeartbeatMonitor() = default;
};
