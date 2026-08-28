/**============================================================================
Name        : application.hpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : application.hpp
============================================================================**/

/*
    Application is the composition root of the trading system.

    Application constructs application components, connects their dependencies
    and controls the application lifecycle.

    Data Flow:

        Exchange
           |
           v
        BinanceMarketDataSource
           |
           | raw message
           v
        MarketDataMessageHandler
           |
           v
        BinanceMarketDataParser
           |
           | BookUpdate
           v
        BookBuilder
           |
           v
        OrderBook
           |
           | MarketEvent
           v
        MarketEventDispatcher
           |
           +----------------------+
           |                      |
           v                      v
        Strategy              Recorder

    Responsibilities:

        - construct application components;
        - establish dependencies between components;
        - configure the market-data pipeline;
        - control application lifecycle.

    Application does not implement trading logic. Domain responsibilities
    remain inside the corresponding modules.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_APPLICATION_HPP
#define FINANCETECHNOLOGYPROJECTS_APPLICATION_HPP

#include "config.hpp"

#include "binance_execution_gateway.hpp"
#include "binance_execution_report_source.hpp"
#include "binance_market_data_parser.hpp"
#include "binance_market_data_source.hpp"
#include "binance_snapshot_provider.hpp"

#include "book_builder.hpp"
#include "book_builder_worker.hpp"
#include "condition_variable_queue.hpp"
#include "execution_worker.hpp"
#include "imbalance_strategy.hpp"
#include "market_data_message_handler.hpp"
#include "market_event_dispatcher.hpp"
#include "order_book.hpp"
#include "order_manager.hpp"
#include "position_manager.hpp"
#include "recording_worker.hpp"
#include "risk_manager.hpp"
#include "strategy_executor.hpp"
#include "strategy_worker.hpp"
#include "trade_recorder.hpp"

namespace trading::app
{
    class Application final
    {
    public:
        Application(const std::filesystem::path& configPath);
        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        Application(Application&&) = delete;
        Application& operator=(Application&&) = delete;

        void start();
        void stop();

    private:

        void configureMarketData();

        concurrency::ConditionVariableQueue<market_data::BookUpdates> bookUpdateQueue;
        concurrency::ConditionVariableQueue<market_data::MarketEvent> strategyEventQueue;
        concurrency::ConditionVariableQueue<market_data::MarketEvent> recordingEventQueue;
        concurrency::ConditionVariableQueue<execution::ExecutionWorkItem>  executionQueue;

        config::Config config;

        market_data::OrderBook orderBook;
        recording::TradeRecorder recorder;
        position::PositionManager positionManager;
        risk::RiskManager riskManager;

        strategy::ImbalanceStrategy strategy;

        exchanges::binance::BinanceExecutionGateway binanceExecutionGateway;
        execution::OrderManager orderManager;
        execution::ExecutionWorker executionWorker;
        strategy::StrategyExecutor strategyExecutor;
        strategy::StrategyWorker strategyWorker;

        exchanges::binance::BinanceExecutionReportSource executionReportSource;

        market_data::MarketEventDispatcher marketEventDispatcher;
        market_data::BookBuilder bookBuilder;
        exchanges::binance::BinanceSnapshotProvider binanceSnapshotProvider;
        market_data::BookBuilderWorker bookBuilderWorker;

        recording::RecordingWorker recordingWorker;

        exchanges::binance::BinanceMarketDataParser marketDataParser;
        market_data::MarketDataMessageHandler marketDataMessageHandler;
        exchanges::binance::BinanceMarketDataSource marketDataSource;

        bool running { false };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_APPLICATION_HPP