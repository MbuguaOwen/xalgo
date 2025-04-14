# tests/test_execution.py

import pytest
from backtester.execution import ExecutionSimulator
import time

@pytest.fixture
def simulator():
    return ExecutionSimulator(latency_sim_us=50, slippage_bps=2)

def test_limit_order_fill(simulator):
    result = simulator.place_limit_order(price=1.2345, qty=100_000)

    assert result.filled_qty <= result.requested_qty, "Filled more than requested"
    assert result.status in {"filled", "partially_filled", "open"}, f"Unexpected status: {result.status}"

def test_partial_fill_handling(simulator):
    # Simulate partial fill scenario
    result = simulator.place_limit_order(price=1.2345, qty=10_000_000)
    assert result.status in {"partially_filled", "open"}, "Should be partially filled due to size"

def test_execution_latency(simulator):
    start = time.perf_counter_ns()
    simulator.place_limit_order(price=1.2345, qty=1_000)
    duration_ns = time.perf_counter_ns() - start
    assert duration_ns < 100_000, f"Execution latency too high: {duration_ns} ns"

def test_order_status_consistency(simulator):
    result = simulator.place_limit_order(price=1.2345, qty=500_000)
    assert hasattr(result, 'status') and result.status is not None, "Missing status attribute"
    assert hasattr(result, 'filled_qty'), "Missing filled_qty attribute"
