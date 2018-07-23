/**
 * Copyright (c) 2017-2018 Bitprim Inc.
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
#include <bitprim/keoken/wallet/createtransaction.hpp>
#include <bitcoin/bitcoin/formats/base_16.hpp>
#include <bitcoin/bitcoin/wallet/transaction_functions.hpp>
#include <bitprim/keoken/message/base.hpp>
#include <bitprim/keoken/message/create_asset.hpp>
#include <bitprim/keoken/message/send_tokens.hpp>


namespace bitprim {
namespace keoken {
namespace wallet {

using namespace bc;

static bool push_scripts(libbitcoin::chain::output::list& outputs,
                         libbitcoin::config::output const& output, uint8_t script_version) {
  static constexpr uint64_t no_amount = 0;

  // explicit script
  if (!output.is_stealth() && output.script().is_valid()) {
    outputs.push_back({output.amount(), output.script()});
    return true;
  }

  // If it's not explicit the script must be a form of pay to short hash.
  if (output.pay_to_hash() == libbitcoin::null_short_hash)
    return false;

  libbitcoin::machine::operation::list payment_ops;
  const auto hash = output.pay_to_hash();

  // This presumes stealth versions are the same as non-stealth.
  if (output.version() != script_version)
    payment_ops = libbitcoin::chain::script::to_pay_key_hash_pattern(hash);
  else if (output.version() == script_version)
    payment_ops = libbitcoin::chain::script::to_pay_script_hash_pattern(hash);
  else
    return false;

  // If stealth add null data stealth output immediately before payment.
  if (output.is_stealth())
    outputs.push_back({no_amount, output.script()});

  outputs.push_back({output.amount(), {payment_ops}});
  return true;
}


inline
libbitcoin::chain::output create_keoken_message(libbitcoin::data_chunk const& keoken_message){
  libbitcoin::data_chunk header;
  libbitcoin::decode_base16(header,"00004b50");
  // Note: Adding an op_code using {data_chunk} automatically adds the size on front of the message
  libbitcoin::machine::operation::list op_codes = {
  {libbitcoin::machine::opcode::return_},
  {header},
  {keoken_message}
  };
  return {0, op_codes};

}

std::pair<libbitcoin::error::error_code_t, libbitcoin::chain::transaction> tx_encode_create_asset(libbitcoin::chain::input_point::list const& outputs_to_spend,
                                                                          libbitcoin::wallet::raw_output const& input_and_amount,
                                                                          std::string& name,
                                                                          bitprim::keoken::message::amount_t amount_tokens,
                                                                          uint32_t locktime /*= 0*/,
                                                                          uint32_t tx_version /*= 1*/,
                                                                          uint8_t script_version /*= 5*/) {

  libbitcoin::chain::transaction tx;
  tx.set_version(tx_version);
  tx.set_locktime(locktime);

  for (auto const &input: outputs_to_spend) {
    //TODO: move the elements instead of pushing back
    tx.inputs().push_back(libbitcoin::config::input(input));
  }

  std::string destiny_string = input_and_amount.first.encoded() + ":" + std::to_string(input_and_amount.second - 2000);
  if (!push_scripts(tx.outputs(), libbitcoin::config::output(destiny_string), script_version)) {
    return {libbitcoin::error::error_code_t::invalid_output, {}};
  }


  bitprim::keoken::message::create_asset asset{};
  asset.set_name(name);
  asset.set_amount(amount_tokens);

  tx.outputs().push_back(create_keoken_message(asset.to_data()));

  if (tx.is_locktime_conflict()) {
    return {libbitcoin::error::error_code_t::lock_time_conflict, {}};
  }

  return {libbitcoin::error::error_code_t::success, tx};
}


std::pair<libbitcoin::error::error_code_t, libbitcoin::chain::transaction> tx_encode_simple_send(libbitcoin::chain::input_point::list const& outputs_to_spend,
                                                                          libbitcoin::wallet::raw_output const& input_and_amount,
                                                                          libbitcoin::wallet::raw_output const& output_and_amount,
                                                                          bitprim::keoken::message::asset_id_t asset_id,
                                                                          bitprim::keoken::message::amount_t amount_tokens,
                                                                          uint32_t locktime /*= 0*/,
                                                                          uint32_t tx_version /*= 1*/,
                                                                          uint8_t script_version /*= 5*/) {

  libbitcoin::chain::transaction tx;
  tx.set_version(tx_version);
  tx.set_locktime(locktime);

  for (auto const &input: outputs_to_spend) {
    //TODO: move the elements instead of pushing back
    tx.inputs().push_back(libbitcoin::config::input(input));
  }

  std::string return_string = input_and_amount.first.encoded() + ":" + std::to_string(input_and_amount.second - output_and_amount.second - 2000);
  if (!push_scripts(tx.outputs(), libbitcoin::config::output(return_string), script_version)) {
    return {libbitcoin::error::error_code_t::invalid_output, {}};
  }

  std::string destiny_string = output_and_amount.first.encoded() + ":" + std::to_string(output_and_amount.second);
  if (!push_scripts(tx.outputs(), libbitcoin::config::output(destiny_string), script_version)) {
    return {libbitcoin::error::error_code_t::invalid_output, {}};
  }


  bitprim::keoken::message::send_tokens send_tokens{};
  send_tokens.set_asset(asset_id);
  send_tokens.set_amount(amount_tokens);

  tx.outputs().push_back(create_keoken_message(send_tokens.to_data()));

  if (tx.is_locktime_conflict()) {
    return {libbitcoin::error::error_code_t::lock_time_conflict, {}};
  }

  return {libbitcoin::error::error_code_t::success, tx};
}


// For internal use only
libbitcoin::error::error_code_t sign_and_set(libbitcoin::chain::script const& output_script,
                                             libbitcoin::ec_secret const& private_key,
                                             libbitcoin::wallet::ec_public const& public_key,
                                             uint64_t amount,
                                             libbitcoin::chain::transaction & tx) {

  auto sig = libbitcoin::wallet::input_signature_bch(private_key, output_script, tx, amount, 0);
  if (sig.first != libbitcoin::error::success) {
    return sig.first;
  }

  auto complete_tx = libbitcoin::wallet::input_set(sig.second, public_key, tx);
  if (complete_tx.first != libbitcoin::error::success) {
    return complete_tx.first;
  }
}

std::pair<libbitcoin::error::error_code_t,
                 libbitcoin::chain::transaction> create_asset_tx_complete(libbitcoin::chain::input_point const& output_to_spend,
                                                                          libbitcoin::chain::script const& output_script,
                                                                          libbitcoin::ec_secret const& private_key,
                                                                          libbitcoin::wallet::ec_public const& public_key,
                                                                          uint64_t amount,
                                                                          libbitcoin::wallet::payment_address const& addr,
                                                                          std::string& token_name,
                                                                          bitprim::keoken::message::amount_t amount_tokens) {
  uint64_t fee = 0;
  bool complete = false;
  // NOTE: the txns is created one time and then recreated using the calculated fee
  // TODO: calculate the txn size without the need of the signature

  while ( ! complete ) {
    //Prepare to create_asset
    libbitcoin::chain::input_point::list inputs;
    inputs.push_back(output_to_spend);

    libbitcoin::wallet::raw_output output = std::make_pair(addr, amount - fee);

    auto raw_tx = tx_encode_create_asset(inputs, output, token_name, amount_tokens);
    if ( raw_tx.first != libbitcoin::error::success ) {
      return {raw_tx.first, {}};
    }

    // Sign the transaction
    auto sign_and_set_result = sign_and_set (output_script, private_key, public_key, amount, raw_tx.second);
    if ( sign_and_set_result != libbitcoin::error::success) {
      return {sign_and_set_result, {}};
    }


    if ( fee == 0 ) {
      fee = raw_tx.second.serialized_size(true);
    } else {
      complete = true;
      return raw_tx;
    }
  }
}

std::pair<libbitcoin::error::error_code_t,
                 libbitcoin::chain::transaction> send_simple_tx_complete(libbitcoin::chain::input_point const& output_to_spend,
                                                                         libbitcoin::chain::script const& output_script,
                                                                         libbitcoin::ec_secret const& private_key,
                                                                         libbitcoin::wallet::ec_public const& public_key,
                                                                         uint64_t amount,
                                                                         libbitcoin::wallet::payment_address const& addr_origin,
                                                                         libbitcoin::wallet::payment_address const& addr_dest,
                                                                         bitprim::keoken::message::asset_id_t asset_id,
                                                                         bitprim::keoken::message::amount_t amount_tokens) {
  //TODO: dust should be a constant
  uint64_t dust = 2000;

  uint64_t fee = 0;
  bool complete = false;
  // NOTE: the txns is created one time and then recreated using the calculated fee
  // TODO: calculate the txn size without the need of the signature

  while ( ! complete ) {
    //Prepare to send_simple
    libbitcoin::chain::input_point::list inputs;
    inputs.push_back(output_to_spend);

    libbitcoin::wallet::raw_output output_origin_addr = std::make_pair(addr_origin, amount - fee - dust);
    libbitcoin::wallet::raw_output output_dest_addr = std::make_pair(addr_dest, dust);

    // Create raw transaction using the generated data
    auto raw_tx = tx_encode_simple_send(inputs, output_origin_addr, output_dest_addr, asset_id, amount_tokens);
    if ( raw_tx.first != libbitcoin::error::success ) {
      return {raw_tx.first, {}};
    }

    // Sign the transaction
    auto sign_and_set_result = sign_and_set (output_script, private_key, public_key, amount, raw_tx.second);
    if ( sign_and_set_result != libbitcoin::error::success ) {
      return {sign_and_set_result, {}};
    }

    // The first time calculate the fee and recreate, the second return the transaction
    if ( fee == 0 ) {
      fee = raw_tx.second.serialized_size(true);
    } else {
      complete = true;
      return raw_tx;
    }
  }
}






} // namespace wallet
} // namespace keoken
} // namespace bitprim
