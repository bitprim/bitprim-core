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
#include <bitprim/keoken/wallet/create_transaction.hpp>

#include <bitcoin/bitcoin/formats/base_16.hpp>
#include <bitcoin/bitcoin/wallet/transaction_functions.hpp>

#include <bitprim/keoken/message/base.hpp>
#include <bitprim/keoken/message/create_asset.hpp>
#include <bitprim/keoken/message/send_tokens.hpp>

namespace bitprim {
namespace keoken {
namespace wallet {

// using namespace bc;
using libbitcoin::chain::output;
using libbitcoin::chain::input_point;
using libbitcoin::chain::script;
using libbitcoin::chain::transaction;
using libbitcoin::data_chunk;
using libbitcoin::ec_secret;
using libbitcoin::error::success;
using libbitcoin::wallet::tx_encode;
using libbitcoin::wallet::payment_address;
using libbitcoin::wallet::ec_public;
using libbitcoin::wallet::raw_output_list;

namespace detail {
// For internal use only
result_t sign_and_set(script const& output_script,
                      ec_secret const& private_key,
                      ec_public const& public_key,
                      uint64_t amount,
                      transaction & tx) {

    auto sig = libbitcoin::wallet::input_signature_bch(private_key, output_script, tx, amount, 0);
    if (sig.first != success) {
        return {sig.first, {}};
    }

    return libbitcoin::wallet::input_set(sig.second, public_key, tx);
}

} // namespace detail

output create_keoken_output(data_chunk const& keoken_message){
    data_chunk header;
    libbitcoin::decode_base16(header,"00004b50");

    // Note: Adding an op_code using {data_chunk} automatically adds the size on front of the message
    libbitcoin::machine::operation::list op_codes = {
        {libbitcoin::machine::opcode::return_},
        {header},
        {keoken_message}
    };

    return {0, op_codes};
}

result_t tx_encode_create_asset(input_point::list const& outputs_to_spend,
                                payment_address const& asset_owner,
                                uint64_t utxo_satoshis,
                                std::string const& asset_name,
                                amount_t asset_amount,
                                uint32_t locktime /*= 0*/,
                                uint32_t tx_version /*= 1*/,
                                uint8_t script_version /*= 5*/) {

    message::create_asset asset(asset_name, asset_amount);

    raw_output_list outputs {
        {asset_owner, (utxo_satoshis - wallet::fees)}
    };

    return tx_encode(outputs_to_spend, outputs, {create_keoken_output(asset.to_data())}, locktime, tx_version, script_version);
}

result_t tx_encode_send_token(input_point::list const& outputs_to_spend,
                              payment_address const& asset_owner,
                              uint64_t utxo_satoshis,
                              payment_address const& asset_receiver,
                              uint64_t dust,
                              asset_id_t asset_id,
                              amount_t asset_amount,
                              uint32_t locktime,
                              uint32_t tx_version,
                              uint8_t script_version) {

    raw_output_list outputs {
        {asset_receiver, dust},
        {asset_owner, utxo_satoshis - dust - wallet::fees}
    };

    message::send_tokens send_tokens(asset_id, asset_amount);
    return tx_encode(outputs_to_spend, outputs, {create_keoken_output(send_tokens.to_data())}, locktime, tx_version, script_version);
}

result_t create_asset_tx_complete(input_point const& output_to_spend,
                                  script const& output_script,
                                  ec_secret const& private_key,
                                  ec_public const& public_key,
                                  uint64_t amount,
                                  payment_address const& addr,
                                  std::string const& asset_name,
                                  amount_t asset_amount) {

    auto raw_tx = tx_encode_create_asset({output_to_spend}, addr, amount, asset_name, asset_amount);
    if (raw_tx.first != success ) {
        return {raw_tx.first, {}};
    }

    // Sign the transaction
    auto sign_and_set_result = detail::sign_and_set(output_script, private_key, public_key, amount, raw_tx.second);
    if (sign_and_set_result.first != success) {
        return {sign_and_set_result.first, {}};
    }

    return {success, sign_and_set_result.second};
}

result_t send_token_tx_complete(input_point const& output_to_spend,
                                script const& output_script,
                                ec_secret const& private_key,
                                ec_public const& public_key,
                                uint64_t amount,
                                payment_address const& addr_origin,
                                payment_address const& addr_dest,
                                uint64_t dust,
                                asset_id_t asset_id,
                                amount_t asset_amount) {

    // Create raw transaction using the generated data
    auto raw_tx = tx_encode_send_token({output_to_spend}, addr_origin, amount, addr_dest, dust, asset_id, asset_amount);
    if (raw_tx.first != success ) {
        return {raw_tx.first, {}};
    }

    // Sign the transaction
    auto sign_and_set_result = detail::sign_and_set(output_script, private_key, public_key, amount, raw_tx.second);
    if (sign_and_set_result.first != success) {
        return {sign_and_set_result.first, {}};
    }

    return {success, sign_and_set_result.second};
}

} // namespace wallet
} // namespace keoken
} // namespace bitprim
