/**============================================================================
Name        : book_builder_worker.cpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Processes market-data book updates on the BookBuilder thread.
============================================================================**/

#include "book_builder_worker.hpp"

namespace trading::market_data
{
    BookBuilderWorker::BookBuilderWorker(BookBuilder& bookBuilder,
                                         ISnapshotProvider& snapshotProvider,
                                         concurrency::Queue<BookUpdates>& queue):
        bookBuilder { bookBuilder },
        snapshotProvider { snapshotProvider },
        queue { queue }
    {
    }

    void BookBuilderWorker::run() const
    {
        const Snapshot snapshot = snapshotProvider.getSnapshot();
        if (!bookBuilder.applySnapshot(snapshot))
            return;

        BookUpdates updates;
        while (queue.waitPop(updates))
        {
            for (const BookUpdate& update : updates) {
                bookBuilder.onBookUpdate(update);
            }
            updates.clear();
        }
    }
}