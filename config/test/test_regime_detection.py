# tests/test_regime_detection.py
from backtester.regimes import RegimeDetector
import pandas as pd

def test_transition_detection():
    detector = RegimeDetector()
    data = pd.read_csv("labelled_regime_data.csv")
    transitions = detector.detect_transitions(data)
    assert len(transitions) > 0, "No regime transitions detected"