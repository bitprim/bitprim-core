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
#include <boost/test/unit_test.hpp>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/bitcoin/wallet/ec_public.hpp>
#include <bitcoin/bitcoin/wallet/transaction_functions.hpp>

using namespace bc;
using namespace bc::wallet;

BOOST_AUTO_TEST_SUITE(transaction_functions_tests)

#define SEED "fffb587496cc54912bbcef874fa9a61a"
#define WALLET "mwx2YDHgpdfHUmCpFjEi9LarXf7EkQN6YG"
#define TX_ENCODE "01000000019373b022dfb99400ee40b8987586aea9e158f3b0c62343d59896c212cee60d980100000000ffffffff0118beeb0b000000001976a914b43ff4532569a00bcab4ce60f87cdeebf985b69a88ac00000000"
#define SIGNATURE "30440220433c405e4cb7698ad5f58e0ea162c3c3571d46d96ff1b3cb9232a06eba3b444d02204bc5f48647c0f052ade7cf85eac3911f7afbfa69fa5ebd92084191a5da33f88d41"
#define COMPLETE_TX "01000000019373b022dfb99400ee40b8987586aea9e158f3b0c62343d59896c212cee60d98010000006a4730440220433c405e4cb7698ad5f58e0ea162c3c3571d46d96ff1b3cb9232a06eba3b444d02204bc5f48647c0f052ade7cf85eac3911f7afbfa69fa5ebd92084191a5da33f88d4121027a45d4abb6ebb00214796e2c7cf61d18c9185ba771fe9ed75b303eb7a8e9028bffffffff0118beeb0b000000001976a914b43ff4532569a00bcab4ce60f87cdeebf985b69a88ac00000000"

// Helpers to replicate the bitprim-dojo functionality

libbitcoin::ec_secret create_secret_from_seed(std::string const& seed_str) {
  libbitcoin::data_chunk seed;
  libbitcoin::decode_base16(seed, seed_str);
  libbitcoin::wallet::hd_private const key(seed);
  // Secret key
  libbitcoin::ec_secret secret_key(key.secret());
  return secret_key;
}

libbitcoin::wallet::ec_public secret_to_compressed_public(libbitcoin::ec_secret const& secret_key) {
  //Public key
  libbitcoin::ec_compressed point;
  libbitcoin::secret_to_public(point, secret_key);
  libbitcoin::wallet::ec_public public_key(point, true /*compress*/);

  return public_key;
}

BOOST_AUTO_TEST_CASE(seed_to_wallet_compressed__test) {
  auto secret = create_secret_from_seed(SEED);
  auto pub_key = secret_to_compressed_public(secret);
  // Payment Address
  uint8_t const version = libbitcoin::wallet::payment_address::testnet_p2kh; // testnet_p2sh
  libbitcoin::wallet::payment_address address(pub_key, version);

  BOOST_REQUIRE_EQUAL(address.encoded(), WALLET);
}

BOOST_AUTO_TEST_CASE(create_transaction__test) {
  // List of inputs (outputs_to_spend)
  std::vector<chain::input_point> outputs_to_spend;
  libbitcoin::hash_digest hash_to_spend;
  libbitcoin::decode_hash(hash_to_spend, "980de6ce12c29698d54323c6b0f358e1a9ae867598b840ee0094b9df22b07393");
  int const index_to_spend = 1;
  outputs_to_spend.push_back({hash_to_spend, index_to_spend});

  // List of outputs
  std::vector<std::pair<payment_address, uint64_t>> outputs;
  outputs.push_back({payment_address(WALLET), 199999000});

  auto result = libbitcoin::wallet::tx_encode(outputs_to_spend, outputs);

  BOOST_REQUIRE_EQUAL(result.first, error::error_code_t::success);
  BOOST_REQUIRE_EQUAL(libbitcoin::encode_base16(result.second.to_data()), TX_ENCODE);
}

// TODO: make test for BTC and LTC signatures

#ifdef BITPRIM_CURRENCY_BCH
BOOST_AUTO_TEST_CASE(sign_transaction__test) {
  // Priv key
  auto const private_key = create_secret_from_seed(SEED);
  // Script
  libbitcoin::data_chunk raw_script;
  libbitcoin::decode_base16(raw_script, "76a914b43ff4532569a00bcab4ce60f87cdeebf985b69a88ac");
  chain::script output_script(raw_script, false);
  // TX
  chain::transaction tx;
  data_chunk raw_data;
  libbitcoin::decode_base16(raw_data, TX_ENCODE);
  tx.from_data(raw_data);
  BOOST_REQUIRE_EQUAL(libbitcoin::encode_base16(tx.to_data()), TX_ENCODE);
  // Amount
  uint64_t const amount = 200000000;
  // Index
  int const index = 0;
  // Create signature
  auto const result = input_signature_bch(private_key, output_script, tx, amount, index);

  BOOST_REQUIRE_EQUAL(result.first, error::error_code_t::success);
  BOOST_REQUIRE_EQUAL(libbitcoin::encode_base16(result.second), SIGNATURE);
}
#endif

BOOST_AUTO_TEST_CASE(set_signature__test) {

  // TX
  chain::transaction tx;
  data_chunk raw_data;
  libbitcoin::decode_base16(raw_data, TX_ENCODE);
  tx.from_data(raw_data);

  // SCRIPT
  auto secret = create_secret_from_seed(SEED);
  auto pub_key = secret_to_compressed_public(secret);
  // Redeem script for P2KH [SIGNATURE][PUBKEY]
  chain::script input_script;
  input_script.from_string("[" + std::string(SIGNATURE) + "] [" + pub_key.encoded() + "]");

  // SET THE INPUT
  auto const result = input_set(input_script, tx);
  BOOST_REQUIRE_EQUAL(result.first, error::error_code_t::success);
  BOOST_REQUIRE_EQUAL(libbitcoin::encode_base16(result.second.to_data()), COMPLETE_TX);
}

BOOST_AUTO_TEST_SUITE_END()
