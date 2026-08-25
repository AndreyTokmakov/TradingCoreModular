/**============================================================================
Name        : recording_worker.hpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Records market events on the recording thread.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_RECORDING_WORKER_HPP
#define FINANCETECHNOLOGYPROJECTS_RECORDING_WORKER_HPP

#include "market_event.hpp"
#include "queue.hpp"
#include "recorder.hpp"

#include <thread>

namespace trading::recording
{
    class RecordingWorker final
    {
    public:
        RecordingWorker(IRecorder& recorder,
                        concurrency::Queue<market_data::MarketEvent>& queue);

        ~RecordingWorker();

        RecordingWorker(const RecordingWorker&) = delete;
        RecordingWorker& operator=(const RecordingWorker&) = delete;

        RecordingWorker(RecordingWorker&&) = delete;
        RecordingWorker& operator=(RecordingWorker&&) = delete;

        void start();

        void stop() noexcept;

    private:
        void run() const;

        IRecorder& recorder;
        concurrency::Queue<market_data::MarketEvent>& queue;

        std::jthread worker;
        bool running { false };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_RECORDING_WORKER_HPP