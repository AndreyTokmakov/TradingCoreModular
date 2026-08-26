/**============================================================================
Name        : execution_worker.hpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Sends orders to the exchange on the execution thread.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_EXECUTION_WORKER_HPP
#define FINANCETECHNOLOGYPROJECTS_EXECUTION_WORKER_HPP

#include "order_manager.hpp"
#include "queue.hpp"

#include <thread>

namespace trading::execution
{
    class ExecutionWorker final
    {
    public:
        ExecutionWorker(concurrency::Queue<OrderRequest>& orderQueue,
                        OrderManager& orderManager);

        ~ExecutionWorker();

        ExecutionWorker(const ExecutionWorker&) = delete;
        ExecutionWorker& operator=(const ExecutionWorker&) = delete;

        ExecutionWorker(ExecutionWorker&&) = delete;
        ExecutionWorker& operator=(ExecutionWorker&&) = delete;

        void start();

        void stop() noexcept;

    private:
        void run() const;

        concurrency::Queue<OrderRequest>& orderQueue;
        OrderManager& orderManager;

        std::jthread worker;
        bool running { false };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EXECUTION_WORKER_HPP