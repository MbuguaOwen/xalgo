# tests/test_data_integrity.py
import pytest
import pandas as pd
from backtester.data import TickReplayEngine

def test_tick_replay_accuracy():
    engine = TickReplayEngine("sample_tick_data.csv")
    snapshot = engine.get_order_book_snapshot("2023-01-01T10:00:00.000000000Z")
    expected = {
        "bid": 1.1234,
        "ask": 1.1236,
        "bid_size": 1000000,
        "ask_size": 900000
    }
    assert snapshot == expected

def test_no_data_gaps():
    engine = TickReplayEngine("sample_tick_data.csv")
    timestamps = engine.get_all_timestamps()
    diffs = [t2 - t1 for t1, t2 in zip(timestamps[:-1], timestamps[1:])]
    assert max(diffs).total_seconds() < 10, "Gap exceeds 10 seconds"
