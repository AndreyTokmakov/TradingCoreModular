### Шаг 1

Сделать базовый `metrics`:

```text
Counter
MetricsRegistry
```

Без histogram/gauge пока.

### Шаг 2

Добавить counters в:

```text
MarketDataSource
MarketDataMessageHandler
BookBuilder
StrategyWorker
ExecutionWorker
OrderManager
RecordingWorker
```

### Шаг 3

Внедрить существующий `ILogger`:

```text
Application
Config loader
BinanceMarketDataSource
BinanceExecutionGateway
BinanceExecutionReportSource
```

### Шаг 4

Сделать полноценный `MockExchange`.

Это, на мой взгляд, **самый важный шаг перед Binance**.

### Шаг 5

Написать integration test:

```text
MarketData
    ↓
Strategy signal
    ↓
Order
    ↓
MockExchange
    ↓
ExecutionReport
    ↓
Position
    ↓
PnL
```

То есть проверить настоящий end-to-end flow.

### Шаг 6

Подключить Binance Testnet.

---