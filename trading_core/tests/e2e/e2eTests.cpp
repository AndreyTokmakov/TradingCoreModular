/**============================================================================
Name        : e2eTests.cpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : order_manager_test.cpp
============================================================================**/

#include <iostream>

#include "config.hpp"
#include "book_builder.hpp"
#include "book_builder_worker.hpp"
#include "condition_variable_queue.hpp"
#include "execution_worker.hpp"
#include "imbalance_strategy.hpp"
#include "market_data_message_handler.hpp"
#include "market_event_dispatcher.hpp"
#include "metrics_collector.hpp"
#include "order_book.hpp"
#include "order_manager.hpp"
#include "position_manager.hpp"
#include "recording_worker.hpp"
#include "risk_manager.hpp"
#include "strategy_executor.hpp"
#include "strategy_worker.hpp"
#include "trade_recorder.hpp"
#include "json_config_loader.hpp"
#include "logger_factory.hpp"

#include "e2e/stubs/test_execution_gateway.hpp"
#include "e2e/stubs/test_execution_report_source.hpp"
#include "e2e/stubs/test_market_data_parser.hpp"
#include "e2e/stubs/test_market_data_source.hpp"
#include "e2e/stubs/test_snapshot_provider.hpp"


namespace
{
    using trading::Price;
    using trading::Quantity;
    using trading::Timestamp;
    using trading::SequenceNumber;

    namespace market_data = trading::market_data;
    namespace concurrency = trading::concurrency;
    namespace execution = trading::execution;
    namespace config = trading::config;
    namespace recording = trading::recording;
    namespace testing = trading::testing;
    namespace position = trading::position;
    namespace strategy = trading::strategy;
    namespace risk = trading::risk;

    using config::Config;
    using config::Error;
    using config::ExchangeConfig;
    using config::JsonConfigLoader;

    class TestApplication final
    {
    public:
        explicit TestApplication(const std::filesystem::path& configPath);
        ~TestApplication();

        TestApplication(const TestApplication&) = delete;
        TestApplication& operator=(const TestApplication&) = delete;

        TestApplication(TestApplication&&) = delete;
        TestApplication& operator=(TestApplication&&) = delete;

        void start();
        void stop();

    private:

        void configureMarketData();

        market_data::Snapshot getSnapshot();

        concurrency::ConditionVariableQueue<market_data::BookUpdates> bookUpdateQueue;
        concurrency::ConditionVariableQueue<market_data::MarketEvent> strategyEventQueue;
        concurrency::ConditionVariableQueue<market_data::MarketEvent> recordingEventQueue;
        concurrency::ConditionVariableQueue<execution::ExecutionWorkItem>  executionOrderQueue;

        trading::metrics::MetricsCollector& metricsCollector;
        Config config;

        market_data::OrderBook orderBook;
        recording::TradeRecorder recorder;
        position::PositionManager positionManager;
        risk::RiskManager riskManager;

        strategy::ImbalanceStrategy strategy;

        testing::stubs::TestExecutionGateway testExecutionGateway;
        execution::OrderManager orderManager;
        execution::ExecutionWorker executionWorker;
        strategy::StrategyExecutor strategyExecutor;
        strategy::StrategyWorker strategyWorker;

        testing::stubs::TestExecutionReportSource testExecutionReportSource;

        market_data::MarketEventDispatcher marketEventDispatcher;
        market_data::BookBuilder bookBuilder;
        testing::stubs::TestSnapshotProvider testSnapshotProvider;
        market_data::BookBuilderWorker bookBuilderWorker;

        recording::RecordingWorker recordingWorker;

        testing::stubs::TestMarketDataParser testMarketDataParser;
        market_data::MarketDataMessageHandler marketDataMessageHandler;
        testing::stubs::TestMarketDataSource testMarketDataSource;

        bool running { false };
    };
}


namespace
{
    [[nodiscard]]
    Config loadConfig(const std::filesystem::path& configPath)
    {
        if (const std::expected<Config, Error> result = JsonConfigLoader::load(configPath))
            return *result;
        throw std::runtime_error {"Failed to load configuration: " +configPath.string()};
    }

    [[nodiscard]]
    const ExchangeConfig& findExchange(const Config& config,
                                       const std::string_view name)
    {
        for (const ExchangeConfig& exchange : config.exchanges)
        {
            if (exchange.name == name)
                return exchange;
        }

        throw std::runtime_error {
            "Exchange configuration not found: " + std::string { name }
        };
    }

    TestApplication::TestApplication(const std::filesystem::path& configPath):
            metricsCollector {  trading::metrics::MetricsCollector::getCollector() },
            config { loadConfig(configPath) },
            orderBook {},
            recorder {},
            positionManager {},
            riskManager { config.riskLimits },
            strategy {
                config.strategy.thresholdNumerator,
                config.strategy.thresholdDenominator
            },
            testExecutionGateway {
                findExchange(config, "binance").executionEndpoint
            },
            orderManager {                                                 // CPU-3:  OrderManager --> gateway.send() / cancel()
                riskManager, positionManager, testExecutionGateway
            },
            executionWorker {                                             // CPU-3: executionOrderQueue --> ExecutionWorker --> OrderManager::createOrder()
                executionOrderQueue, orderManager, recorder, metricsCollector
            },
            strategyExecutor {
                executionOrderQueue, config.strategy.orderQuantity     // CPU-2: StrategyExecutor   --> executionOrderQueue::push()
            },
            strategyWorker {                                              // CPU-2: strategyEventQueue --> StrategyProcessor -> ImbalanceStrategy::evaluate()
                strategyEventQueue, strategy, strategyExecutor   //                                                 -> StrategyExecutor::execute()
            },
            testExecutionReportSource {
                findExchange(config, "binance").executionEndpoint, executionOrderQueue
            },
            marketEventDispatcher {                                        // CPU-1: MarketEventDispatcher   -->   strategyEventQueue::push()
                strategyEventQueue, recordingEventQueue              //                                -->   recordingEventQueue::push()
            },
            bookBuilder {
                config.instrument, orderBook, marketEventDispatcher  // CPU-1: BookBuilder        --> OrderBook::onBookUpdate()
            },                                                             //                           --> MarketEventDispatcher::onMarketEvent()
            testSnapshotProvider {
                findExchange(config, "binance").marketDataEndpoint,  [this]{ return getSnapshot(); }
            },
            bookBuilderWorker {
                bookBuilder, testSnapshotProvider, bookUpdateQueue  // CPU-1: bookUpdateQueue    --> BookBuilderWorker --> BookBuilder
            },
            recordingWorker {                                              // CPU-4:
                recorder, recordingEventQueue
            },
            testMarketDataParser {},                                           // CPU-0:  MarketDataMessageHandler  --> BinanceMarketDataParser -- > bookUpdateQueue.push();
            marketDataMessageHandler {                                     // CPU-0:  MarketDataSource --> MarketDataMessageHandler
                testMarketDataParser, bookUpdateQueue
            },
            testMarketDataSource {                                             // CPU-0:  Exchange --> MarketDataSource
                findExchange(config, "binance").marketDataEndpoint
            }
        {
            configureMarketData();
        }

    TestApplication::~TestApplication()
    {
        stop();
    }

    void TestApplication::configureMarketData()
    {
        testMarketDataSource.setMessageHandler(marketDataMessageHandler);
    }

    void TestApplication::start()
    {
        //const auto logger = logging::LoggerFactory::createLogger({}, {});
        if (running)
            return;

        running = true;

        bookBuilderWorker.start();       // CPU-1
        strategyWorker.start();

        std::this_thread::sleep_for(std::chrono::seconds { 1 });

        testMarketDataSource.start();    // CPU-0

        // executionWorker.start();
        // recordingWorker.start();
        // testExecutionReportSource.start();
    }

    void TestApplication::stop()
    {
        if (!running)
            return;

        testExecutionReportSource.stop();
        recordingWorker.stop();
        executionWorker.stop();
        strategyWorker.stop();
        bookBuilderWorker.stop();
        testMarketDataSource.stop();
        running = false;
    }

    market_data::Snapshot TestApplication::getSnapshot()
    {
        std::cout << __PRETTY_FUNCTION__ << "[" << __LINE__ << "] " << std::endl;
        constexpr Timestamp exchangeTimestamp { 1'000'000 };

        return market_data::Snapshot {
            .instrument = 1,
            .sequence = SequenceNumber { 1'000'000 },
            .exchangeTimestamp = exchangeTimestamp,
            .bids = {
                            { Price { 6'500'000'000'000 }, Quantity { 120'000'000 } },
                            { Price { 6'499'999'000'000 }, Quantity { 250'000'000 } }
            },
            .asks = {
                            { Price { 6'500'001'000'000 }, Quantity { 90'000'000 } },
                            { Price { 6'500'002'000'000 }, Quantity { 310'000'000 } }
            }
        };
    }
}

void e2eTests()
{
    TestApplication test("../../trading_core/tests/e2e/config/test_local.json");
    test.start();
}