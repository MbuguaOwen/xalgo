# project-root/src/core/simulator/market_replay_engine.py

import logging
import time
from typing import List, Dict, Any

logger = logging.getLogger(__name__)

class MarketReplayEngine:
    """
    Handles the replay of historical market data.
    
    Attributes:
        data (List[Dict[str, Any]]): Historical market data records.
        playback_speed (float): Speed multiplier for replaying data.
    """
    
    def __init__(self, data: List[Dict[str, Any]], playback_speed: float = 1.0):
        """
        Initialize the MarketReplayEngine.

        Args:
            data: A list of market data records.
            playback_speed: Multiplier to adjust replay speed.
        """
        self.data = data
        self.playback_speed = playback_speed
        logger.info("MarketReplayEngine initialized with %d records", len(data))
    
    def replay(self):
        """
        Replays market data sequentially. Yield each market data record,
        respecting the playback speed to simulate real-time flow.
        """
        previous_time = None
        for record in self.data:
            timestamp = record.get("timestamp")
            if previous_time is not None:
                # Calculate delay based on historical timestamps
                delay = (timestamp - previous_time) / self.playback_speed
                logger.debug("Replaying record with delay: %fs", delay)
                time.sleep(max(delay, 0))
            previous_time = timestamp
            logger.debug("Yielding market data record: %s", record)
            yield record
