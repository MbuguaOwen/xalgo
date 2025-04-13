# tests/test_performance.py
import time

def test_throughput():
    engine = TickReplayEngine("sample_tick_data.csv")
    start = time.time()
    engine.replay_all()
    elapsed = time.time() - start
    assert engine.event_count / elapsed > 100000, "Throughput below target"
