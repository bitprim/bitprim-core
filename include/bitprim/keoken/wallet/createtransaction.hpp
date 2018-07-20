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
#ifndef BITPRIM_KEOKEN_WALLET_CREATETRANSACTION_HPP_
#define BITPRIM_KEOKEN_WALLET_CREATETRANSACTION_HPP_

#include <bitcoin/bitcoin/define.hpp>
#include <bitprim/keoken/message/base.hpp>
#include <bitcoin/bitcoin/chain/transaction.hpp>
#include <bitcoin/bitcoin/chain/input.hpp>
#include <bitcoin/bitcoin/chain/output.hpp>
#include <bitcoin/bitcoin/chain/input_point.hpp>
#include <bitcoin/bitcoin/chain/output_point.hpp>
#include <bitcoin/bitcoin/config/output.hpp>
#include <bitcoin/bitcoin/config/input.hpp>
#include <bitcoin/bitcoin/config/input.hpp>
#include <bitcoin/bitcoin/wallet/transaction_functions.hpp>

namespace bitprim {
namespace keoken {
namespace wallet {

BC_API std::pair<libbitcoin::error::error_code_t, libbitcoin::chain::transaction> tx_encode_create_asset(libbitcoin::chain::input_point::list const& outputs_to_spend,
                                                                                libbitcoin::wallet::raw_output const& input_and_amount,
                                                                                std::string& name,
                                                                                bitprim::keoken::message::amount_t amount_tokens,
                                                                                uint32_t locktime = 0,
                                                                                uint32_t tx_version = 1,
                                                                                uint8_t script_version = 5);


BC_API std::pair<libbitcoin::error::error_code_t, libbitcoin::chain::transaction> tx_encode_simple_send(libbitcoin::chain::input_point::list const& outputs_to_spend,
                                                                                       libbitcoin::wallet::raw_output const& input_and_amount,
                                                                                       libbitcoin::wallet::raw_output const& output_and_amount,
                                                                                       bitprim::keoken::message::asset_id_t asset_id,
                                                                                       bitprim::keoken::message::amount_t amount_tokens,
                                                                                       uint32_t locktime = 0,
                                                                                       uint32_t tx_version = 1,
                                                                                       uint8_t script_version = 5);


} // namespace message
} // namespace keoken
} // namespace bitprim

#endif //BITPRIM_KEOKEN_WALLET_CREATETRANSACTION_HPP_
