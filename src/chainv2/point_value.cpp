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
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

#include <bitcoin/bitcoin/chainv2/point.hpp>
#include <bitcoin/bitcoin/chainv2/point_value.hpp>

namespace libbitcoin { namespace chainv2 {

static_assert(std::is_move_constructible<point_value>::value, "std::is_move_constructible<point_value>::value");
static_assert(std::is_nothrow_move_constructible<point_value>::value, "std::is_nothrow_move_constructible<point_value>::value");
static_assert(std::is_move_assignable<point_value>::value, "std::is_move_assignable<point_value>::value");
static_assert(std::is_nothrow_move_assignable<point_value>::value, "std::is_nothrow_move_assignable<point_value>::value");
static_assert(std::is_copy_constructible<point_value>::value, "std::is_copy_constructible<point_value>::value");
static_assert(std::is_copy_assignable<point_value>::value, "std::is_copy_assignable<point_value>::value");
static_assert(std::is_swappable<point_value>::value, "std::is_swappable<point_value>::value");
static_assert(std::is_nothrow_swappable<point_value>::value, "std::is_nothrow_swappable<point_value>::value");

// Constructors.
//-------------------------------------------------------------------------

point_value::point_value()
    : value_(0)
{}

point_value::point_value(point&& instance, uint64_t value)
    : point(instance), value_(value)
{}

point_value::point_value(const point& instance, uint64_t value)
    : point(instance), value_(value)
{}

// Operators.
//-------------------------------------------------------------------------
// friend
bool operator==(point_value const& a, point_value const& b) {
    return static_cast<point const&>(a) == static_cast<point const&>(b) && (a.value_ == b.value_);
}

// friend
bool operator!=(point_value const& a, point_value const& b) {
    return !(a == b);
}

// // friend
// void swap(point_value& a, point_value& b) {
//     using std::swap;
//     swap(static_cast<point&>(a), static_cast<point&>(b));
//     swap(a.value_, b.value_);
// }

// Properties (accessors).
//-------------------------------------------------------------------------

uint64_t point_value::value() const {
    return value_;
}

void point_value::set_value(uint64_t value) {
    value_ = value;
}

}} // namespace libbitcoin::chainv2
