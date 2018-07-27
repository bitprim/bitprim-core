/**
 * Copyright (c) 2011-2017 Bitprim developers (see AUTHORS)
 * Copyright (c) 2017 Pieter Wuille
 * Copyright (c) 2017 The Bitcoin developers
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

// Cashaddr is an address format inspired by bech32.

#ifndef BITPRIM_CORE_WALLET_CASHADDR_HPP_
#define BITPRIM_CORE_WALLET_CASHADDR_HPP_

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <bitcoin/bitcoin/utility/data.hpp>

namespace libbitcoin { namespace wallet { namespace cashaddr {

/**
 * Encode a cashaddr string. Returns the empty string in case of failure.
 */
std::string encode(std::string const& prefix, data_chunk const& payload);

/**
 * Decode a cashaddr string. Returns (prefix, data). Empty prefix means failure.
 */
std::pair<std::string, data_chunk> decode(std::string const& str, std::string const& default_prefix);

}}} // namespace libbitcoin::wallet::cashaddr 

#endif /* BITPRIM_CORE_WALLET_CASHADDR_HPP_ */


// Unit Tests ----------------------------------------------------
#ifdef DOCTEST_LIBRARY_INCLUDED
using namespace libbitcoin::wallet;
using namespace std;

static std::pair<std::string, std::vector<uint8_t>>
cashdddr_decode(std::string const& str) {
    return cashaddr::decode(str, "");
}

bool case_insensitive_equal(std::string const& a, std::string const& b) {
    if (a.size() != b.size()) {
        return false;
    }

    for (size_t i = 0; i < a.size(); ++i) {
        char c1 = a[i];
        if (c1 >= 'A' && c1 <= 'Z') {
            c1 -= ('A' - 'a');
        }
        char c2 = b[i];
        if (c2 >= 'A' && c2 <= 'Z') {
            c2 -= ('A' - 'a');
        }
        if (c1 != c2) {
            return false;
        }
    }

    return true;
}

TEST_CASE("[cashaddr_testvectors_valid] cashaddr_testvectors_valid") {
    static const std::string CASES[] = {
        "prefix:x64nx6hz",
        "PREFIX:X64NX6HZ",
        "p:gpf8m4h7",
        "bitcoincash:qpzry9x8gf2tvdw0s3jn54khce6mua7lcw20ayyn",
        "bchtest:testnetaddress4d6njnut",
        "bchreg:555555555555555555555555555555555555555555555udxmlmrz",
    };

    for (std::string const& str : CASES) {
        auto ret = cashdddr_decode(str);
        CHECK( ! ret.first.empty());
        std::string recode = cashaddr::encode(ret.first, ret.second);
        CHECK( ! recode.empty());
        CHECK(case_insensitive_equal(str, recode));
    }
}

TEST_CASE("[cashaddr_testvectors_invalid] cashaddr_testvectors_invalid") {
    static const std::string CASES[] = {
        "prefix:x32nx6hz",
        "prEfix:x64nx6hz",
        "prefix:x64nx6Hz",
        "pref1x:6m8cxv73",
        "prefix:",
        ":u9wsx07j",
        "bchreg:555555555555555555x55555555555555555555555555udxmlmrz",
        "bchreg:555555555555555555555555555555551555555555555udxmlmrz",
        "pre:fix:x32nx6hz",
        "prefixx64nx6hz",
    };

    for (std::string const& str : CASES) {
        auto ret = cashdddr_decode(str);
        CHECK(ret.first.empty());
    }
}

TEST_CASE("[cashaddr_rawencode] cashaddr_rawencode") {
    using raw = std::pair<std::string, std::vector<uint8_t>>;

    raw toEncode;
    toEncode.first = "helloworld";
    toEncode.second = {0x1f, 0x0d};

    std::string encoded = cashaddr::encode(toEncode.first, toEncode.second);
    raw decoded = cashdddr_decode(encoded);

    CHECK(toEncode.first == decoded.first);

    // BOOST_CHECK_EQUAL_COLLECTIONS(begin(toEncode.second), end(toEncode.second),
    //                               begin(decoded.second), end(decoded.second));

    CHECK(std::equal(begin(toEncode.second), end(toEncode.second),
                     begin(decoded.second), end(decoded.second)));

}

TEST_CASE("[cashaddr_testvectors_noprefix] cashaddr_testvectors_noprefix") {
    static const std::pair<std::string, std::string> CASES[] = {
        {"bitcoincash", "qpzry9x8gf2tvdw0s3jn54khce6mua7lcw20ayyn"},
        {"prefix", "x64nx6hz"},
        {"PREFIX", "X64NX6HZ"},
        {"p", "gpf8m4h7"},
        {"bitcoincash", "qpzry9x8gf2tvdw0s3jn54khce6mua7lcw20ayyn"},
        {"bchtest", "testnetaddress4d6njnut"},
        {"bchreg", "555555555555555555555555555555555555555555555udxmlmrz"},
    };

    for (std::pair<std::string, std::string> const& c : CASES) {
        std::string prefix = c.first;
        std::string payload = c.second;
        std::string addr = prefix + ":" + payload;
        auto ret = cashaddr::decode(payload, prefix);

        CHECK(case_insensitive_equal(ret.first, prefix));
        std::string recode = cashaddr::encode(ret.first, ret.second);
        CHECK( ! recode.empty());
        CHECK(case_insensitive_equal(addr, recode));
    }
}
#endif /*DOCTEST_LIBRARY_INCLUDED*/
// Unit Tests ----------------------------------------------------
