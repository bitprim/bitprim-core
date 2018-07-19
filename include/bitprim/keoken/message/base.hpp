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

#include <bitcoin/bitcoin/define.hpp>
#include <bitcoin/bitcoin/utility/data.hpp>
#include <bitcoin/bitcoin/utility/reader.hpp>
#include <bitcoin/bitcoin/utility/writer.hpp>

namespace bitprim {
namespace keoken {
namespace message {

using asset_id_t = uint32_t;
using amount_t = int64_t;      //TODO(fernando): signed or unsigned ??

class BC_API base {
public:

    // Constructors.
    //-------------------------------------------------------------------------

    // base() = default;
    // base(base const& other) = default;

    // Operators.
    //-------------------------------------------------------------------------

    friend
    bool operator==(base const& a, base const& b);
    
    friend
    bool operator!=(base const& a, base const& b);

    // Deserialization.
    //-------------------------------------------------------------------------

    static base factory_from_data(libbitcoin::data_chunk const& data);
    static base factory_from_data(std::istream& stream);
    static base factory_from_data(libbitcoin::reader& source);

    bool from_data(libbitcoin::data_chunk const& data);
    bool from_data(std::istream& stream);
    bool from_data(libbitcoin::reader& source);

    // bool is_valid() const;

    // Serialization.
    //-------------------------------------------------------------------------

    libbitcoin::data_chunk to_data() const;
    void to_data(std::ostream& stream) const;
    void to_data(libbitcoin::writer& sink) const;

    // Properties (size, accessors, cache).
    //-------------------------------------------------------------------------

    size_t serialized_size() const;

    uint16_t version() const;
    void set_version(uint16_t x);

    uint16_t type() const;
    void set_type(uint16_t x);

private:
    uint16_t version_;
    uint16_t type_;
};

} // namespace message
} // namespace keoken
} // namespace bitprim

#endif //BITPRIM_KEOKEN_MESSAGE_BASE_HPP_
