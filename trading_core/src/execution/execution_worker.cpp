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
    ExecutionWorker::ExecutionWorker(concurrency::Queue<ExecutionWorkItem>& executionQueue,
                                     OrderManager& orderManager):
        executionQueue { executionQueue },
        orderManager { orderManager }
    {
    }

    void ExecutionWorker::run() const
    {
        ExecutionWorkItem workItem;
        while (executionQueue.waitPop(workItem))
        {
            if (const OrderRequest* request = std::get_if<OrderRequest>(&workItem))
            {
                // FIXME
                const auto _  = orderManager.createOrder(*request);
            }
        }
    }
}