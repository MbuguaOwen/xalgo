#!/usr/bin/env python3
"""
InternalSimulator.py

A production-ready market simulator for order execution and strategy performance tracking.
Designed with low-latency, thread safety, and comprehensive error handling in mind.
"""

import time
import threading
from collections import deque
from dataclasses import dataclass, field
from typing import List, Dict, Optional
import logging
from typing import List, Dict, Any

from market_replay_engine import MarketReplayEngine
from synthetic_order_book import SyntheticOrderBook
from latency_emulator import LatencyEmulator
from scenario_designer import ScenarioDesigner

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)


###############################################################################
# Data Structures for Orders, Trades, and Position Tracking
###############################################################################

@dataclass
class Order:
    id: int
    symbol: str
    side: str           # 'buy' or 'sell'
    qty: float
    price: float        # limit price for the order
    timestamp: int      # nanoseconds timestamp of order submission
    latency_ns: int     # simulated network latency in nanoseconds
    filled: float = 0.0
    status: str = "pending"   # 'pending', 'filled', 'cancelled'


@dataclass
class Trade:
    symbol: str
    side: str           # 'buy' or 'sell'
    qty: float
    price: float        # executed price after slippage adjustment
    timestamp: int      # nanoseconds of execution


@dataclass
class Position:
    symbol: str
    qty: float = 0.0
    avg_price: float = 0.0
    realized_pnl: float = 0.0
    unrealized_pnl: float = 0.0

    def update_on_fill(self, trade: Trade) -> None:
        """
        Update position metrics based on the executed trade.
        """
        signed_qty = trade.qty if trade.side == 'buy' else -trade.qty
        new_qty = self.qty + signed_qty

        # Set avg_price on a fresh position
        if self.qty == 0:
            self.avg_price = trade.price
        # Update average price if position is scaled in the same direction
        elif (self.qty > 0 and signed_qty > 0) or (self.qty < 0 and signed_qty < 0):
            self.avg_price = ((self.avg_price * abs(self.qty)) + (trade.price * abs(signed_qty))) / abs(new_qty)

        # Calculate realized PnL for position closure or reduction
        if (self.qty > 0 > new_qty) or (self.qty < 0 < new_qty) or new_qty == 0:
            self.realized_pnl += (trade.price - self.avg_price) * -self.qty

        self.qty = new_qty


###############################################################################
# CombinedSimulator Class: Order Execution & Strategy Performance
###############################################################################

class CombinedSimulator:
    def __init__(
        self,
        initial_balance: float,
        slippage_bps: float = 0.5,
        simulate_latency_ns: int = 1_000_000,
        commission: float = 0.00005,
        risk_per_trade: float = 0.01
    ):
        # Order-level components
        self.order_id_counter = 1
        self.orders: Dict[int, Order] = {}
        self.pending_orders: deque[Order] = deque()
        self.executed_trades: List[Trade] = []
        self.positions: Dict[str, Position] = {}
        self.slippage_bps = slippage_bps
        self.simulate_latency_ns = simulate_latency_ns
        self.current_time_ns: int = 0

        # Performance tracking components
        self.balance = initial_balance
        self.initial_balance = initial_balance
        self.equity_curve: List[float] = [initial_balance]
        self.trade_log: List[Dict] = []
        self.total_trades = 0
        self.total_wins = 0
        self.total_losses = 0
        self.commission = commission
        self.risk_per_trade = risk_per_trade

        # Thread-safety locks
        self.order_lock = threading.Lock()
        self.trade_lock = threading.Lock()

    ###########################################
    # Order Management & Execution Methods
    ###########################################

    def submit_order(
        self,
        symbol: str,
        side: str,
        qty: float,
        price: float,
        current_time_ns: Optional[int] = None
    ) -> int:
        """
        Submits an order with the provided parameters.
        Thread-safe method.
        """
        if current_time_ns is None:
            current_time_ns = int(time.time() * 1e9)
        with self.order_lock:
            order = Order(
                id=self.order_id_counter,
                symbol=symbol,
                side=side,
                qty=qty,
                price=price,
                timestamp=current_time_ns,
                latency_ns=self.simulate_latency_ns
            )
            self.orders[order.id] = order
            self.pending_orders.append(order)
            self.order_id_counter += 1
            return order.id

    def cancel_order(self, order_id: int) -> bool:
        """
        Cancel an order if it is still pending.
        Thread-safe method.
        """
        with self.order_lock:
            if order_id in self.orders:
                order = self.orders[order_id]
                if order.status == 'pending':
                    order.status = 'cancelled'
                    return True
        return False

    def _apply_slippage(self, price: float, side: str) -> float:
        """
        Apply slippage to the given price based on trade side.
        """
        try:
            slippage = price * (self.slippage_bps / 10_000)
            return price + slippage if side == 'buy' else price - slippage
        except Exception as e:
            raise ValueError(f"Error applying slippage: {e}")

    def _can_fill_order(self, order: Order, tick_price: float) -> bool:
        """
        Determine if the order can be filled given the current market tick.
        """
        if order.side == 'buy':
            return tick_price <= order.price
        else:
            return tick_price >= order.price

    def _fill_order(self, order: Order, tick_price: float) -> None:
        """
        Process the order fill: adjusts price for slippage, records the trade,
        and updates the corresponding position.
        """
        filled_price = self._apply_slippage(tick_price, order.side)
        trade = Trade(
            symbol=order.symbol,
            side=order.side,
            qty=order.qty,
            price=filled_price,
            timestamp=self.current_time_ns
        )
        # Record the trade (thread-safe)
        with self.trade_lock:
            self.executed_trades.append(trade)
        order.status = 'filled'
        order.filled = order.qty
        # Update position
        pos = self.positions.setdefault(order.symbol, Position(symbol=order.symbol))
        pos.update_on_fill(trade)
        # Log performance details for the trade
        self._log_trade(order, trade)

    def process_tick(self, symbol: str, price: float, timestamp_ns: int) -> None:
        """
        Process a market tick by updating current time and evaluating pending orders.
        Should be called by the market data feed handler.
        """
        self.current_time_ns = timestamp_ns
        # Process pending orders in a thread-safe manner
        with self.order_lock:
            while self.pending_orders:
                order = self.pending_orders[0]
                if self.current_time_ns - order.timestamp >= order.latency_ns:
                    if order.symbol == symbol and self._can_fill_order(order, price):
                        try:
                            self._fill_order(order, price)
                        except Exception as e:
                            # Log error, but continue processing subsequent orders.
                            print(f"Error processing order {order.id}: {e}")
                    # Remove order regardless of fill outcome
                    self.pending_orders.popleft()
                else:
                    break  # Next order not yet ready based on latency

    ###########################################
    # Capital Management & Risk Metrics Methods
    ###########################################

    def calculate_position_size(self) -> float:
        """
        Calculate the trade's position size based on the current balance and defined risk.
        """
        try:
            return self.balance * self.risk_per_trade
        except Exception as e:
            raise ValueError(f"Error calculating position size: {e}")

    def _log_trade(self, order: Order, trade: Trade) -> None:
        """
        Log trade details and update the performance metrics.
        Uses a simulated exit price to calculate profit/loss.
        """
        position_size = self.calculate_position_size()
        # Simulate immediate exit (for performance reporting) with a slight perturbation
        simulated_exit_price = trade.price * (1.001 if order.side == 'buy' else 0.999)
        entry_cost = trade.price * position_size * (1 + self.slippage_bps / 10_000) + self.commission
        exit_revenue = simulated_exit_price * position_size * (1 - self.slippage_bps / 10_000) - self.commission
        pnl = exit_revenue - entry_cost

        # Update balance and equity curve in a thread-safe manner
        with self.trade_lock:
            self.balance += pnl
            self.equity_curve.append(self.balance)

            trade_details = {
                'order_id': order.id,
                'symbol': order.symbol,
                'side': order.side,
                'entry_price': trade.price,
                'exit_price': simulated_exit_price,
                'pnl': pnl,
                'balance_after_trade': self.balance,
                'timestamp': trade.timestamp
            }
            self.trade_log.append(trade_details)
            self.total_trades += 1
            if pnl > 0:
                self.total_wins += 1
            else:
                self.total_losses += 1

    def generate_report(self) -> None:
        """
        Print out a comprehensive performance report.
        """
        with self.trade_lock:
            win_rate = (self.total_wins / self.total_trades) if self.total_trades else 0.0
            loss_rate = (self.total_losses / self.total_trades) if self.total_trades else 0.0
            profit_factor = (self.total_wins / self.total_losses) if self.total_losses > 0 else float('inf')

            report = (
                "=== PERFORMANCE REPORT ===\n"
                f"Initial Balance: {self.initial_balance:.2f}\n"
                f"Final Balance: {self.balance:.2f}\n"
                f"Total Trades: {self.total_trades}\n"
                f"Win Rate: {win_rate * 100:.2f}%\n"
                f"Loss Rate: {loss_rate * 100:.2f}%\n"
                f"Profit Factor: {profit_factor:.2f}\n"
                "=========================="
            )
            print(report)

    ###########################################
    # Strategy Simulation Wrapper
    ###########################################

    def simulate_strategy(self, market_data: List[Dict]) -> None:
        """
        Run the simulation given a list of market ticks.
        Each tick is a dict with keys: 'symbol', 'price', 'timestamp_ns'
        """
        for tick in market_data:
            try:
                self.process_tick(tick['symbol'], tick['price'], tick['timestamp_ns'])
            except Exception as e:
                print(f"Error processing tick {tick}: {e}")

        # Generate performance report after processing market data
        self.generate_report()


###############################################################################
# Main Entry Point (Example Usage)
###############################################################################

if __name__ == "__main__":
    try:
        # Initialize the simulator with a starting balance of 100 million dollars
        simulator = CombinedSimulator(
            initial_balance=100e6,
            slippage_bps=0.5,
            simulate_latency_ns=1_000_000,
            commission=50,           # Fixed commission per trade
            risk_per_trade=0.01
        )

        # Submit an order (for example purposes)
        now_ns = int(time.time() * 1e9)
        order_id = simulator.submit_order(symbol="AAPL", side="buy", qty=1000, price=150.00, current_time_ns=now_ns)
        print(f"Submitted order {order_id}")

        # Simulated market ticks (would come from live market data in production)
        simulated_ticks = [
            {'symbol': "AAPL", 'price': 149.95, 'timestamp_ns': now_ns + 1_500_000},
            {'symbol': "AAPL", 'price': 150.05, 'timestamp_ns': now_ns + 3_000_000},
            {'symbol': "AAPL", 'price': 150.10, 'timestamp_ns': now_ns + 4_500_000},
        ]
        
        # Run the simulation with provided market data
        simulator.simulate_strategy(simulated_ticks)
    
    except Exception as e:
        # Comprehensive error handling at the top level
        print(f"Critical error in simulation: {e}")

class InternalSimulator:
    """
    Central simulation orchestrator that integrates:
      - Market replay engine (historical data playback)
      - Synthetic order book generator (simulate order book updates)
      - Latency emulator (simulate processing/network delays)
      - Custom scenario designer (apply user-defined simulation scenarios)
      
    Provides a unified API to run simulation cycles.
    """
    
    def __init__(self, market_data: List[Dict[str, Any]]):
        self.market_replay_engine = MarketReplayEngine(market_data)
        self.order_book = SyntheticOrderBook()
        self.latency_emulator = LatencyEmulator()
        self.scenario_designer = ScenarioDesigner()
        logger.info("Internal Simulator initialized.")
    
    def register_scenario(self, name: str, simulation_function):
        """
        Register a custom scenario with the simulator.
        """
        self.scenario_designer.register_scenario(name, simulation_function)
    def register_dynamic_scenario(self, name: str, trigger_function: Callable[[Dict[str, Any]], bool],
                                  simulation_function: Callable[[Dict[str, Any]], Dict[str, Any]]):
        """
        Register a dynamic simulation scenario with trigger condition.

        Args:
            name: Name of the dynamic scenario.
            trigger_function: Function that evaluates if the scenario should trigger.
            simulation_function: Function that transforms market data when trigger is true.
        """
        self.scenario_designer.register_dynamic_scenario(name, trigger_function, simulation_function)
    
    def run_simulation(self, scenario: str = None):
        """
        Run a single simulation cycle:
          - Replay market data
          - Optionally apply a custom scenario to the market data
          - Update the synthetic order book
          - Emulate processing latency
          
        Args:
            scenario: Name of the registered scenario to apply.
        """
        logger.info("Starting simulation cycle.")
        for market_data in self.market_replay_engine.replay():
            # Optionally modify the data via a custom scenario.
            if scenario:
                market_data = self.scenario_designer.apply_scenario(scenario, market_data)
            
            # Update the synthetic order book.
            self.order_book.update(market_data)
            
            # Emulate system latency.
            self.latency_emulator.emulate_latency()
            
            # For demonstration, we log the current order book state.
            current_state = self.order_book.get_order_book()
            logger.info("Current Order Book State: %s", current_state)
        
        logger.info("Simulation cycle completed.")

# Example usage for testing:
if __name__ == "__main__":
    # Sample historical market data
    sample_data = [
        {"timestamp": 1.0, "bids": [100, 99], "asks": [101, 102]},
        {"timestamp": 2.0, "bids": [101, 100], "asks": [102, 103]},
        {"timestamp": 3.0, "bids": [102, 101], "asks": [103, 104]}
    ]
    
    simulator = InternalSimulator(sample_data)
    
    # Register a custom scenario (e.g., a liquidity shock that drops bids)
    def liquidity_shock(data: Dict[str, Any]) -> Dict[str, Any]:
        data["bids"] = [bid * 0.9 for bid in data.get("bids", [])]
        return data
    
    simulator.register_scenario("liquidity_shock", liquidity_shock)
    
    # Run simulation with the custom scenario applied
    simulator.run_simulation(scenario="liquidity_shock")
    
     # Register a dynamic scenario that triggers when ask prices exceed a threshold.
    def high_ask_trigger(data: Dict[str, Any]) -> bool:
        asks = data.get("asks", [])
        return any(ask > 103 for ask in asks)
    
    def adjust_for_high_asks(data: Dict[str, Any]) -> Dict[str, Any]:
        # Example: Slightly shift ask prices downward to simulate market correction.
        data["asks"] = [ask * 0.98 for ask in data.get("asks", [])]
        return data
    simulator.register_dynamic_scenario("high_ask_correction", high_ask_trigger, adjust_for_high_asks)
    
    # Run simulation with a static scenario applied, plus dynamic scenario evaluation.
    simulator.run_simulation(static_scenario="liquidity_shock")