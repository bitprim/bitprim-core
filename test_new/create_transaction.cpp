/**
 * Copyright (c) 2016-2018 Bitprim Inc.
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
// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

// #include <bitcoin/bitcoin.hpp>
#include <bitprim/keoken/message/base.hpp>
#include <bitcoin/bitcoin/chain/transaction.hpp>
#include <bitcoin/bitcoin/chain/input.hpp>
#include <bitcoin/bitcoin/chain/output.hpp>
#include <bitcoin/bitcoin/chain/input_point.hpp>
#include <bitcoin/bitcoin/chain/output_point.hpp>
#include <bitcoin/bitcoin/config/output.hpp>
#include <bitcoin/bitcoin/config/input.hpp>
#include <bitcoin/bitcoin/wallet/transaction_functions.hpp>
#include <bitcoin/bitcoin/formats/base_16.hpp>

#include <bitprim/keoken/message/base.hpp>
#include <bitprim/keoken/message/create_asset.hpp>
#include <bitprim/keoken/message/send_tokens.hpp>
#include <bitprim/keoken/wallet/create_transaction.hpp>

using namespace bc;
using namespace bitprim::keoken::wallet;

#define SEED "fffb587496cc54912bbcef874fa9a61a"
#define WALLET "mwx2YDHgpdfHUmCpFjEi9LarXf7EkQN6YG"
#define WALLETDESTINY "ms9qrYaJ468QCwgX6v7ittgX3vBk3mg6PM"
#define TX_ENCODE "01000000019373b022dfb99400ee40b8987586aea9e158f3b0c62343d59896c212cee60d980100000000ffffffff0118beeb0b000000001976a914b43ff4532569a00bcab4ce60f87cdeebf985b69a88ac00000000"
#define SIGNATURE "30440220433c405e4cb7698ad5f58e0ea162c3c3571d46d96ff1b3cb9232a06eba3b444d02204bc5f48647c0f052ade7cf85eac3911f7afbfa69fa5ebd92084191a5da33f88d41"
#define COMPLETE_TX "01000000019373b022dfb99400ee40b8987586aea9e158f3b0c62343d59896c212cee60d98010000006a4730440220433c405e4cb7698ad5f58e0ea162c3c3571d46d96ff1b3cb9232a06eba3b444d02204bc5f48647c0f052ade7cf85eac3911f7afbfa69fa5ebd92084191a5da33f88d4121027a45d4abb6ebb00214796e2c7cf61d18c9185ba771fe9ed75b303eb7a8e9028bffffffff0118beeb0b000000001976a914b43ff4532569a00bcab4ce60f87cdeebf985b69a88ac00000000"


TEST_CASE("[keoken__send_token__expected]") {
    std::vector<chain::input_point> outputs_to_spend;
    bc::hash_digest hash_to_spend;
    bc::decode_hash(hash_to_spend, "980de6ce12c29698d54323c6b0f358e1a9ae867598b840ee0094b9df22b07393");
    uint32_t const index_to_spend = 1;
    outputs_to_spend.push_back({hash_to_spend, index_to_spend});
    bitprim::keoken::asset_id_t asset_id = 1;
    bitprim::keoken::amount_t amount_tokens = 1;
    auto result = tx_encode_send_token(outputs_to_spend, bc::wallet::payment_address(WALLET), 21647102398, bc::wallet::payment_address(WALLETDESTINY), 20000, asset_id, amount_tokens);
    // BOOST_REQUIRE_EQUAL(1, 1);
    REQUIRE(true); 
}

TEST_CASE("[keoken__create_asset__expected]") {
    std::vector<chain::input_point> outputs_to_spend;
    bc::hash_digest hash_to_spend;
    bc::decode_hash(hash_to_spend, "980de6ce12c29698d54323c6b0f358e1a9ae867598b840ee0094b9df22b07393");
    uint32_t const index_to_spend = 1;
    outputs_to_spend.push_back({hash_to_spend, index_to_spend});
    std::string name = "testcoin";
    bitprim::keoken::amount_t amount_tokens = 1;
    auto result = tx_encode_create_asset(outputs_to_spend, bc::wallet::payment_address(WALLET), 21647102398, name, amount_tokens);
    // BOOST_REQUIRE_EQUAL(1, 1);
    REQUIRE(true); 
}



