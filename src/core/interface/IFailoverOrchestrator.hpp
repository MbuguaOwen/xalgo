
// File: include/core/interfaces/IFailoverOrchestrator.hpp
#pragma once

class IFailoverOrchestrator {
public:
    virtual void evaluateHealth() = 0;
    virtual void triggerFailover(const std::string& failedComponentId) = 0;
    virtual std::string getCurrentPrimary() const = 0;
    virtual void registerComponent(const std::string& componentId) = 0;
    virtual void deregisterComponent(const std::string& componentId) = 0;
    virtual ~IFailoverOrchestrator() = default;
};
