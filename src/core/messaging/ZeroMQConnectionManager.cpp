// ZeroMQConnectionManager.cpp
#include "ZeroMQConnectionManager.hpp"
#include <iostream>

using namespace hft::core::messaging;

//—— Primary constructor —————————————————————————————————————————————————————————————————
ZeroMQConnectionManager::ZeroMQConnectionManager(
    zmq::context_t& context,
    const ConfigLoader& config,
    std::shared_ptr<Logger> logger,
    std::shared_ptr<Metrics> metrics
)
    : zmqContextPtr_(&context)
    , ownedContext_(nullptr)
    , configLoader_(config)
    , logger_(std::move(logger))
    , metrics_(std::move(metrics))
    , reconnectIntervalMs_(configLoader_.getInt("reconnectIntervalMs", 1000))
    , heartbeatIntervalMs_(configLoader_.getInt("heartbeatIntervalMs", 5000))
    , monitoringIntervalMs_(configLoader_.getInt("monitoringIntervalMs", 1000))
{
    logger_->info("ZeroMQConnectionManager initialized");
}

//—— Convenience ctor: load from a JSON/YAML file path ——————————————————————————————————————
ZeroMQConnectionManager::ZeroMQConnectionManager(const std::string& configFile)
    : ownedContext_(std::make_unique<zmq::context_t>(1))  // One I/O thread
    , zmqContextPtr_(ownedContext_.get())
    , configLoader_(configFile)
    , logger_(nullptr)
    , metrics_(nullptr)
    , reconnectIntervalMs_(configLoader_.getInt("reconnectIntervalMs", 1000))
    , heartbeatIntervalMs_(configLoader_.getInt("heartbeatIntervalMs", 5000))
    , monitoringIntervalMs_(configLoader_.getInt("monitoringIntervalMs", 1000))
{
    std::cout << "ZeroMQConnectionManager initialized with config file: " << configFile << std::endl;
}

ZeroMQConnectionManager::~ZeroMQConnectionManager() {
    monitorRunning_.store(false);
    if (monitorThread_.joinable()) monitorThread_.join();
    // ownedContext_ will clean up sockets if we created our own context
}

//—— Context accessor ——————————————————————————————————————————————————————————————
zmq::context_t& ZeroMQConnectionManager::getContext() {
    return *zmqContextPtr_;
}

//—— Low‑level socket factory ———————————————————————————————————————————————————————
std::unique_ptr<zmq::socket_t> ZeroMQConnectionManager::createSocket(int type, const std::string& endpoint, bool bind) {
    auto sock = std::make_unique<zmq::socket_t>(*zmqContextPtr_, type);
    if (bind)   sock->bind(endpoint);
    else        sock->connect(endpoint);
    return sock;
}

//—— Publisher + Subscriber helpers ————————————————————————————————————————————————————
std::shared_ptr<zmq::socket_t> ZeroMQConnectionManager::createPublisher(
    const std::string& name,
    const std::string& endpoint
) {
    auto sock = createSocket(ZMQ_PUB, endpoint, false);
    auto sp   = std::shared_ptr<zmq::socket_t>(std::move(sock));

    {
        std::lock_guard<std::mutex> lk(socketMutex_);
        sockets_[name] = sp;
        healthStatus_[name] = ConnectionHealth{};
    }
    setupSocketMonitoring(sp, name);
    return sp;
}

std::shared_ptr<zmq::socket_t> ZeroMQConnectionManager::createSubscriber(
    const std::string& name,
    const std::string& endpoint,
    const std::vector<std::string>& topics
) {
    auto sock = createSocket(ZMQ_SUB, endpoint, false);
    for (auto& t : topics) {
        sock->setsockopt(ZMQ_SUBSCRIBE, t.data(), t.size());
    }
    auto sp   = std::shared_ptr<zmq::socket_t>(std::move(sock));

    {
        std::lock_guard<std::mutex> lk(socketMutex_);
        sockets_[name] = sp;
        healthStatus_[name] = ConnectionHealth{};
    }
    setupSocketMonitoring(sp, name);
    return sp;
}

//—— Stubbed health‑monitoring entrypoint —————————————————————————————————————————————
void ZeroMQConnectionManager::startHealthMonitoring() {
    if (monitorRunning_.exchange(true)) return;  // already running
    monitorThread_ = std::thread(&ZeroMQConnectionManager::monitorThreadFunction, this);
}

const ConnectionHealth& ZeroMQConnectionManager::getConnectionHealth(const std::string& name) const {
    std::lock_guard<std::mutex> lk(socketMutex_);
    return healthStatus_.at(name);
}

std::vector<std::string> ZeroMQConnectionManager::getConnectionNames() const {
    std::lock_guard<std::mutex> lk(socketMutex_);
    std::vector<std::string> names;
    for (auto& kv : sockets_) names.push_back(kv.first);
    return names;
}

//—— Remaining private methods need your health‑check + metrics logic ———————————————————————
void ZeroMQConnectionManager::setupSocketMonitoring(const std::shared_ptr<zmq::socket_t>&, const std::string&) {
    // TODO
}
void ZeroMQConnectionManager::monitorThreadFunction() {
    // TODO
}
void ZeroMQConnectionManager::processMonitorEvents(zmq::socket_t&, const std::string&) {
    // TODO
}
void ZeroMQConnectionManager::updateMetrics() {
    // TODO
}
void ZeroMQConnectionManager::reconnectDisconnectedSockets() {
    // TODO
}
