/**
 * Copyright (c) 2018 Bitprim developers (see AUTHORS)
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

#include <bitcoin/bitcoin/chain/transaction.hpp>
#include <bitcoin/bitcoin/formats/base_16.hpp>
#include <bitcoin/bitcoin/utility/data.hpp>
#include <bitprim/keoken/transaction_extractor.hpp>


#include <bitprim/keoken/message/base.hpp>
#include <bitprim/keoken/message/create_asset.hpp>

#include <bitcoin/bitcoin/utility/container_source.hpp>
#include <bitcoin/bitcoin/utility/istream_reader.hpp>

using namespace bitprim::keoken;

using libbitcoin::data_chunk;
using libbitcoin::to_chunk;
using libbitcoin::base16_literal;
using libbitcoin::data_source;
using libbitcoin::istream_reader;

TEST_CASE("[test_get_keoken_output_empty]") {
    
   data_chunk raw_tx = to_chunk(base16_literal(
        "0100000001f08e44a96bfb5ae63eda1a6620adae37ee37ee4777fb0336e1bbbc"
        "4de65310fc010000006a473044022050d8368cacf9bf1b8fb1f7cfd9aff63294"
        "789eb1760139e7ef41f083726dadc4022067796354aba8f2e02363c5e510aa7e"
        "2830b115472fb31de67d16972867f13945012103e589480b2f746381fca01a9b"
        "12c517b7a482a203c8b2742985da0ac72cc078f2ffffffff02f0c9c467000000"
        "001976a914d9d78e26df4e4601cf9b26d09c7b280ee764469f88ac80c4600f00"
        "0000001976a9141ee32412020a324b93b1a1acfdfff6ab9ca8fac288ac000000"
        "00"));

    libbitcoin::chain::transaction tx;
    tx.from_data(raw_tx);
    
    CHECK(tx.is_valid()); 
    auto ret = first_keoken_output(tx);
    CHECK(ret.empty());
}


TEST_CASE("[test_get_keoken_output_non_empty]") {
    
    data_chunk raw_tx = to_chunk(base16_literal("0100000001bd56eab5f51d3d888f72c3e88187dc6cbd0b1abeefbe2348912619301a9e489f000000006b4830450221009a89bf0c34b87154fc4eb3e99a6e044ae21e76e244264645e8de4a747f6989dc02205d350d3113af2ce3cb013f4931c9f4c34d5925d9ffc76e56272befd9f47b521a412102bbfc0ef6f18b7594a930e2dd4e05bb90fbe7be60f58fbc8829f4fda9580af72dffffffff02606b042a010000001976a91456233da90fa320a56359161d02a9eed76b6157c088ac00000000000000001b6a0400004b5014000000014269747072696d0000000000000f424000000000"));

    libbitcoin::chain::transaction tx;
    tx.from_data(raw_tx);
    
    CHECK(tx.is_valid()); 
    auto ret = first_keoken_output(tx);
    CHECK( ! ret.empty());
}


TEST_CASE("[test_get_keoken_output_create_asset_valid]") {
    
    data_chunk raw_tx = to_chunk(base16_literal("0100000001bd56eab5f51d3d888f72c3e88187dc6cbd0b1abeefbe2348912619301a9e489f000000006b4830450221009a89bf0c34b87154fc4eb3e99a6e044ae21e76e244264645e8de4a747f6989dc02205d350d3113af2ce3cb013f4931c9f4c34d5925d9ffc76e56272befd9f47b521a412102bbfc0ef6f18b7594a930e2dd4e05bb90fbe7be60f58fbc8829f4fda9580af72dffffffff02606b042a010000001976a91456233da90fa320a56359161d02a9eed76b6157c088ac00000000000000001b6a0400004b5014000000014269747072696d0000000000000f424000000000"));

    libbitcoin::chain::transaction tx;
    tx.from_data(raw_tx);
    
    CHECK(tx.is_valid()); 
    auto ret = first_keoken_output(tx);
    CHECK( ! ret.empty());

    data_source ds(ret);
    istream_reader source(ds);

    auto version = source.read_2_bytes_big_endian();
    CHECK(version == 0);
    
    auto type = source.read_2_bytes_big_endian();
    CHECK(type == 1);
    
    auto msg = message::create_asset::factory_from_data(source);
    auto name = msg.name();
    CHECK(name == "Bitprim");

    auto amount = msg.amount();
    CHECK(amount == 1000000);
}


// #include <bitcoin/bitcoin/multi_crypto_support.hpp>
// //#include <bitcoin/bitcoin/wallet/cashaddr.hpp>
// #include <bitcoin/bitcoin/wallet/payment_address.hpp>

// using namespace libbitcoin::wallet;
// using namespace std;

// #ifdef BITPRIM_CURRENCY_BCH
// TEST_CASE("[payment_address__construct__payment__valid_expected] payment_address__construct__payment__valid_expected") {

//     libbitcoin::set_bitcoin_cash(true);
//     std::vector<std::pair<std::string, std::string>> cases = {
//         {"1BpEi6DfDAUFd7GtittLSdBeYJvcoaVggu", "bitcoincash:qpm2qsznhks23z7629mms6s4cwef74vcwvy22gdx6a"},
//         {"1KXrWXciRDZUpQwQmuM1DbwsKDLYAYsVLR", "bitcoincash:qr95sy3j9xwd2ap32xkykttr4cvcu7as4y0qverfuy"},
//         {"16w1D5WRVKJuZUsSRzdLp9w3YGcgoxDXb", "bitcoincash:qqq3728yw0y47sqn6l2na30mcw6zm78dzqre909m2r"},
//         {"3CWFddi6m4ndiGyKqzYvsFYagqDLPVMTzC", "bitcoincash:ppm2qsznhks23z7629mms6s4cwef74vcwvn0h829pq"},
//         {"3LDsS579y7sruadqu11beEJoTjdFiFCdX4", "bitcoincash:pr95sy3j9xwd2ap32xkykttr4cvcu7as4yc93ky28e"},
//         {"31nwvkZwyPdgzjBJZXfDmSWsC4ZLKpYyUw", "bitcoincash:pqq3728yw0y47sqn6l2na30mcw6zm78dzq5ucqzc37"}
//     };

//     for (auto&& c : cases) {
//         payment_address const from_legacy(c.first);
//         payment_address const from_cashaddr(c.second);
//         CHECK(from_legacy.encoded() == from_cashaddr.encoded());
//         CHECK(from_legacy.encoded_cashaddr() == from_cashaddr.encoded_cashaddr());
//     }
// }
// #endif //BITPRIM_CURRENCY_BCH


