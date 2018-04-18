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
#include <bitcoin/bitcoin/chainv2/output_point.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>
#include <bitcoin/bitcoin/constants.hpp>
#include <bitcoin/bitcoin/chainv2/point.hpp>

namespace libbitcoin { namespace chainv2 {

static_assert(std::is_move_constructible<output_point>::value, "std::is_move_constructible<output_point>::value");
static_assert(std::is_nothrow_move_constructible<output_point>::value, "std::is_nothrow_move_constructible<output_point>::value");
static_assert(std::is_move_assignable<output_point>::value, "std::is_move_assignable<output_point>::value");
static_assert(std::is_nothrow_move_assignable<output_point>::value, "std::is_nothrow_move_assignable<output_point>::value");
static_assert(std::is_copy_constructible<output_point>::value, "std::is_copy_constructible<output_point>::value");
static_assert(std::is_copy_assignable<output_point>::value, "std::is_copy_assignable<output_point>::value");


// Constructors.
//-----------------------------------------------------------------------------

output_point::output_point(point const& value)
    : point(value)
{}

output_point::output_point(hash_digest const& hash, uint32_t index)
    : point(hash, index)
{}

// output_point::output_point(output_point&& other) noexcept
//     : point(other), validation(std::move(other.validation))
// {}


// Operators.
//-----------------------------------------------------------------------------

output_point& output_point::operator=(point const& other) {
    point::operator=(other);
    return *this;
}

// output_point& output_point::operator=(output_point&& other) noexcept {
//     point::operator=(other);
//     validation = std::move(other.validation);
//     return *this;
// }


// friend
bool operator==(output_point const& a, output_point const& b) {
    return static_cast<point const&>(a) == static_cast<point const&>(b);
}

// friend
bool operator!=(output_point const& a, output_point const& b) {
    return !(a == b);
}

// friend
bool operator==(output_point const& a, point const& b) {
    return static_cast<point const&>(a) == b;
}

// friend
bool operator!=(output_point const& a, point const& b) {
    return !(a == b);
}

// friend
bool operator==(point const& a, output_point const& b) {
    return a == static_cast<point const&>(b);
}

// friend
bool operator!=(point const& a, output_point const& b) {
    return !(a == b);
}


// Deserialization.
//-----------------------------------------------------------------------------

output_point output_point::factory_from_data(const data_chunk& data, bool wire) {
    output_point instance;
    instance.from_data(data, wire);
    return instance;
}

output_point output_point::factory_from_data(std::istream& stream, bool wire) {
    output_point instance;
    instance.from_data(stream, wire);
    return instance;
}

output_point output_point::factory_from_data(reader& source, bool wire) {
    output_point instance;
    instance.from_data(source, wire);
    return instance;
}

// Validation.
//-----------------------------------------------------------------------------

// For tx pool validation height is that of the candidate block.
bool output_point::is_mature(size_t height) const {
    // Coinbase (null) inputs and those with non-coinbase prevouts are mature.
    if (!validation.coinbase || is_null()) {
        return true;
    }

    // The (non-coinbase) input refers to a coinbase output, so validate depth.
    return floor_subtract(height, validation.height) >= coinbase_maturity;
}

}} // namespace libbitcoin::chainv2
