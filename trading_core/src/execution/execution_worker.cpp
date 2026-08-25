/**============================================================================
Name        : execution_worker.cpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Sends orders to the exchange on the execution thread.
============================================================================**/

#include "execution_worker.hpp"

namespace trading::execution
{
    ExecutionWorker::ExecutionWorker(IExecutionGateway& gateway,
                                     concurrency::Queue<Order>& queue):
        gateway { gateway },
        queue { queue }
    {
    }

    ExecutionWorker::~ExecutionWorker()
    {
        stop();
    }

    void ExecutionWorker::start()
    {
        if (running)
            return;

        running = true;
        worker = std::jthread { &ExecutionWorker::run, this };
    }

    void ExecutionWorker::stop() noexcept
    {
        if (!running)
            return;
        queue.close();
        if (worker.joinable())
            worker.join();
        running = false;
    }

    void ExecutionWorker::run() const
    {
        Order order;
        while (queue.waitPop(order)) {
            gateway.send(order);
        }
    }
}