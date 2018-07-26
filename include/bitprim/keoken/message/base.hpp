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
#ifndef BITPRIM_KEOKEN_MESSAGE_BASE_HPP_
#define BITPRIM_KEOKEN_MESSAGE_BASE_HPP_

#include <bitcoin/bitcoin/utility/writer.hpp>

// Platform check.
static_assert(std::is_same<std::uint8_t, char>::value || std::is_same<std::uint8_t, unsigned char>::value,
    "Bitprim requires std::uint8_t to be implemented as char or unsigned char.");

namespace bitprim {
namespace keoken {
namespace message {
namespace base {

size_t serialized_size();
void to_data(libbitcoin::writer& sink, uint16_t version, uint16_t type);

} // namespace base
} // namespace message
} // namespace keoken
} // namespace bitprim

#endif //BITPRIM_KEOKEN_MESSAGE_BASE_HPP_
