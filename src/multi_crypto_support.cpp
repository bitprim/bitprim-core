/**
 * Copyright (c) 2011-2017 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// #include <iostream>

#include <string>

#include <bitcoin/bitcoin/multi_crypto_support.hpp>

namespace libbitcoin {

namespace {

static config::currency currency_ = config::currency::none;
static config::settings network_ = config::settings::none;

#ifdef BITPRIM_CURRENCY_BCH
static std::string cashaddr_prefix_ = "bitcoincash";
#endif // BITPRIM_CURRENCY_BCH

} // namespace anonymous

config::currency get_currency() {
#ifdef BITPRIM_CURRENCY_LTC
    return config::currency::litecoin;
#elif defined(BITPRIM_CURRENCY_BCH)
    return config::currency::bitcoin_cash;
#else
    return config::currency::bitcoin;
#endif //BITPRIM_CURRENCY_LTC
}

// void set_network(config::settings x) {
//     network_ = x;
// }

// void set_network(uint32_t identifier) {
// #ifdef BITPRIM_CURRENCY_LTC
//     switch(identifier) {
//         case netmagic::ltc_mainnet: set_network(config::settings::mainnet); break;
//         case netmagic::ltc_testnet: set_network(config::settings::testnet); break;
//         case netmagic::ltc_regtest: set_network(config::settings::regtest); break;
//         default: set_network(config::settings::none);
//     }
// #elif defined(BITPRIM_CURRENCY_BCH)
//     switch(identifier) {
//         case netmagic::bch_mainnet: set_network(config::settings::mainnet); break;
//         case netmagic::bch_testnet: set_network(config::settings::testnet); break;
//         case netmagic::bch_regtest: set_network(config::settings::regtest); break;
//         default: set_network(config::settings::none);
//     }
// #else
//     switch(identifier) {
//         case netmagic::btc_mainnet: set_network(config::settings::mainnet); break;
//         case netmagic::btc_testnet: set_network(config::settings::testnet); break;
//         case netmagic::btc_regtest: set_network(config::settings::regtest); break;
//         default: set_network(config::settings::none);
//     }
// #endif //BITPRIM_CURRENCY_LTC
// }

// config::settings get_network() {
//     return network_;
// }


config::settings get_network(uint32_t identifier) {
#ifdef BITPRIM_CURRENCY_LTC
    switch (identifier) {
        case netmagic::ltc_mainnet: return config::settings::mainnet;
        case netmagic::ltc_testnet: return config::settings::testnet;
        case netmagic::ltc_regtest: return config::settings::regtest;
        default: return config::settings::none;
    }
#elif defined(BITPRIM_CURRENCY_BCH)
    switch (identifier) {
        case netmagic::bch_mainnet: return config::settings::mainnet;
        case netmagic::bch_testnet: return config::settings::testnet;
        case netmagic::bch_regtest: return config::settings::regtest;
        default: return config::settings::none;
    }
#else
    switch (identifier) {
        case netmagic::btc_mainnet: return config::settings::mainnet;
        case netmagic::btc_testnet: return config::settings::testnet;
        case netmagic::btc_regtest: return config::settings::regtest;
        default: return config::settings::none;
    }
#endif //BITPRIM_CURRENCY_LTC
}

#ifdef BITPRIM_CURRENCY_BCH
std::string cashaddr_prefix() {
    return cashaddr_prefix_;
}

void set_cashaddr_prefix(std::string const& x) {
    cashaddr_prefix_ = x;
}
#endif //BITPRIM_CURRENCY_BCH


// bool is_testnet(uint32_t identifier, bool bitcoin_cash) {
// #ifdef BITPRIM_CURRENCY_LTC
//     return identifier == 0xf1c8d2fdu //4056470269u; //Litecoin Testnet Netmagic
// #else
//     if (bitcoin_cash) {
//         return identifier == 0xf4f3e5f4u;  //Bitcoin Cash Testnet Netmagic
//     } else {
//         return identifier == 0x0709110bu;  //Bitcoin Testnet Netmagic
//     }
// #endif //BITPRIM_CURRENCY_LTC
// }

} /*namespace libbitcoin*/

