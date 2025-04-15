import logging
from typing import Dict, Any, Callable, Optional

logger = logging.getLogger(__name__)

class ScenarioDesigner:
    """
    Provides capabilities to register and apply both static and dynamic simulation scenarios.

    Static scenarios modify every market data record when applied by name.
    Dynamic scenarios are evaluated per-record: a trigger function checks market data and,
    if true, applies the scenario function.

    Attributes:
        static_scenarios (Dict[str, Callable]): Mapping of scenario names to simulation functions.
        dynamic_scenarios (Dict[str, Dict[str, Callable]]): Mapping of scenario names to a dictionary with:
            'trigger': a function (market_data -> bool) that determines when to apply the scenario.
            'scenario': a simulation function (market_data -> market_data).
    """
    def __init__(self):
        self.static_scenarios: Dict[str, Callable[[Dict[str, Any]], Dict[str, Any]]] = {}
        self.dynamic_scenarios: Dict[str, Dict[str, Callable]] = {}
        logger.info("Scenario Designer initialized.")
    
    def register_scenario(self, name: str, simulation_function: Callable[[Dict[str, Any]], Dict[str, Any]]):
        """
        Register a static simulation scenario.

        Args:
            name: Unique identifier for the scenario.
            simulation_function: Function to modify a market data record.
        """
        self.static_scenarios[name] = simulation_function
        logger.info("Registered static scenario: %s", name)
    
    def register_dynamic_scenario(self, name: str, trigger_function: Callable[[Dict[str, Any]], bool],
                                  simulation_function: Callable[[Dict[str, Any]], Dict[str, Any]]):
        """
        Register a dynamic simulation scenario that is applied only when its trigger condition is met.

        Args:
            name: Unique identifier for the scenario.
            trigger_function: Function that, given market data, returns True if the scenario should be applied.
            simulation_function: Function to modify the market data record when trigger is True.
        """
        self.dynamic_scenarios[name] = {
            "trigger": trigger_function,
            "scenario": simulation_function,
        }
        logger.info("Registered dynamic scenario: %s", name)
    
    def apply_scenario(self, name: str, market_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Apply a registered static scenario to modify market data.

        Args:
            name: The scenario's name.
            market_data: The data record to transform.

        Returns:
            Modified market data record.
        """
        if name not in self.static_scenarios:
            logger.error("Static scenario %s not found.", name)
            raise ValueError(f"Scenario {name} not registered.")
        
        logger.debug("Applying static scenario %s to market data", name)
        return self.static_scenarios[name](market_data)
    
    def apply_dynamic_scenarios(self, market_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Evaluate and apply all registered dynamic scenarios if their triggers are met.

        Args:
            market_data: The current market data record.

        Returns:
            Modified market data after applying all triggered dynamic scenarios.
        """
        for name, funcs in self.dynamic_scenarios.items():
            trigger = funcs["trigger"]
            scenario = funcs["scenario"]
            if trigger(market_data):
                logger.debug("Dynamic scenario %s triggered; applying changes.", name)
                market_data = scenario(market_data)
        return market_data
