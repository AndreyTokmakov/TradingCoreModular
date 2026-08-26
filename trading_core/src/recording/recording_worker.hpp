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
#include <worker.hpp>

namespace trading::recording
{
    class RecordingWorker final: public common::Worker<RecordingWorker> {
    public:
        RecordingWorker(IRecorder& recorder,
                        concurrency::Queue<market_data::MarketEvent>& queue);

        void run() const;

    private:
        IRecorder& recorder;
        concurrency::Queue<market_data::MarketEvent>& queue;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_RECORDING_WORKER_HPP