/**
 * Copyright (c) 2011-2018 Bitprim developers (see AUTHORS)
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
#ifndef LIBBITCOIN_CHAINV2_POINT_ITERATOR_HPP_
#define LIBBITCOIN_CHAINV2_POINT_ITERATOR_HPP_

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <bitcoin/bitcoin/define.hpp>

namespace libbitcoin { namespace chainv2 {

class point;

/// A point iterator for store serialization (does not support wire).
class BC_API point_iterator {
public:
    using pointer = uint8_t;
    using reference = uint8_t;
    using value_type = uint8_t;
    using difference_type = ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    using iterator = point_iterator;
    using const_iterator = point_iterator;

    // Constructors.
    //-------------------------------------------------------------------------

    point_iterator(point_iterator const& other);

    explicit
    point_iterator(point const& value, unsigned int index = 0);

    // Operators.
    //-------------------------------------------------------------------------

    operator bool() const;
    pointer operator->() const;
    reference operator*() const;
    point_iterator& operator++();
    point_iterator operator++(int);
    point_iterator& operator--();
    point_iterator operator--(int);
    point_iterator operator+(int value) const;
    point_iterator operator-(int value) const;
    bool operator==(point_iterator const& other) const;
    bool operator!=(point_iterator const& other) const;

    /// The iterator may only be assigned to another of the same point.
    point_iterator& operator=(point_iterator const& other);

protected:
    void increment();
    void decrement();
    point_iterator increase(unsigned value) const;
    point_iterator decrease(unsigned value) const;

private:
    uint8_t current() const;

    point const& point_;
    unsigned int current_;
};

}} // namespace libbitcoin::chainv2

#endif // LIBBITCOIN_CHAINV2_POINT_ITERATOR_HPP_
