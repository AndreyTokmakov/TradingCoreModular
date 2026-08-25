
```text
                       ┌──────────────────────┐
                       │       Strategy       │
                       └──────────┬───────────┘
                                  │
                           OrderRequest
                                  │
                                  ▼
                       ┌──────────────────────┐
                       │    OrderManager      │
                       │                      │
                       │  - validate risk     │
                       │  - create Order      │
                       │  - track Order       │
                       └──────────┬───────────┘
                                  │
                                Order
                                  │
                                  ▼
                       ┌──────────────────────┐
                       │ IExecutionGateway    │
                       └──────────┬───────────┘
                                  │
                                  ▼
                       ┌──────────────────────┐
                       │ BinanceExecution     │
                       │ Gateway              │
                       └──────────┬───────────┘
                                  │
                                  ▼
                              Binance
                                  │
                         execution message
                                  │
                                  ▼
                       ┌──────────────────────┐
                       │ Binance Execution    │
                       │ Adapter / Parser     │
                       └──────────┬───────────┘
                                  │
                            ExecutionReport
                                  │
                                  ▼
                 ┌────────────────────────────────┐
                 │  ExecutionReportHandler        │
                 └─────────┬──────────┬───────────┘
                           │          │
                           │          │
                           ▼          ▼
                    OrderManager   PositionManager
                           │          │
                           ▼          ▼
                         Order     Position
                           │          │
                           └────┬─────┘
                                │
                                ▼
                            Recorder
```
