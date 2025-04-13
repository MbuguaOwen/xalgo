class InternalSimulator:
    def __init__(self, initial_balance, slippage=0.0001, commission=0.00005):
        self.balance = initial_balance
        self.equity_curve = [initial_balance]  # Keeps track of the equity curve over time
        self.open_positions = []
        self.total_trades = 0
        self.total_wins = 0
        self.total_losses = 0
        self.slippage = slippage
        self.commission = commission
        self.trade_log = []

    def execute_trade(self, trade_signal, entry_price, exit_price):
        """
        Simulate trade execution with slippage and commissions.
        """
        position_size = self.calculate_position_size()
        entry_cost = entry_price * position_size * (1 + self.slippage) + self.commission
        exit_revenue = exit_price * position_size * (1 - self.slippage) - self.commission
        
        # Track the position
        self.open_positions.append({
            'entry_price': entry_price,
            'exit_price': exit_price,
            'position_size': position_size,
            'profit_loss': exit_revenue - entry_cost
        })
        
        # Update balance
        self.balance += exit_revenue - entry_cost
        self.equity_curve.append(self.balance)

        # Log the trade
        self.log_trade(trade_signal, entry_price, exit_price, self.balance)

    def calculate_position_size(self):
        """
        Calculate position size based on account balance and predefined risk parameters.
        """
        risk_per_trade = 0.01  # Risk 1% of account balance per trade
        position_size = self.balance * risk_per_trade
        return position_size

    def log_trade(self, trade_signal, entry_price, exit_price, current_balance):
        """
        Log detailed trade information for reporting.
        """
        trade = {
            'trade_signal': trade_signal,
            'entry_price': entry_price,
            'exit_price': exit_price,
            'final_balance': current_balance,
        }
        self.trade_log.append(trade)

        if trade['exit_price'] > trade['entry_price']:  # Win
            self.total_wins += 1
        else:  # Loss
            self.total_losses += 1

        self.total_trades += 1

    def generate_report(self):
        """
        Generate a performance report for strategy analysis.
        """
        win_rate = self.total_wins / self.total_trades
        loss_rate = self.total_losses / self.total_trades
        profit_factor = self.total_wins / (self.total_losses if self.total_losses > 0 else 1)
        
        print(f"Total Trades: {self.total_trades}")
        print(f"Win Rate: {win_rate * 100:.2f}%")
        print(f"Loss Rate: {loss_rate * 100:.2f}%")
        print(f"Profit Factor: {profit_factor:.2f}")
        print(f"Final Balance: {self.balance:.2f}")
        # More metrics like Sharpe ratio can be added here

    def simulate(self, signals):
        """
        Simulate the trading strategy given the trade signals (entry/exit times and prices).
        """
        for signal in signals:
            if signal['type'] == 'entry':
                entry_price = signal['price']
                trade_signal = 'BUY'  # Example, can be SELL based on signal type
            elif signal['type'] == 'exit':
                exit_price = signal['price']
                self.execute_trade(trade_signal, entry_price, exit_price)
                
        self.generate_report()
