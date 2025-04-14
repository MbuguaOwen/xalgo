class ISignalGenerator {
    public:
        virtual Signal generate(const MarketData& data) = 0;
        virtual void updateParameters(const MarketRegime& regime) = 0;
    };
    