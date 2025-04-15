import logging
import time
from typing import List, Dict, Any,  Optional

# Import the Internal Simulator from our modular simulation components.
from internal_simulator import InternalSimulator

import numpy as np  # Used for computing correlation

logger = logging.getLogger(__name__)

class MockOrderRouter:
    """
    A mock order router to simulate order routing performance.
    
    This class accepts orders (derived from simulated market data) and simulates 
    processing delays, capturing the routing time and other synthetic performance metrics.
    
    Attributes:
        base_processing_time (float): The baseline processing delay in seconds.
        variability (float): Additional delay variability factor.
    """
    def __init__(self, base_processing_time: float = 0.001, variability: float = 0.001):
        self.base_processing_time = base_processing_time
        self.variability = variability
    
    def route_order(self, order_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Simulate routing an order and record its processing time.
        
        Args:
            order_data: Dictionary containing order information with an optional impact factor.
        
        Returns:
            A dictionary containing order routing results.
        """
        start_time = time.time()
        # Simulate delay based on base processing time and an impact factor derived from market state.
        delay = self.base_processing_time + self.variability * order_data.get("impact_factor", 1)
        time.sleep(delay)
        processing_time = time.time() - start_time
        
        result = {
            "order_id": order_data.get("order_id", "unknown"),
            "status": "routed",
            "processing_time": processing_time,
            "details": order_data,
        }
        logger.debug("Order routed: %s", result)
        return result

class RoutingPerformanceAssessor:
    """
    Integrates the internal simulator with a mock order router to assess routing performance.
    
    This class runs simulation cycles to generate market data, then feeds derived order data
    into the mock router. It measures routing latencies and collects performance metrics across
    various simulated stress conditions.
    
    Attributes:
        simulator (InternalSimulator): The core simulator that replays market data and applies scenarios.
        order_router (MockOrderRouter): The simulated order router.
        performance_metrics (List[Dict[str, Any]]): Collected metrics for analysis.
    """
    def __init__(self, market_data: List[Dict[str, Any]]):
        self.simulator = InternalSimulator(market_data)
        self.order_router = MockOrderRouter()
        self.performance_metrics: List[Dict[str, Any]] = []
        self.order_book_lags: List[float] = []
        self.processing_times: List[float] = []
    
    def run_assessment(self, static_scenario: str = None):
        """
        Runs the simulation and routes orders, measuring routing performance.
        
        Args:
            static_scenario: Optional static scenario (by name) to apply to market data before evaluation.
        """
        logger.info("Starting routing performance assessment.")
        
        # Retrieve market data records using the simulator's replay engine.
        for market_data in self.simulator.market_replay_engine.replay():
            # If provided, apply a static scenario.
            if static_scenario:
                market_data = self.simulator.scenario_designer.apply_scenario(static_scenario, market_data)
            # Apply dynamic scenarios.
            market_data = self.simulator.scenario_designer.apply_dynamic_scenarios(market_data)
            simulation_timestamp = market_data.get("timestamp", time.time())
            update_start = time.time()
            update_end = time.time()
            # Update the synthetic order book and simulate latency.
            self.simulator.order_book.update(market_data)
            self.simulator.latency_emulator.emulate_latency()
            
            update_lag = update_end - simulation_timestamp
            self.order_book_lags.append(update_lag)
            logger.debug("Order book update lag: %fs", update_lag)
            # Generate an order based on market data.
            # Here, a synthetic "impact_factor" is computed from the number of bid levels.
            order_data = {
                "order_id": f"order_{market_data.get('timestamp')}",
                "impact_factor": len(market_data.get("bids", [])) / 10.0,  # sample metric for variability
                "market_snapshot": market_data
            }
            self.simulator.latency_emulator.emulate_latency()
            
            # Generate an order based on market snapshot.
            order_data = {
                "order_id": f"order_{market_data.get('timestamp')}",
                "impact_factor": len(market_data.get("bids", [])) / 10.0,  # A synthetic variability metric.
                "market_snapshot": market_data
            }
            
            # Route the order and record the performance.
            result = self.order_router.route_order(order_data)
            self.processing_times.append(result.get("processing_time", 0))
            self.performance_metrics.append(result)
            logger.info("Routing Performance: %s", result)
        
        logger.info("Routing performance assessment completed.")

    def get_performance_metrics(self) -> List[Dict[str, Any]]:
        """
        Returns the collected performance metrics.
        
        Returns:
            List of dictionaries with routing performance details.
        """
        return self.performance_metrics
    def _log_additional_metric(self, key: str, value: float):
        """
        Log additional performance metrics for future analysis.
        
        Args:
            key: The metric name.
            value: The metric value.
        """
        # In production, you might integrate with a monitoring system here.
        logger.info("Additional Metric - %s: %f", key, value)
# Example usage for standalone testing:
if __name__ == "__main__":
    # Sample historical market data records.
    sample_data = [
        {"timestamp": 1.0, "bids": [100, 99], "asks": [101, 102]},
        {"timestamp": 2.0, "bids": [101, 100], "asks": [102, 103]},
        {"timestamp": 3.0, "bids": [102, 101], "asks": [103, 104]}
    ]
    
    # Create an assessor instance using the sample market data.
    assessor = RoutingPerformanceAssessor(sample_data)
    
    # Optionally, you could register scenarios on the simulator as already available.
    # For demonstration, we run without a static scenario.
    assessor.run_assessment(static_scenario=None)
    
    # Retrieve and log the performance metrics.
    metrics = assessor.get_performance_metrics()
    for metric in metrics:
        logger.info("Collected Metric: %s", metric)

if self.order_book_lags and self.processing_times:
            try:
                correlation = float(np.corrcoef(self.order_book_lags, self.processing_times)[0, 1])
                logger.info("Correlation between Order Book Lag and Processing Time: %f", correlation)
                self._log_additional_metric("order_book_processing_correlation", correlation)
            except Exception as e:
                logger.error("Failed to compute correlation: %s", str(e))
            else:
                logger.warning("Insufficient data to compute correlation.")
        
                logger.info("Routing performance assessment completed.")
    