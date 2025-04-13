# tests/test_execution.py
from backtester.execution import ExecutionSimulator

def test_limit_order_fill():
    sim = ExecutionSimulator()
    result = sim.place_limit_order(price=1.2345, qty=100000)
    assert result.filled_qty <= result.requested_qty
    assert result.status in ("filled", "partially_filled", "open")