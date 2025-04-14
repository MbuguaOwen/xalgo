# tests/performance/test_throughput.py
import time
import logging
import pytest
from backtester.engine import TickReplayEngine

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)

@pytest.mark.performance
def test_throughput_benchmark():
    engine = TickReplayEngine("data/sample_tick_data.csv")
    
    # Warmup phase (optional)
    for _ in range(3):
        engine.reset()
        engine.replay_all()

    # Precise timing (nanoseconds)
    start_ns = time.perf_counter_ns()
    engine.reset()
    engine.replay_all()
    end_ns = time.perf_counter_ns()

    duration_s = (end_ns - start_ns) / 1e9
    throughput = engine.event_count / duration_s

    logger.info(f"Processed {engine.event_count} events in {duration_s:.6f}s => {throughput:.2f} events/sec")
    assert throughput > 100_000, f"Throughput below target: {throughput:.2f} < 100K events/sec"
