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
#include <bitcoin/bitcoin/chainv2/point_iterator.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <utility>
#include <bitcoin/bitcoin/chainv2/point.hpp>
#include <bitcoin/bitcoin/constants.hpp>
#include <bitcoin/bitcoin/utility/assert.hpp>
////#include <bitcoin/bitcoin/utility/endian.hpp>
#include <bitcoin/bitcoin/math/limits.hpp>

namespace libbitcoin { namespace chainv2 {

static constexpr auto point_size = static_cast<size_t>(std::tuple_size<point>::value);

// Constructors.
//-----------------------------------------------------------------------------

point_iterator::point_iterator(point const& value, size_t index)
    : point_(value), current_(index)
{}


// point_iterator::point_iterator(point_iterator const& other)
//     : point_(other.point_)
//     , current_(other.current_)
// {}

// Operators.
//-----------------------------------------------------------------------------

// point_iterator::operator bool() const {
//     return current_ < point_size;
// }

// private
uint8_t point_iterator::current() const {
    if (current_ < hash_size) {
        return point_.hash()[current_];
    }

    // TODO(libbitcoin): move the little-endian iterator into endian.hpp.
    const auto position = current_ - hash_size;
    return static_cast<uint8_t>(point_.index() >> (position * byte_bits));
}

point_iterator::pointer point_iterator::operator->() const {
    return current();
}

point_iterator::reference point_iterator::operator*() const {
    return current();
}

point_iterator::iterator& point_iterator::operator++() {
    increment();
    return *this;
}

point_iterator::iterator point_iterator::operator++(int) {
    auto it = *this;
    increment();
    return it;
}

point_iterator::iterator& point_iterator::operator--() {
    decrement();
    return *this;
}

point_iterator::iterator point_iterator::operator--(int) {
    auto it = *this;
    decrement();
    return it;
}

point_iterator point_iterator::operator+(difference_type n) const {
    return n < 0 ? decrease(static_cast<size_t>(std::abs(n))) : increase(n);
}

point_iterator point_iterator::operator-(difference_type n) const {
    return n < 0 ? increase(static_cast<size_t>(std::abs(n))) : decrease(n);
}

bool point_iterator::operator==(const point_iterator& other) const {
    return (current_ == other.current_) && (&point_ == &other.point_);
}

bool point_iterator::operator!=(const point_iterator& other) const {
    return !(*this == other);
}

// Utilities.
//-----------------------------------------------------------------------------
// private

void point_iterator::increment() {
    if (current_ < point_size) {
        current_++;
    }
}

void point_iterator::decrement() {
    if (current_ > 0) {
        current_--;
    }
}

point_iterator point_iterator::increase(size_t n) const {
    const auto index = ceiling_add(current_, n);
    return point_iterator(point_, std::max(index, point_size));
}

point_iterator point_iterator::decrease(size_t n) const {
    const auto index = floor_subtract(current_, n);
    return point_iterator(point_, index);
}

}} // namespace libbitcoin::chain
