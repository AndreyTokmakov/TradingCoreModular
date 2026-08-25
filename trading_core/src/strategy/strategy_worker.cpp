/**============================================================================
Name        : strategy_worker.cpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Executes strategy processing on the strategy thread.
============================================================================**/

#include "strategy_worker.hpp"

namespace trading::strategy
{
    StrategyWorker::StrategyWorker(IStrategy& strategy,
                                   StrategyExecutor& executor,
                                   concurrency::Queue<market_data::MarketEvent>& queue):
        strategy { strategy },
        executor { executor },
        queue { queue }
    {
    }

    StrategyWorker::~StrategyWorker()
    {
        stop();
    }

    void StrategyWorker::start()
    {
        if (running)
            return;
        running = true;
        worker = std::jthread { &StrategyWorker::run, this };
    }

    void StrategyWorker::stop() noexcept
    {
        if (!running)
            return;
        queue.close();
        if (worker.joinable())
            worker.join();
        running = false;
    }

    void StrategyWorker::run() const
    {
        market_data::MarketEvent event;

        while (queue.waitPop(event))
        {
            const Signal signal = strategy.evaluate(event);
            const StrategyExecutionResult result = executor.execute(signal, event);

            if (!result)
            {
                // TODO: log / metrics / risk event.
                continue;
            }

            if (!result->has_value())
                continue;

            // TODO: order-created event / metrics.
        }
    }
}