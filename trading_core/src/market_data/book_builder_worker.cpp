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
                                         concurrency::Queue<BookUpdates>& queue):
        bookBuilder { bookBuilder },
        queue { queue }
    {
    }

    BookBuilderWorker::~BookBuilderWorker()
    {
        stop();
    }

    void BookBuilderWorker::start()
    {
        if (running)
            return;

        running = true;
        worker = std::jthread { &BookBuilderWorker::run, this };
    }

    void BookBuilderWorker::stop() noexcept
    {
        if (!running)
            return;
        queue.close();
        if (worker.joinable())
            worker.join();
        running = false;
    }

    void BookBuilderWorker::run() const
    {
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