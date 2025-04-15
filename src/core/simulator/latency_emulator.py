# project-root/src/core/simulator/latency_emulator.py

import logging
import time
import random

logger = logging.getLogger(__name__)

class LatencyEmulator:
    """
    Emulates latency for stress testing the simulation, allowing for injection of
    random delays to mimic network or system latency spikes.
    
    Attributes:
        base_latency (float): The base latency in seconds.
        jitter (float): Maximum additional random delay.
    """
    
    def __init__(self, base_latency: float = 0.001, jitter: float = 0.005):
        """
        Initialize the latency emulator with base latency and jitter.
        
        Args:
            base_latency: Base delay in seconds.
            jitter: Maximum jitter in seconds to add randomness.
        """
        self.base_latency = base_latency
        self.jitter = jitter
        logger.info("Latency Emulator initialized with base latency: %fs and jitter: %fs", 
                    base_latency, jitter)
    
    def emulate_latency(self):
        """
        Applies a delay to emulate latency in processing.
        """
        delay = self.base_latency + random.uniform(0, self.jitter)
        logger.debug("Emulating latency delay: %fs", delay)
        time.sleep(delay)
