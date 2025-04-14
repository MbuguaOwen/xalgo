# tests/unit/test_market_impact_model.py
import pytest
from backtester.models import MarketImpactModel

@pytest.mark.unit
def test_market_impact_model_scaling_behavior():
    model = MarketImpactModel()
    impact_small_aum = model.estimate_impact(order_size=1e6, aum=10e6)
    impact_large_aum = model.estimate_impact(order_size=1e6, aum=1e9)

    assert impact_large_aum < impact_small_aum, (
        f"Expected impact to decrease with larger AUM. Got {impact_large_aum:.4f} >= {impact_small_aum:.4f}"
    )
    assert impact_small_aum > 0.0, "Impact should be positive for valid input"
