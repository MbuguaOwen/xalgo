#pragma once

#include <vector>
#include <chrono>
#include <stdexcept>
#include <cmath>
#include <mutex>
#include <Eigen/Dense> // Requires Eigen library for matrix operations

namespace TradingSystem {

/// @brief Container for tick data (bid/ask prices, volumes, etc.)
struct TickData {
    double eurUsd;
    double gbpUsd;
    double eurGbp;
    std::chrono::steady_clock::time_point timestamp;
};

/// @brief Encapsulates the Johansen cointegration test logic.
/// This is a simplified version for demonstration purposes.
/// In production, robust statistical testing is required.
class JohansenTestEngine {
public:
    JohansenTestEngine() = default;
    ~JohansenTestEngine() = default;

    /// @brief Runs the Johansen cointegration test on the provided time series.
    /// @param data A vector of vectors where each inner vector represents a time series for one asset.
    /// @return true if cointegration is detected, false otherwise.
    bool runTest(const std::vector<Eigen::VectorXd>& data) noexcept {
        try {
            // Check that at least two time series are provided (here we expect three)
            if(data.size() < 2) return false;

            // Construct a data matrix with each column as a time series
            size_t n = data[0].size(); 
            Eigen::MatrixXd mat(data.size(), n);
            for (size_t i = 0; i < data.size(); i++) {
                if (data[i].size() != n) {
                    throw std::invalid_argument("All time series must be of equal length");
                }
                mat.row(i) = data[i].transpose();
            }

            // Center the data: subtract the mean from each series
            for (size_t i = 0; i < mat.rows(); i++) {
                mat.row(i).array() -= mat.row(i).mean();
            }
            
            // Here, perform eigenvalue decomposition on the covariance matrix.
            // In practice, you’d compute the trace statistics and compare to critical values.
            Eigen::MatrixXd cov = (mat * mat.transpose()) / static_cast<double>(n - 1);
            Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigensolver(cov);
            if (eigensolver.info() != Eigen::Success) {
                throw std::runtime_error("Eigen decomposition failed in Johansen test");
            }
            // This simplified test: if the smallest eigenvalue is below a threshold,
            // assume cointegration exists.
            double threshold = 1e-5;
            return (eigensolver.eigenvalues()(0) < threshold);
        } catch (...) {
            // In production log the exception details for monitoring and error recovery.
            return false;
        }
    }
};

/// @brief SignalEngine responsible for computing the spread and Z-score.
/// Spread formula: spread = EURUSD - (EURGBP * GBPUSD)
class SignalEngine {
public:
    SignalEngine() = default;
    ~SignalEngine() = default;

    /// @brief Computes the spread from the current tick data.
    inline double computeSpread(const TickData &tick) const noexcept {
        return tick.eurUsd - (tick.eurGbp * tick.gbpUsd);
    }

    /// @brief Computes an adaptive Z-score given the spread series.
    /// @param spreads A vector of spread values.
    /// @return Calculated Z-score.
    double computeAdaptiveZScore(const std::vector<double>& spreads) const noexcept {
        if(spreads.empty()) return 0.0;

        double mean = 0.0;
        for (double s : spreads) {
            mean += s;
        }
        mean /= spreads.size();

        double variance = 0.0;
        for (double s : spreads) {
            variance += (s - mean) * (s - mean);
        }
        variance /= spreads.size();

        double stddev = std::sqrt(variance);
        if(stddev == 0) return 0.0;
        // Latest spread is the last element.
        double latest = spreads.back();
        return (latest - mean) / stddev;
    }
};

} // namespace TradingSystem
