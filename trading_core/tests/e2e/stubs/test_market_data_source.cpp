/**============================================================================
Name        : test_market_data_source.cpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Test market data source implementation.
============================================================================**/

#include "test_market_data_source.hpp"

#include <utility>

namespace trading::testing::stubs
{
    TestMarketDataSource::TestMarketDataSource(std::string endpoint) noexcept :
        endpoint { std::move(endpoint) }
    {
    }

    void TestMarketDataSource::start()
    {
        running = true;

        // TODO: Connect to endpoint.
        // TODO: Start receiving market data.
    }

    void TestMarketDataSource::stop()
    {
        running = false;

        // TODO: Close connection.
    }

    void TestMarketDataSource::setMessageHandler(market_data::IMarketDataMessageHandler& handler)
    {
        messageHandler = &handler;
    }
}