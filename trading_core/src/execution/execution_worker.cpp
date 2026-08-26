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
    ExecutionWorker::ExecutionWorker(concurrency::Queue<OrderRequest>& orderQueue,
                                     OrderManager& orderManager):
        orderQueue { orderQueue },
        orderManager { orderManager }
    {
    }

    void ExecutionWorker::run() const
    {
        OrderRequest request {};
        while (orderQueue.waitPop(request)) {
            orderManager.createOrder(request);
        }
    }
}