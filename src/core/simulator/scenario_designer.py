# project-root/src/core/simulator/scenario_designer.py

import logging
from typing import Dict, Any, Callable

logger = logging.getLogger(__name__)

class ScenarioDesigner:
    """
    Provides tools to create custom simulation scenarios.
    Scenarios could include sudden market changes, liquidity shocks, or other
    user-defined events to stress test the system.
    
    Attributes:
        scenarios (Dict[str, Callable]): A dictionary mapping scenario names to simulation functions.
    """
    
    def __init__(self):
        self.scenarios = {}
        logger.info("Scenario Designer initialized.")
    
    def register_scenario(self, name: str, simulation_function: Callable[[Dict[str, Any]], Dict[str, Any]]):
        """
        Register a new custom scenario.
        
        Args:
            name: The name of the scenario.
            simulation_function: A function that takes a market data record and returns a modified record.
        """
        self.scenarios[name] = simulation_function
        logger.info("Registered custom scenario: %s", name)
    
    def apply_scenario(self, name: str, market_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Apply a registered custom scenario to a market data record.
        
        Args:
            name: The name of the registered scenario.
            market_data: The market data record to modify.
        
        Returns:
            Modified market data record after scenario application.
        """
        if name not in self.scenarios:
            logger.error("Scenario %s not found.", name)
            raise ValueError(f"Scenario {name} not registered.")
        
        logger.debug("Applying scenario %s to market data", name)
        return self.scenarios[name](market_data)
