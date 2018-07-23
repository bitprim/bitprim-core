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


#ifndef BITPRIM_TRANSACTION_FUNCTIONS_HPP
#define BITPRIM_TRANSACTION_FUNCTIONS_HPP

#include <string>
#include <utility>
#include <vector>
#include <bitcoin/bitcoin.hpp>

namespace libbitcoin {
namespace wallet {


using raw_output = std::pair<payment_address, uint64_t>;
using raw_output_list = std::vector<raw_output>;

BC_API std::pair<error::error_code_t, chain::transaction> tx_encode(chain::input_point::list const& outputs_to_spend,
                                               raw_output_list const& destiny_and_amount,
                                               std::vector<libbitcoin::chain::output> const& extra_outputs,
                                               uint32_t locktime = 0,
                                               uint32_t tx_version = 1,
                                               uint8_t script_version = 5);

BC_API std::pair<error::error_code_t, chain::transaction> tx_encode(chain::input_point::list const& outputs_to_spend,
                                               raw_output_list const& destiny_and_amount,
                                               uint32_t locktime = 0,
                                               uint32_t tx_version = 1,
                                               uint8_t script_version = 5);

BC_API std::pair<error::error_code_t, data_chunk> input_signature_old(libbitcoin::ec_secret const& private_key,
                                                                      chain::script const& output_script,
                                                                      chain::transaction const& tx,
                                                                      uint32_t index,
                                                                      uint8_t sign_type = 0x01,
                                                                      bool anyone_can_pay = false);

BC_API std::pair<error::error_code_t, data_chunk> input_signature_btc(libbitcoin::ec_secret const& private_key,
                                                                      chain::script const& output_script,
                                                                      chain::transaction const& tx,
                                                                      uint64_t amount,
                                                                      uint32_t index,
                                                                      uint8_t sign_type = 0x01,
                                                                      bool anyone_can_pay = false);

BC_API std::pair<error::error_code_t, data_chunk> input_signature_bch(libbitcoin::ec_secret const& private_key,
                                                                      chain::script const& output_script,
                                                                      chain::transaction const& tx,
                                                                      uint64_t amount,
                                                                      uint32_t index,
                                                                      uint8_t sign_type = 0x01,
                                                                      bool anyone_can_pay = false);

BC_API std::pair<error::error_code_t, chain::transaction> input_set(chain::script const& script,
                                                                    chain::transaction const& raw_tx,
                                                                    uint32_t index = 0);

BC_API std::pair<error::error_code_t, chain::transaction> input_set(data_chunk const& signature,
                                                                    wallet::ec_public const& public_key,
                                                                    chain::transaction const& tx, uint32_t index = 0);

} //end namespace wallet
} //end namespace libbitcoin

#endif //BITPRIM_TRANSACTION_FUNCTIONS_HPP