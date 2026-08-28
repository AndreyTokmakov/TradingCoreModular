/**============================================================================
Name        : test_market_data_parser.cpp
Created on  : 20.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : test_market_data_parser.cpp
============================================================================**/

#include "test_market_data_parser.hpp"
#include <nlohmann/json.hpp>

namespace
{
    using trading::Side;
    using trading::market_data::ParseResult;

    template<typename T>
    bool parseNumber(std::string_view str, T& out)
    {
        if (str.empty())
            return false;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), out);
        return ec == std::errc() && ptr == str.data() + str.size();
    }

    ParseResult
    parseBookUpdate(const std::string_view data,
                    trading::market_data::BookUpdate& bookUpdate)
    {
        if (data.empty())
            return trading::market_data::ParseResult::EmptyMessage;
        std::vector<std::string_view> fields;
        for (size_t start = 0, end = 0; end <= data.size(); ++end) {
            if (end == data.size() || data[end] == ',') {
                fields.push_back(data.substr(start, end - start));
                start = end + 1;
            }
        }

        if (fields.size() != 7)
            return ParseResult::InvalidMessage;
        if (!parseNumber(fields[0], bookUpdate.instrument))
            return ParseResult::InvalidInstrument;
        if (!parseNumber(fields[1], bookUpdate.sequence))
            return ParseResult::InvalidSequence;

        int64_t value { 0 };
        if (!parseNumber(fields[2], value))
            return ParseResult::InvalidTimestamp;
        bookUpdate.exchangeTimestamp = static_cast<decltype(bookUpdate.exchangeTimestamp)>(value);

        if (fields[3] == "Buy") {
            bookUpdate.side = Side::Buy;
        } else if (fields[3] == "Sell") {
            bookUpdate.side = Side::Sell;
        } else {
            return ParseResult::InvalidSide;
        }

        if (!parseNumber(fields[4], value))
            return ParseResult::InvalidPrice;
        bookUpdate.price = static_cast<decltype(bookUpdate.price)>(value);

        if (!parseNumber(fields[5], value))
            return ParseResult::InvalidPrice;
        bookUpdate.quantity = static_cast<decltype(bookUpdate.quantity)>(value);

        return ParseResult::Success;
    }

}


namespace trading::testing::stubs
{
    market_data::ParseResult
    TestMarketDataParser::parse([[maybe_unused]] std::string_view message,
                                    market_data::BookUpdates& bookUpdates) const
    {
        bookUpdates.clear();

        /*
            Existing Binance parsing logic goes here.

            Every place where the previous implementation created or returned
            a local vector must now write directly into bookUpdates.

            Example:
                bookUpdates.emplace_back(...);

            Successful parsing:
                return market_data::ParseResult::Success;

            Parsing failure:
                return market_data::ParseResult::InvalidMessage;

            The concrete error values should correspond to the actual
            validation failure.
        */

        return market_data::ParseResult::Success;;


#if 0
        try
        {
            const auto json = nlohmann::json::parse(message);

            if (!json.is_object())
                return std::unexpected { market_data::ParseError::InvalidMessage };
            /*
                Binance depthUpdate example:

                {
                    "e": "depthUpdate",
                    "E": 1672515782136,
                    "s": "BTCUSDT",
                    "U": 157,
                    "u": 160,
                    "b": [
                        ["0.10000000", "1.00000000"]
                    ],
                    "a": [
                        ["0.20000000", "2.00000000"]
                    ]
                }

                The exact conversion to InstrumentId, Price and Quantity
                should use the existing project-specific conversion logic.
            */

            if (!json.contains("e"))
                return std::unexpected { market_data::ParseError::MissingField };

            if (json.at("e") != "depthUpdate")
                return std::unexpected {
                    market_data::ParseError::UnsupportedMessage
                };

            if (!json.contains("E") ||
                !json.contains("U") ||
                !json.contains("u") ||
                !json.contains("b") ||
                !json.contains("a"))
            {
                return std::unexpected {
                    market_data::ParseError::MissingField
                };
            }

            const auto sequence = json.at("u").get<SequenceNumber>();
            const auto timestamp = json.at("E").get<Timestamp::Value>();

            constexpr InstrumentId instrument {
                // Use the existing Binance symbol -> InstrumentId mapping.
                0
            };

            std::vector<market_data::BookUpdate> updates;

            for (const auto& level : json.at("b"))
            {
                if (!level.is_array() || level.size() != 2)
                    return std::unexpected {
                        market_data::ParseError::InvalidField
                    };

                market_data::BookUpdate update;
                update.instrument = instrument;
                update.sequence = sequence;
                update.exchangeTimestamp = Timestamp { timestamp };
                update.side = Side::Buy;

                // Use the existing Price / Quantity construction logic here.
                update.price = Price { level.at(0).get<double>() };
                update.quantity = Quantity { level.at(1).get<double>() };

                updates.push_back(update);
            }

            for (const auto& level : json.at("a"))
            {
                if (!level.is_array() || level.size() != 2)
                    return std::unexpected {
                        market_data::ParseError::InvalidField
                    };

                market_data::BookUpdate update;
                update.instrument = instrument;
                update.sequence = sequence;
                update.exchangeTimestamp = Timestamp { timestamp };
                update.side = Side::Sell;

                update.price = Price { level.at(0).get<double>() };
                update.quantity = Quantity { level.at(1).get<double>() };

                updates.push_back(update);
            }

            return updates;
        }
        catch (const nlohmann::json::exception&)
        {
            return std::unexpected {
                market_data::ParseError::InvalidJson
            };
        }
#endif
    }
}