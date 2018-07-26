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
#ifndef BITPRIM_KEOKEN_UTILITY_HPP_
#define BITPRIM_KEOKEN_UTILITY_HPP_

#include <bitcoin/bitcoin/chain/transaction.hpp>
#include <bitcoin/bitcoin/utility/data.hpp>

namespace bitprim {
namespace keoken {

//C++11
template <typename E>
constexpr 
typename std::underlying_type<E>::type to_underlying(E e) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

// //C++14
// template <typename E>
// constexpr auto to_underlying(E e) noexcept {
//     return static_cast<std::underlying_type_t<E>>(e);
// }

} // namespace keoken
} // namespace bitprim

#endif //BITPRIM_KEOKEN_UTILITY_HPP_
