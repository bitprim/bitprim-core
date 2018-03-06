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
#ifndef LIBBITCOIN_MULTI_CRYPTO_SUPPORT_HPP_
#define LIBBITCOIN_MULTI_CRYPTO_SUPPORT_HPP_

#include <string>
#include <bitcoin/bitcoin/config/settings.hpp>

namespace libbitcoin {

namespace netmagic {
constexpr uint32_t btc_mainnet = 0xd9b4bef9u;
constexpr uint32_t btc_testnet = 0x0709110bu;
constexpr uint32_t btc_regtest = 0xdab5bffau;

constexpr uint32_t ltc_mainnet = 0xdbb6c0fbu;
constexpr uint32_t ltc_testnet = 0xf1c8d2fdu;
constexpr uint32_t ltc_regtest = 0xdab5bffau;

constexpr uint32_t bch_mainnet = 0xe8f3e1e3u;
constexpr uint32_t bch_testnet = 0xf4f3e5f4u;
constexpr uint32_t bch_regtest = 0xfabfb5dau;
} // namespace netmagic

namespace config {

enum class currency {
    none,
    bitcoin,
    bitcoin_cash,
    litecoin
};

} // namespace config

config::currency get_currency();

// void set_network(config::settings x);
// void set_network(uint32_t identifier);
// config::settings get_network();
config::settings get_network(uint32_t identifier);

#ifdef BITPRIM_CURRENCY_BCH
std::string cashaddr_prefix();
void set_cashaddr_prefix(std::string const& x);
#endif //BITPRIM_CURRENCY_BCH

} /*namespace libbitcoin*/

#endif /*LIBBITCOIN_MULTI_CRYPTO_SUPPORT_HPP_*/
