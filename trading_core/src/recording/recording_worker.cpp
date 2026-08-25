/**============================================================================
Name        : recording_worker.cpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Records market events on the recording thread.
============================================================================**/

#include "recording_worker.hpp"

namespace trading::recording
{
    RecordingWorker::RecordingWorker(
        IRecorder& recorder,
        concurrency::Queue<market_data::MarketEvent>& queue):
        recorder { recorder },
        queue { queue }
    {
    }

    RecordingWorker::~RecordingWorker()
    {
        stop();
    }

    void RecordingWorker::start()
    {
        if (running)
            return;
        running = true;
        worker = std::jthread { &RecordingWorker::run, this };
    }

    void RecordingWorker::stop() noexcept
    {
        if (!running)
            return;
        queue.close();
        if (worker.joinable())
            worker.join();
        running = false;
    }

    void RecordingWorker::run() const
    {
        market_data::MarketEvent event;
        while (queue.waitPop(event)) {
            recorder.record(event);
        }
    }
}