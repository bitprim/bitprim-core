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
#ifndef BITPRIM_KEOKEN_MESSAGE_CREATE_ASSET_HPP_
#define BITPRIM_KEOKEN_MESSAGE_CREATE_ASSET_HPP_

#include <bitcoin/bitcoin/define.hpp>
#include <bitcoin/bitcoin/utility/data.hpp>
#include <bitcoin/bitcoin/utility/reader.hpp>
#include <bitcoin/bitcoin/utility/writer.hpp>

#include <bitprim/keoken/message/base.hpp>


namespace bitprim {
namespace keoken {
namespace message {

static_assert(std::is_same<std::uint8_t, char>::value || std::is_same<std::uint8_t, unsigned char>::value,
    "Bitprim requires std::uint8_t to be implemented as char or unsigned char.");

class BC_API create_asset {
public:
    static const uint16_t version = 0;
    static const uint16_t type = 0;

    // Constructors.
    //-------------------------------------------------------------------------

    // create_asset() = default;
    // create_asset(create_asset const& other) = default;

    // Operators.
    //-------------------------------------------------------------------------

    friend
    bool operator==(create_asset const& a, create_asset const& b);
    
    friend
    bool operator!=(create_asset const& a, create_asset const& b);

    // Deserialization.
    //-------------------------------------------------------------------------

    static create_asset factory_from_data(libbitcoin::data_chunk const& data);
    static create_asset factory_from_data(std::istream& stream);
    static create_asset factory_from_data(libbitcoin::reader& source);

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

    std::string const& name() const;
    void set_name(std::string const& x);
    void set_name(std::string&& x);

    amount_t amount() const;
    void set_amount(amount_t x);

private:
    // char name[17];      // 16 + 1. minus the \0 termination
    std::string name_;
    amount_t amount_;
};

} // namespace message
} // namespace keoken
} // namespace bitprim

#endif //BITPRIM_KEOKEN_MESSAGE_CREATE_ASSET_HPP_
