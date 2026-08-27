/**============================================================================
Name        : book_builder_worker.hpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Processes market-data book updates on the BookBuilder thread.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_BOOK_BUILDER_WORKER_HPP
#define FINANCETECHNOLOGYPROJECTS_BOOK_BUILDER_WORKER_HPP

#include "book_builder.hpp"
#include "interfaces/market_data_parser.hpp"
#include "interfaces/snapshot_provider.hpp"
#include "queue.hpp"
#include "worker.hpp"

namespace trading::market_data
{
    class BookBuilderWorker final: public common::Worker<BookBuilderWorker>
    {
    public:
        BookBuilderWorker(BookBuilder& bookBuilder,
                          ISnapshotProvider& snapshotProvider,
                          concurrency::Queue<BookUpdates>& queue);

        void run() const;

    private:

        BookBuilder& bookBuilder;
        ISnapshotProvider& snapshotProvider;
        concurrency::Queue<BookUpdates>& queue;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BOOK_BUILDER_WORKER_HPP