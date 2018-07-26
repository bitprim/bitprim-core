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

libbitcoin::chain::output create_keoken_output(libbitcoin::data_chunk const &keoken_message) {

  // TODO: make prefix static
  libbitcoin::data_chunk header;
  libbitcoin::decode_base16(header, "00004b50");
  // Note: Adding an op_code using {data_chunk} automatically adds the size on front of the message
  libbitcoin::machine::operation::list op_codes = {
      {libbitcoin::machine::opcode::return_},
      {header},
      {keoken_message}
  };
  return {0, op_codes};
}

std::pair<libbitcoin::error::error_code_t,
          libbitcoin::chain::transaction> tx_encode_create_asset(libbitcoin::chain::input_point::list const &outputs_to_spend,
                                                                 libbitcoin::wallet::payment_address const &asset_owner,
                                                                 uint64_t const &utxo_satoshis,
                                                                 std::string &name,
                                                                 bitprim::keoken::message::amount_t amount_tokens,
                                                                 uint32_t locktime /*= 0*/,
                                                                 uint32_t tx_version /*= 1*/,
                                                                 uint8_t script_version /*= 5*/) {

  bitprim::keoken::message::create_asset asset{};
  asset.set_name(name);
  asset.set_amount(amount_tokens);

  libbitcoin::wallet::raw_output_list outputs;
  outputs.push_back({asset_owner, (utxo_satoshis - keoken::wallet::fees)});

  return libbitcoin::wallet::tx_encode(outputs_to_spend,
                                       outputs,
                                       {create_keoken_output(asset.to_data())},
                                       locktime,
                                       tx_version,
                                       script_version);

}

std::pair<libbitcoin::error::error_code_t,
          libbitcoin::chain::transaction> tx_encode_send_token(libbitcoin::chain::input_point::list const &outputs_to_spend,
                                                               libbitcoin::wallet::payment_address const &token_owner,
                                                               uint64_t const &utxo_satoshis,
                                                               libbitcoin::wallet::payment_address const &token_receiver,
                                                               uint64_t const &dust,
                                                               bitprim::keoken::message::asset_id_t asset_id,
                                                               bitprim::keoken::message::amount_t amount_tokens,
                                                               uint32_t locktime,
                                                               uint32_t tx_version,
                                                               uint8_t script_version) {
  libbitcoin::wallet::raw_output_list outputs;
  outputs.push_back({token_receiver, dust});
  outputs.push_back({token_owner, utxo_satoshis - dust - keoken::wallet::fees});

  bitprim::keoken::message::send_tokens send_tokens{};
  send_tokens.set_asset_id(asset_id);
  send_tokens.set_amount(amount_tokens);

  return libbitcoin::wallet::tx_encode(outputs_to_spend,
                                       outputs,
                                       {create_keoken_output(send_tokens.to_data())},
                                       locktime,
                                       tx_version,
                                       script_version);
}

// For internal use only
std::pair<libbitcoin::error::error_code_t,
          libbitcoin::chain::transaction> sign_and_set(libbitcoin::chain::script const &output_script,
                                                       libbitcoin::ec_secret const &private_key,
                                                       libbitcoin::wallet::ec_public const &public_key,
                                                       uint64_t amount,
                                                       libbitcoin::chain::transaction &tx) {

  auto sig = libbitcoin::wallet::input_signature_bch(private_key, output_script, tx, amount, 0);
  if (sig.first != libbitcoin::error::success) {
    return {sig.first, {}};
  }

  return libbitcoin::wallet::input_set(sig.second, public_key, tx);
}

std::pair<libbitcoin::error::error_code_t,
          libbitcoin::chain::transaction> create_asset_tx_complete(libbitcoin::chain::input_point const &output_to_spend,
                                                                   libbitcoin::chain::script const &output_script,
                                                                   libbitcoin::ec_secret const &private_key,
                                                                   libbitcoin::wallet::ec_public const &public_key,
                                                                   uint64_t amount,
                                                                   libbitcoin::wallet::payment_address const &addr,
                                                                   std::string &token_name,
                                                                   bitprim::keoken::message::amount_t amount_tokens) {
  auto raw_tx = tx_encode_create_asset({output_to_spend}, addr, amount, token_name, amount_tokens);
  if (raw_tx.first != libbitcoin::error::success) {
    return {raw_tx.first, {}};
  }

  // Sign the transaction
  auto sign_and_set_result = sign_and_set(output_script, private_key, public_key, amount, raw_tx.second);
  if (sign_and_set_result.first != libbitcoin::error::success) {
    return {sign_and_set_result.first, {}};
  }

  return {libbitcoin::error::success, sign_and_set_result.second};
}

std::pair<libbitcoin::error::error_code_t,
          libbitcoin::chain::transaction> send_token_tx_complete(libbitcoin::chain::input_point const &output_to_spend,
                                                                 libbitcoin::chain::script const &output_script,
                                                                 libbitcoin::ec_secret const &private_key,
                                                                 libbitcoin::wallet::ec_public const &public_key,
                                                                 uint64_t amount,
                                                                 libbitcoin::wallet::payment_address const &addr_origin,
                                                                 libbitcoin::wallet::payment_address const &addr_dest,
                                                                 uint64_t const &dust,
                                                                 bitprim::keoken::message::asset_id_t asset_id,
                                                                 bitprim::keoken::message::amount_t amount_tokens) {

  // Create raw transaction using the generated data
  auto raw_tx = tx_encode_send_token({output_to_spend}, addr_origin, amount, addr_dest, dust, asset_id, amount_tokens);
  if (raw_tx.first != libbitcoin::error::success) {
    return {raw_tx.first, {}};
  }

  // Sign the transaction
  auto sign_and_set_result = sign_and_set(output_script, private_key, public_key, amount, raw_tx.second);
  if (sign_and_set_result.first != libbitcoin::error::success) {
    return {sign_and_set_result.first, {}};
  }

  return {libbitcoin::error::success, sign_and_set_result.second};
}

} // namespace wallet
} // namespace keoken
} // namespace bitprim
