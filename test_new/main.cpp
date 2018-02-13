/**
 * Copyright (c) 2017 Bitprim developers (see AUTHORS)
 *
 * This file is part of Bitprim.
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

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <bitcoin/bitcoin/multi_crypto_support.hpp>
//#include <bitcoin/bitcoin/wallet/cashaddr.hpp>
#include <bitcoin/bitcoin/wallet/payment_address.hpp>

using namespace libbitcoin::wallet;
using namespace std;

#ifdef BITPRIM_CURRENCY_BCH
TEST_CASE("[payment_address__construct__payment__valid_expected] payment_address__construct__payment__valid_expected") {

    libbitcoin::set_bitcoin_cash(true);
    std::vector<std::pair<std::string, std::string>> cases = {
        {"1BpEi6DfDAUFd7GtittLSdBeYJvcoaVggu", "bitcoincash:qpm2qsznhks23z7629mms6s4cwef74vcwvy22gdx6a"},
        {"1KXrWXciRDZUpQwQmuM1DbwsKDLYAYsVLR", "bitcoincash:qr95sy3j9xwd2ap32xkykttr4cvcu7as4y0qverfuy"},
        {"16w1D5WRVKJuZUsSRzdLp9w3YGcgoxDXb", "bitcoincash:qqq3728yw0y47sqn6l2na30mcw6zm78dzqre909m2r"},
        {"3CWFddi6m4ndiGyKqzYvsFYagqDLPVMTzC", "bitcoincash:ppm2qsznhks23z7629mms6s4cwef74vcwvn0h829pq"},
        {"3LDsS579y7sruadqu11beEJoTjdFiFCdX4", "bitcoincash:pr95sy3j9xwd2ap32xkykttr4cvcu7as4yc93ky28e"},
        {"31nwvkZwyPdgzjBJZXfDmSWsC4ZLKpYyUw", "bitcoincash:pqq3728yw0y47sqn6l2na30mcw6zm78dzq5ucqzc37"}
    };

    for (auto&& c : cases) {
        payment_address const from_legacy(c.first);
        payment_address const from_cashaddr(c.second);
        CHECK(from_legacy.encoded() == from_cashaddr.encoded());
        CHECK(from_legacy.encoded_cashaddr() == from_cashaddr.encoded_cashaddr());
    }
}
#endif //BITPRIM_CURRENCY_BCH


