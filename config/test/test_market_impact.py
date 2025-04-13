# tests/test_market_impact.py
from backtester.models import MarketImpactModel

def test_market_impact_scaling():
    model = MarketImpactModel()
    impact_10M = model.estimate_impact(order_size=1e6, aum=10e6)
    impact_1B = model.estimate_impact(order_size=1e6, aum=1e9)
    assert impact_1B > impact_10M