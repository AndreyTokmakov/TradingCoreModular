```text
CPU 0
┌──────────────────────────────┐
│ T1 Market Data RX + Parser   │
└──────────────┬───────────────┘
               │
               ▼
CPU 1
┌──────────────────────────────┐
│ T2 BookBuilder               │
│    Market Data State         │
└──────────────┬───────────────┘
               │
               ▼
CPU 2
┌──────────────────────────────┐
│ T3 Strategy + RiskManager    │
│    → OrderCommand            │
└──────────────┬───────────────┘
               │
               ▼
CPU 3
┌──────────────────────────────┐
│ T4 Execution Gateway         │
│    Order → Exchange          │
│    Exchange → Execution      │
└──────────────────────────────┘


CPU 4
┌──────────────────────────────┐
│ T5 Recording / Audit         │
└──────────────────────────────┘

CPU 5
┌──────────────────────────────┐
│ T6 Logging / Metrics         │
└──────────────────────────────┘
```


```text
          CRITICAL PATH

CPU 0             CPU 1                 CPU 2
┌─────────┐       ┌────────────────┐    ┌──────────────┐
│ T1      │       │ T2             │    │ T3           │
│ RX      │──────→│ BookBuilder    │───→│ Execution    │
│ Parser  │       │ Strategy       │    │ Gateway      │
└─────────┘       │ RiskManager    │    └──────────────┘
                  └────────────────┘


          OFF CRITICAL PATH

CPU 3             CPU 4                 CPU 5
┌─────────┐       ┌─────────┐           ┌─────────┐
│ T4      │       │ T5      │           │ T6      │
│ Record  │       │ Logging │           │ Metrics │
└─────────┘       └─────────┘           └─────────┘
```

```text
             ┌─────────────┐
Exchange ───►│ Market Data │
             │ RX + Parser │
             └──────┬──────┘
                    │
                    ▼
             ┌─────────────┐
             │ Trading     │
             │ Engine      │
             │             │
             │ Book        │
             │ Strategy    │
             │ Risk        │
             └──────┬──────┘
                    │
                    ▼
             ┌─────────────┐
             │ Execution   │
             │ Gateway     │
             └─────────────┘
```
