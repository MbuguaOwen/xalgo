# tests/test_regime_detection.py

import pytest
import pandas as pd
from backtester.regimes import RegimeDetector

@pytest.fixture
def labelled_data():
    return pd.read_csv("labelled_regime_data.csv")

def test_transition_detection(labelled_data):
    detector = RegimeDetector()
    transitions = detector.detect_transitions(labelled_data)

    assert transitions is not None, "Transition result is None"
    assert isinstance(transitions, list), "Expected transitions to be a list"
    assert len(transitions) > 0, "No regime transitions detected"

def test_label_consistency(labelled_data):
    unique_labels = labelled_data['regime'].unique()
    assert len(unique_labels) >= 2, f"Only one regime label found: {unique_labels}"

def test_transition_order(labelled_data):
    detector = RegimeDetector()
    transitions = detector.detect_transitions(labelled_data)

    previous_index = -1
    for t in transitions:
        assert t['index'] > previous_index, "Transitions not strictly increasing"
        previous_index = t['index']

def test_massive_order_spam(simulator):
    orders = [simulator.place_limit_order(1.2345, 10_000) for _ in range(1000)]
    filled = sum(1 for o in orders if o.status != "open")
    assert filled > 0, "No orders were filled under spam load"

def test_slippage_model(simulator):
    order = simulator.place_limit_order(1.2345, 100_000)
    slippage = abs(order.execution_price - 1.2345)
    max_expected = 1.2345 * (simulator.slippage_bps / 10_000)
    assert slippage <= max_expected, f"Excessive slippage: {slippage} > {max_expected}"
