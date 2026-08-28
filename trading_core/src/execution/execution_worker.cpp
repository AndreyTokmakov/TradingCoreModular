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
                                    OrderManager& orderManager,
                                    recording::IRecorder& recorder) noexcept :
        executionQueue { executionQueue },
        orderManager { orderManager },
        recorder { recorder }
    {
    }

    void ExecutionWorker::run() const
    {
        ExecutionWorkItem workItem;
        while (executionQueue.waitPop(workItem))
        {
            std::visit([this](const auto& item) {
                process(item);
            },workItem);
        }
    }

    void ExecutionWorker::process(const OrderRequest& request) const
    {
        [[maybe_unused]]
        const OrderCreationResult result = orderManager.createOrder(request);

        // TODO: Handle order creation errors: logging / metrics / risk event.
    }

    void ExecutionWorker::process(const ExecutionReport& report) const
    {
        recorder.record(report);

        [[maybe_unused]]
        const bool processed = orderManager.applyExecution(report);
        // TODO: Handle unknown orders or invalid execution reports.
    }
}