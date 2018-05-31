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
#ifndef LIBBITCOIN_CHAINV2_POINT_VALUE_HPP_
#define LIBBITCOIN_CHAINV2_POINT_VALUE_HPP_

#include <cstdint>
#include <vector>
#include <bitcoin/bitcoin/chainv2/point.hpp>
#include <bitcoin/bitcoin/define.hpp>

namespace libbitcoin { namespace chainv2 {

/// A valued point, does not implement specialized serialization methods.
class BC_API point_value
    : public point
{
public:
    using list = std::vector<point_value>;

    // Constructors.
    //-------------------------------------------------------------------------

    point_value();
    point_value(point&& instance, uint64_t value);
    point_value(point const& instance, uint64_t value);

    /// This class is move assignable and copy assignable.

    // Operators.
    //-------------------------------------------------------------------------

    // point_value& operator=(point_value other);

    friend
    bool operator==(point_value const& a, point_value const& b);

    friend
    bool operator!=(point_value const& a, point_value const& b);

    // // Swap implementation required to properly handle base class.
    // friend 
    // void swap(point_value& a, point_value& b);

    // Properties (accessors).
    //-------------------------------------------------------------------------

    uint64_t value() const;
    void set_value(uint64_t value);

private:
    uint64_t value_;
};

}} // namespace libbitcoin::chainv2

#endif // LIBBITCOIN_CHAINV2_POINT_VALUE_HPP_
