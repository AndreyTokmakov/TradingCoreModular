/**============================================================================
Name        : strategy_worker.hpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Executes strategy processing on the strategy thread.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_STRATEGY_WORKER_HPP
#define FINANCETECHNOLOGYPROJECTS_STRATEGY_WORKER_HPP

#include "model/market_event.hpp"
#include "queue.hpp"
#include "strategy.hpp"
#include "strategy_executor.hpp"

#include <thread>

namespace trading::strategy
{
    class StrategyWorker final
    {
    public:
        StrategyWorker(concurrency::Queue<market_data::MarketEvent>& marketEventQueue,
                          IStrategy& strategy,
                          StrategyExecutor& executor) noexcept;

        ~StrategyWorker();

        StrategyWorker(const StrategyWorker&) = delete;
        StrategyWorker& operator=(const StrategyWorker&) = delete;

        StrategyWorker(StrategyWorker&&) = delete;
        StrategyWorker& operator=(StrategyWorker&&) = delete;

        void start();
        void stop() noexcept;

    private:
        void run() const;

        concurrency::Queue<market_data::MarketEvent>& marketEventQueue;
        IStrategy& strategy;
        StrategyExecutor& executor;

        std::jthread worker;
        bool running { false };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_STRATEGY_WORKER_HPP