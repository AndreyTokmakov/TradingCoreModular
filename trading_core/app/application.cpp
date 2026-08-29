/**============================================================================
Name        : application.cpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : application.cpp
============================================================================**/

/*
    Application implementation.
    Application constructs and wires the market-data components.

    Data Flow:

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
                 +------------------+
                 |                  |
                 v                  v
              Strategy           Recorder

    Application does not process market-data events itself. It only creates
    the components and establishes their relationships.
*/

#include "application.hpp"
#include "json_config_loader.hpp"
#include "logger_factory.hpp"

namespace
{
    using trading::config::Config;
    using trading::config::Error;
    using trading::config::ExchangeConfig;
    using trading::config::JsonConfigLoader;

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
}

namespace trading::app
{
Application::Application(const std::filesystem::path& configPath):
        metricsCollector {  metrics::MetricsCollector::getCollector() },
        config { loadConfig(configPath) },
        orderBook {},
        recorder {},
        positionManager {},
        riskManager { config.riskLimits },
        strategy {
            config.strategy.thresholdNumerator,
            config.strategy.thresholdDenominator
        },
        binanceExecutionGateway {
            findExchange(config, "binance").executionEndpoint
        },
        orderManager {                                                // CPU-3:  OrderManager --> gateway.send() / cancel()
            riskManager, positionManager, binanceExecutionGateway
        },
        executionWorker {                                             // CPU-3: executionQueue --> ExecutionWorker --> OrderManager::createOrder()
            executionQueue, orderManager, recorder, metricsCollector
        },
        strategyExecutor {
            executionQueue, config.strategy.orderQuantity          // CPU-2: StrategyExecutor   --> executionQueue::push()
        },
        strategyWorker {                                              // CPU-2: strategyEventQueue --> StrategyProcessor -> ImbalanceStrategy::evaluate()
            strategyEventQueue, strategy, strategyExecutor   //                                                 -> StrategyExecutor::execute()
        },
        executionReportSource {
            findExchange(config, "binance").executionEndpoint, executionQueue
        },
        marketEventDispatcher {                                        // CPU-1: MarketEventDispatcher   -->   strategyEventQueue::push()
            strategyEventQueue, recordingEventQueue              //                                -->   recordingEventQueue::push()
        },
        bookBuilder {
            config.instrument, orderBook, marketEventDispatcher  // CPU-1: BookBuilder        --> OrderBook::onBookUpdate()
        },                                                             //                           --> MarketEventDispatcher::onMarketEvent()
        binanceSnapshotProvider {
            findExchange(config, "binance").marketDataEndpoint
        },
        bookBuilderWorker {                                            // CPU-1: bookUpdateQueue    --> BookBuilderWorker --> BookBuilder
            bookBuilder, binanceSnapshotProvider, bookUpdateQueue
        },
        recordingWorker {                                              // CPU-4:
            recorder, recordingEventQueue
        },
        marketDataParser {},                                           // CPU-0:  MarketDataMessageHandler  --> BinanceMarketDataParser -- > bookUpdateQueue.push();
        marketDataMessageHandler {                                     // CPU-0:  MarketDataSource --> MarketDataMessageHandler
            marketDataParser, bookUpdateQueue
        },
        marketDataSource {                                             // CPU-0:  Exchange --> MarketDataSource
            findExchange(config, "binance").marketDataEndpoint
        }
    {
        configureMarketData();
    }

    Application::~Application() {
        stop();
    }

    void Application::configureMarketData() {
        marketDataSource.setMessageHandler(marketDataMessageHandler);
    }

    void Application::start()
    {
        //const auto logger = logging::LoggerFactory::createLogger({}, {});
        if (running)
            return;

        running = true;

        marketDataSource.start();    // CPU-0
        bookBuilderWorker.start();   // CPU-1
        strategyWorker.start();
        executionWorker.start();
        recordingWorker.start();
        executionReportSource.start();
    }

    void Application::stop()
    {
        if (!running)
            return;

        executionReportSource.stop();
        recordingWorker.stop();
        executionWorker.stop();
        strategyWorker.stop();
        bookBuilderWorker.stop();
        marketDataSource.stop();
        running = false;
    }
}