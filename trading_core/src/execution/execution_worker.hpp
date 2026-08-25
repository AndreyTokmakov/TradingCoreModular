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

#include "execution_gateway.hpp"
#include "order.hpp"
#include "queue.hpp"

#include <thread>

namespace trading::execution
{
    class ExecutionWorker final
    {
    public:
        ExecutionWorker(IExecutionGateway& gateway,
                        concurrency::Queue<Order>& queue);

        ~ExecutionWorker();

        ExecutionWorker(const ExecutionWorker&) = delete;
        ExecutionWorker& operator=(const ExecutionWorker&) = delete;

        ExecutionWorker(ExecutionWorker&&) = delete;
        ExecutionWorker& operator=(ExecutionWorker&&) = delete;

        void start();

        void stop() noexcept;

    private:
        void run() const;

        IExecutionGateway& gateway;
        concurrency::Queue<Order>& queue;

        std::jthread worker;
        bool running { false };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EXECUTION_WORKER_HPP