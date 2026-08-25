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
        orderManager {
            riskManager, positionManager, executionOrderQueue
        },
        strategyExecutor {
            orderManager, config.strategy.orderQuantity
        },
        executionReportHandler {
            orderManager, positionManager, recorder
        },
        executionReportSource {
            findExchange(config, "binance").executionEndpoint
        },
        marketEventDispatcher {
            strategyEventQueue, recordingEventQueue
        },
        bookBuilder {
            config.instrument, orderBook, marketEventDispatcher
        },
        bookBuilderWorker {
            bookBuilder, bookUpdateQueue
        },
        strategyWorker {
            strategy, strategyExecutor, strategyEventQueue
        },
        executionWorker {
            binanceExecutionGateway,executionOrderQueue
        },
        recordingWorker {
            recorder, recordingEventQueue
        },
        marketDataParser {},
        marketDataMessageHandler {
            marketDataParser, bookUpdateQueue
        },
        marketDataSource {
            findExchange(config, "binance").marketDataEndpoint
        }
    {
        configureMarketData();
    }

    Application::~Application()
    {
        stop();
    }

    void Application::configureMarketData()
    {
        marketDataSource.setMessageHandler(marketDataMessageHandler);
    }

    void Application::configureExecutionReports()
    {
        executionReportSource.setExecutionReportHandler(executionReportHandler);
    }

    void Application::start()
    {
        //const auto logger = logging::LoggerFactory::createLogger({}, {});
        if (running)
            return;

        running = true;

        marketDataSource.start();
        bookBuilderWorker.start();
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