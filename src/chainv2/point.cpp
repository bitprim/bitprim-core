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
#include <bitcoin/bitcoin/chainv2/point.hpp>

#include <cstdint>
#include <sstream>
#include <utility>
#include <bitcoin/bitcoin/constants.hpp>
#include <bitcoin/bitcoin/formats/base_16.hpp>
#include <bitcoin/bitcoin/message/messages.hpp>
#include <bitcoin/bitcoin/utility/assert.hpp>
#include <bitcoin/bitcoin/utility/container_sink.hpp>
#include <bitcoin/bitcoin/utility/container_source.hpp>
#include <bitcoin/bitcoin/utility/istream_reader.hpp>
#include <bitcoin/bitcoin/utility/ostream_writer.hpp>
#include <bitcoin/bitcoin/utility/serializer.hpp>

namespace libbitcoin { namespace chainv2 {

static_assert(std::is_move_constructible<point>::value, "std::is_move_constructible<point>::value");
static_assert(std::is_nothrow_move_constructible<point>::value, "std::is_nothrow_move_constructible<point>::value");
static_assert(std::is_move_assignable<point>::value, "std::is_move_assignable<point>::value");
static_assert(std::is_nothrow_move_assignable<point>::value, "std::is_nothrow_move_assignable<point>::value");
static_assert(std::is_copy_constructible<point>::value, "std::is_copy_constructible<point>::value");
static_assert(std::is_copy_assignable<point>::value, "std::is_copy_assignable<point>::value");


static constexpr auto store_point_size = std::tuple_size<point>::value;

// This sentinel is serialized and defined by consensus, not implementation.
uint32_t const point::null_index = no_previous_output;

// Constructors.
//-----------------------------------------------------------------------------

// A default instance is invalid (until modified).
point::point()
    : hash_(null_hash), index_(0)
{}

point::point(hash_digest const& hash, uint32_t index)
    : hash_(hash), index_(index)
{}

// Operators.
//-----------------------------------------------------------------------------

// point& point::operator=(point&& other) noexcept {
//     hash_ = other.hash_;
//     index_ = other.index_;
//     return *this;
// }

// point& point::operator=(point const& other) {
//     hash_ = other.hash_;
//     index_ = other.index_;
//     return *this;
// }

// friend
bool operator==(point const& a, point const& b) {
    return (a.hash_ == b.hash_) && (a.index_ == b.index_);
}

// friend
bool operator!=(point const& a, point const& b) {
    return !(a == b);
}

// This arbitrary order is produced to support set uniqueness determinations.
// friend
bool operator<(point const& a, point const& b) {
    // The index is primary only because its comparisons are simpler.
    return a.index_ == b.index_ ? a.hash_ < b.hash_ : a.index_ < b.index_;
}

// friend
bool operator>(point const& a, point const& b) {
    return (b < a);
}

// friend
bool operator<=(point const& a, point const& b) {
    return !(b < a);
}

// friend
bool operator>=(point const& a, point const& b) {
    return !(a < b);
}


// Deserialization.
//-----------------------------------------------------------------------------

// static
point point::factory_from_data(const data_chunk& data, bool wire) {
    point instance;
    instance.from_data(data, wire);
    return instance;
}

// static
point point::factory_from_data(std::istream& stream, bool wire) {
    point instance;
    instance.from_data(stream, wire);
    return instance;
}

// static
point point::factory_from_data(reader& source, bool wire) {
    point instance;
    instance.from_data(source, wire);
    return instance;
}

bool point::from_data(const data_chunk& data, bool wire) {
    data_source istream(data);
    return from_data(istream, wire);
}

bool point::from_data(std::istream& stream, bool wire) {
    istream_reader source(stream);
    return from_data(source, wire);
}

bool point::from_data(reader& source, bool wire) {
    reset();

    // valid_ = true;
    hash_ = source.read_hash();

    if (wire) {
        index_ = source.read_4_bytes_little_endian();
    } else {
        index_ = source.read_2_bytes_little_endian();

        if (index_ == max_uint16) {
            index_ = null_index;
        }
    }

    if (!source) {
        reset();
    }

    return source;
}

// private
void point::reset() {
    // valid_ = false;
    hash_ = null_hash;
    index_ = 0;
}

bool point::is_valid() const {
    // return valid_ || (hash_ != null_hash) || (index_ != 0);
    return (hash_ != null_hash) || (index_ != 0);
}

// Serialization.
//-----------------------------------------------------------------------------

data_chunk point::to_data(bool wire) const {
    data_chunk data;
    const auto size = serialized_size(wire);
    data.reserve(size);
    data_sink ostream(data);
    to_data(ostream, wire);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == size);
    return data;
}

void point::to_data(std::ostream& stream, bool wire) const {
    ostream_writer sink(stream);
    to_data(sink, wire);
}

void point::to_data(writer& sink, bool wire) const {
    sink.write_hash(hash_);

    if (wire) {
        sink.write_4_bytes_little_endian(index_);
    } else {
        BITCOIN_ASSERT(index_ == null_index || index_ < max_uint16);
        sink.write_2_bytes_little_endian(static_cast<uint16_t>(index_));
    }
}

// Iterator.
//-----------------------------------------------------------------------------

point_iterator point::begin() const {
    return point_iterator(*this);
}

point_iterator point::end() const {
    return point_iterator(*this, static_cast<unsigned>(store_point_size));
}

// Properties.
//-----------------------------------------------------------------------------

size_t point::serialized_size(bool wire) const {
    return wire ? point::satoshi_fixed_size() : store_point_size;
}

size_t point::satoshi_fixed_size() {
    return hash_size + sizeof(index_);
}

hash_digest& point::hash() {
    return hash_;
}

hash_digest const& point::hash() const {
    return hash_;
}

void point::set_hash(hash_digest const& value) {
    // This is no longer a default instance, so valid.
    hash_ = value;
}

uint32_t point::index() const {
    return index_;
}

void point::set_index(uint32_t value) {
    // This is no longer a default instance, so valid.
    index_ = value;
}

// Utilities.
//-----------------------------------------------------------------------------

// Changed in v3.0 and again in v3.1 (3.0 was unmasked, lots of collisions).
// This is used with output_point identification within a set of history rows
// of the same address. Collision will result in miscorrelation of points by
// client callers. This is stored in database. This is NOT a bitcoin checksum.
uint64_t point::checksum() const {
    // Reserve 49 bits for the tx hash and 15 bits (32768) for the input index.
    static constexpr uint64_t mask = 0xffffffffffff8000;

    // Use an offset to the middle of the hash to avoid coincidental mining
    // of values into the front or back of tx hash (not a security feature).
    // Use most possible bits of tx hash to make intentional collision hard.
    const auto tx = from_little_endian_unsafe<uint64_t>(hash_.begin() + 12);
    const auto index = static_cast<uint64_t>(index_);

    const auto tx_upper_49_bits = tx & mask;
    const auto index_lower_15_bits = index & ~mask;
    return tx_upper_49_bits | index_lower_15_bits;
}

// Validation.
//-----------------------------------------------------------------------------

bool point::is_null() const {
    return (index_ == null_index) && (hash_ == null_hash);
}

}} // namespace libbitcoin::chainv2
