/**============================================================================
Name        : metrics_collector.hpp
Created on  : 29.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Registers per-thread metrics and aggregates them in the slow path.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_METRICS_COLLECTOR_HPP
#define FINANCETECHNOLOGYPROJECTS_METRICS_COLLECTOR_HPP

#include "metrics.hpp"

#include <list>
#include <mutex>

namespace trading::metrics
{
    class MetricsCollector
    {
    public:
        MetricsCollector(const MetricsCollector&) = delete;
        MetricsCollector& operator=(const MetricsCollector&) = delete;

        MetricsCollector(MetricsCollector&&) = delete;
        MetricsCollector& operator=(MetricsCollector&&) = delete;

        [[nodiscard]]
        Metrics& getThreadMetrics() noexcept
        {
            std::lock_guard<std::mutex> lock{mutex};
            thread_local Metrics &metrics = allMetrics.emplace_back();
            return metrics;
        }

        [[nodiscard]]
        static MetricsCollector& getCollector() noexcept
        {
            static MetricsCollector metrics_collector;
            return metrics_collector;
        }

        [[maybe_unused]]
        void aggregate() const
        {
            Metrics stats;
            {
                std::lock_guard<std::mutex> lock{mutex};
                for (const auto &metric : allMetrics){
                    stats += metric;
                }
            }
        }

    private:

        MetricsCollector() = default;

        mutable std::mutex mutex;
        std::list<Metrics> allMetrics {};
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_METRICS_COLLECTOR_HPP