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
#ifndef BITPRIM_KEOKEN_CONSTANTS_HPP_
#define BITPRIM_KEOKEN_CONSTANTS_HPP_

#include <bitcoin/bitcoin/machine/opcode.hpp>
#include <bitcoin/bitcoin/machine/operation.hpp>

#include <bitprim/keoken/utility.hpp>

namespace bitprim {
namespace keoken {

//TODO(fernando): move to constants
uint8_t const protocol_name[] = {0x00, 0x00, 0x4b, 0x50};   //"  KP"
// constexpr size_t kp_min_size = 4; //TODO(fernando): define
constexpr size_t kp_min_size = to_underlying(libbitcoin::machine::opcode::push_size_4);
constexpr size_t kp_max_size = to_underlying(libbitcoin::machine::opcode::push_four_size);

} // namespace keoken
} // namespace bitprim

#endif //BITPRIM_KEOKEN_CONSTANTS_HPP_
