// File: include/core/interfaces/ICircuitBreaker.hpp
#pragma once

class ICircuitBreaker {
public:
    virtual void recordFailure(const std::string& componentId) = 0;
    virtual void reset(const std::string& componentId) = 0;
    virtual bool isOpen(const std::string& componentId) const = 0;
    virtual void setThresholds(const std::string& componentId, int failureLimit, int timeoutMs) = 0;
    virtual ~ICircuitBreaker() = default;
};