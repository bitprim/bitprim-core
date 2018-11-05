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
#ifndef BITPRIM_BUILT_IN_TYPE_HPP_
#define BITPRIM_BUILT_IN_TYPE_HPP_

// #include <cstddef>
// #include <cstdint>

namespace libbitcoin {

template <typename T>
struct built_in_t {
    constexpr
    built_in_t() = default;

    constexpr
    built_in_t(T x)
        : val_(x)
    {}

    constexpr
    // explicit
    operator T() const {
        return val_;
    }

private:
    T val_;
};

}  // namespace libbitcoin

#endif // BITPRIM_BUILT_IN_TYPE_HPP_
