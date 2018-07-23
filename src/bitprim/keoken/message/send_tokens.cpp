/**
 * Copyright (c) 2017-2018 Bitprim Inc.
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
#include <bitprim/keoken/message/send_tokens.hpp>

#include <bitcoin/bitcoin/utility/container_sink.hpp>
#include <bitcoin/bitcoin/utility/container_source.hpp>
#include <bitcoin/bitcoin/utility/istream_reader.hpp>
#include <bitcoin/bitcoin/utility/ostream_writer.hpp>

namespace bitprim {
namespace keoken {
namespace message {

using namespace bc;

// Operators.
//-----------------------------------------------------------------------------

// friend
bool operator==(send_tokens const& a, send_tokens const& b) {
    return a.asset_ == b.asset_ && a.amount_ == b.amount_;
}

// friend
bool operator!=(send_tokens const& a, send_tokens const& b) {
    return !(a == b);
}

// Deserialization.
//-----------------------------------------------------------------------------

// static
send_tokens send_tokens::factory_from_data(data_chunk const& data) {
    send_tokens instance;
    instance.from_data(data);
    return instance;
}

// static
send_tokens send_tokens::factory_from_data(std::istream& stream) {
    send_tokens instance;
    instance.from_data(stream);
    return instance;
}

// static
send_tokens send_tokens::factory_from_data(reader& source) {
    send_tokens instance;
    instance.from_data(source);
    return instance;
}

bool send_tokens::from_data(data_chunk const& data) {
    data_source istream(data);
    return from_data(istream);
}

bool send_tokens::from_data(std::istream& stream) {
    istream_reader source(stream);
    return from_data(source);
}

//Note: from_data and to_data are not longer simetrical.
bool send_tokens::from_data(reader& source) {
    asset_ = source.read_4_bytes_big_endian();
    amount_ = source.read_8_bytes_big_endian();

    // if ( ! source)
    //     reset();

    return source;
}

// Serialization.
//-----------------------------------------------------------------------------

data_chunk send_tokens::to_data() const {
    data_chunk data;
    const auto size = serialized_size();
    data.reserve(size);
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == size);
    return data;
}

void send_tokens::to_data(std::ostream& stream) const {
    ostream_writer sink(stream);
    to_data(sink);
}

//Note: from_data and to_data are not longer simetrical.
void send_tokens::to_data(writer& sink) const {
    base::to_data(sink, version, type);
    sink.write_4_bytes_big_endian(asset_);
    sink.write_8_bytes_big_endian(amount_);
}


// Properties (size, accessors, cache).
//-----------------------------------------------------------------------------

size_t send_tokens::serialized_size() const {
    return base::serialized_size() +
           sizeof(asset_) + 
           sizeof(amount_);
}

asset_id_t send_tokens::asset_id() const {
    return asset_;
}

void send_tokens::set_asset_id(asset_id_t x) {
    asset_ = x;
}

amount_t send_tokens::amount() const {
    return amount_;
}

void send_tokens::set_amount(amount_t x) {
    amount_ = x;
}

} // namespace message
} // namespace keoken
} // namespace bitprim
