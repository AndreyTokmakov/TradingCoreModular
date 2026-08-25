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
#include "market_data_parser.hpp"
#include "queue.hpp"

#include <thread>


namespace trading::market_data
{
    class BookBuilderWorker final
    {
    public:
        BookBuilderWorker(BookBuilder& bookBuilder,
                          concurrency::Queue<BookUpdates>& queue);

        ~BookBuilderWorker();

        BookBuilderWorker(const BookBuilderWorker&) = delete;
        BookBuilderWorker& operator=(const BookBuilderWorker&) = delete;

        BookBuilderWorker(BookBuilderWorker&&) = delete;
        BookBuilderWorker& operator=(BookBuilderWorker&&) = delete;

        void start();

        void stop() noexcept;

    private:
        void run() const;

        BookBuilder& bookBuilder;
        concurrency::Queue<BookUpdates>& queue;
        std::jthread worker;
        bool running { false };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BOOK_BUILDER_WORKER_HPP