# File: tests/test_performance.py

import asyncio
import time
import uvloop
import statistics
from loguru import logger
from src.core.simulator.InternalSimulator import InternalSimulator
from src.core.router.SmartOrderRouter import SmartOrderRouter
from src.core.models.order import Order
from src.core.models.venue import Venue

asyncio.set_event_loop_policy(uvloop.EventLoopPolicy())  # High-performance event loop


async def measure_router_latency(router, order, iterations=1000):
    latencies = []
    for _ in range(iterations):
        start_ns = time.perf_counter_ns()
        await router.route(order)
        end_ns = time.perf_counter_ns()
        latency_us = (end_ns - start_ns) / 1_000  # Convert to microseconds
        latencies.append(latency_us)
    return latencies


def log_latency_stats(latencies, label="Router"):
    logger.info(f"[{label}] Executed {len(latencies)} orders")
    logger.info(f"Min latency:     {min(latencies):.2f} μs")
    logger.info(f"Max latency:     {max(latencies):.2f} μs")
    logger.info(f"Mean latency:    {statistics.mean(latencies):.2f} μs")
    logger.info(f"Median latency:  {statistics.median(latencies):.2f} μs")
    logger.info(f"99th percentile: {statistics.quantiles(latencies, n=100)[98]:.2f} μs")


async def main():
    # Set up venues (simulating colocated exchanges)
    venues = [
        Venue(name="LD4-LMAX", latency=35.0, reliability=0.997),
        Venue(name="LD4-EBS", latency=40.0, reliability=0.996),
        Venue(name="LD4-Currenex", latency=45.0, reliability=0.998)
    ]

    # Create the SmartOrderRouter instance
    router = SmartOrderRouter(venues)

    # Simulate a basic order
    test_order = Order(symbol="EUR/USD", price=1.1234, quantity=1_000_000, side="buy")

    # Run latency benchmark
    logger.info("Starting latency benchmark for SmartOrderRouter")
    latencies = await measure_router_latency(router, test_order, iterations=1000)
    log_latency_stats(latencies, label="SmartOrderRouter")

    # Assert average latency is within HFT constraint (< 100μs)
    assert statistics.mean(latencies) < 100.0, "Router latency exceeds 100μs budget"


if __name__ == "__main__":
    asyncio.run(main())
