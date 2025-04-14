# tests/test_data_integrity.py

import pytest
import pandas as pd
from backtester.data import TickReplayEngine
from datetime import timedelta
import logging
import time

# Configure test-level logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

TICK_DATA_FILE = "sample_tick_data.csv"

@pytest.fixture(scope="module")
def engine():
    return TickReplayEngine(TICK_DATA_FILE)


def test_tick_replay_accuracy(engine):
    """
    Validate that the replay engine correctly reconstructs the order book snapshot at a given timestamp.
    """
    snapshot = engine.get_order_book_snapshot("2023-01-01T10:00:00.000000000Z")
    expected = {
        "bid": 1.1234,
        "ask": 1.1236,
        "bid_size": 1_000_000,
        "ask_size": 900_000
    }
    assert snapshot == expected, f"Expected {expected}, got {snapshot}"


def test_bid_less_than_ask(engine):
    """
    Validate that the bid is always less than the ask across all ticks.
    """
    ticks = engine.get_all_snapshots()
    for i, tick in enumerate(ticks):
        assert tick['bid'] < tick['ask'], f"Tick {i} has invalid bid/ask: {tick}"


def test_no_data_gaps(engine):
    """
    Ensure there are no large time gaps (> 10 seconds) between consecutive ticks.
    """
    timestamps = engine.get_all_timestamps()
    diffs = [t2 - t1 for t1, t2 in zip(timestamps[:-1], timestamps[1:])]
    max_gap = max(diffs)
    logger.info(f"Maximum tick gap: {max_gap.total_seconds()}s")
    assert max_gap < timedelta(seconds=10), f"Data gap detected: {max_gap}"


def test_chronological_integrity(engine):
    """
    Check if tick data is strictly time-sorted and non-overlapping.
    """
    timestamps = engine.get_all_timestamps()
    for i in range(1, len(timestamps)):
        assert timestamps[i] > timestamps[i - 1], f"Out-of-order timestamp at index {i}"


def test_tick_replay_performance(engine):
    """
    Ensure tick replay engine can handle a high number of ticks within latency bounds.
    """
    timestamps = engine.get_all_timestamps()
    start_time = time.perf_counter()
    for ts in timestamps[:1000]:  # Simulate tick replay for 1000 entries
        engine.get_order_book_snapshot(ts)
    duration = time.perf_counter() - start_time
    logger.info(f"Replayed 1000 ticks in {duration:.6f} seconds")
    assert duration < 1.0, "Tick replay is too slow (>1s for 1000 ticks)"
