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
#include "worker.hpp"

namespace trading::execution
{
    class ExecutionWorker final: public common::Worker<ExecutionWorker>
    {
    public:
        ExecutionWorker(concurrency::Queue<OrderRequest>& orderQueue,
                        OrderManager& orderManager);

        void run() const;

    private:

        concurrency::Queue<OrderRequest>& orderQueue;
        OrderManager& orderManager;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EXECUTION_WORKER_HPP