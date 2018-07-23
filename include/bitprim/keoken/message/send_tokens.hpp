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
#ifndef BITPRIM_KEOKEN_MESSAGE_SEND_TOKENS_HPP_
#define BITPRIM_KEOKEN_MESSAGE_SEND_TOKENS_HPP_

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

class BC_API send_tokens {
public:
    static const uint16_t version = 0;
    static const uint16_t type = 10;


    // Constructors.
    //-------------------------------------------------------------------------

    // send_tokens() = default;
    // send_tokens(send_tokens const& other) = default;

    // Operators.
    //-------------------------------------------------------------------------

    friend
    bool operator==(send_tokens const& a, send_tokens const& b);
    
    friend
    bool operator!=(send_tokens const& a, send_tokens const& b);

    // Deserialization.
    //-------------------------------------------------------------------------

    static send_tokens factory_from_data(libbitcoin::data_chunk const& data);
    static send_tokens factory_from_data(std::istream& stream);
    static send_tokens factory_from_data(libbitcoin::reader& source);

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

    asset_id_t asset_id() const;
    void set_asset_id(asset_id_t x);

    amount_t amount() const;
    void set_amount(amount_t x);

private:
    asset_id_t asset_;
    amount_t amount_;
};

} // namespace message
} // namespace keoken
} // namespace bitprim

#endif //BITPRIM_KEOKEN_MESSAGE_SEND_TOKENS_HPP_
