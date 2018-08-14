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
#include <bitprim/keoken/message/pay_offer.hpp>

#include <bitcoin/bitcoin/utility/container_sink.hpp>
#include <bitcoin/bitcoin/utility/container_source.hpp>
#include <bitcoin/bitcoin/utility/istream_reader.hpp>
#include <bitcoin/bitcoin/utility/ostream_writer.hpp>

namespace bitprim {
namespace keoken {
namespace message {

using bc::data_chunk;
using bc::data_source;
using bc::data_sink;
using bc::istream_reader;
using bc::ostream_writer;
using bc::writer;
    
// Constructors.
//-------------------------------------------------------------------------

pay_offer::pay_offer(asset_id_t asset_id)
    : asset_id_(asset_id)
{}

// Operators.
//-----------------------------------------------------------------------------

// friend
bool operator==(pay_offer const& a, pay_offer const& b) {
    return a.asset_id_ == b.asset_id_;
}

// friend
bool operator!=(pay_offer const& a, pay_offer const& b) {
    return !(a == b);
}

// Deserialization.
//-----------------------------------------------------------------------------

// static
pay_offer pay_offer::factory_from_data(data_chunk const& data) {
    pay_offer instance;       //NOLINT
    instance.from_data(data);
    return instance;
}

// static
pay_offer pay_offer::factory_from_data(std::istream& stream) {
    pay_offer instance;       //NOLINT
    instance.from_data(stream);
    return instance;
}

// static
pay_offer pay_offer::factory_from_data(bc::reader& source) {
    pay_offer instance;       //NOLINT
    instance.from_data(source);
    return instance;
}

bool pay_offer::from_data(data_chunk const& data) {
    data_source istream(data);
    return from_data(istream);
}

bool pay_offer::from_data(std::istream& stream) {
    istream_reader source(stream);
    return from_data(source);
}

//Note: from_data and to_data are not longer simetrical.
bool pay_offer::from_data(bc::reader& source) {
    asset_id_ = source.read_4_bytes_big_endian();

    // if ( ! source)
    //     reset();

    return source;
}

// Serialization.
//-----------------------------------------------------------------------------

data_chunk pay_offer::to_data() const {
    data_chunk data;
    const auto size = serialized_size();
    data.reserve(size);
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == size);
    return data;
}

void pay_offer::to_data(std::ostream& stream) const {
    ostream_writer sink(stream);
    to_data(sink);
}

//Note: from_data and to_data are not longer simetrical.
void pay_offer::to_data(writer& sink) const {
    base::to_data(sink, version, type);
    sink.write_4_bytes_big_endian(asset_id_);
}


// Properties (size, accessors, cache).
//-----------------------------------------------------------------------------

size_t pay_offer::serialized_size() const {
    return base::serialized_size() +
           sizeof(asset_id_);
}

asset_id_t pay_offer::asset_id() const {
    return asset_id_;
}

void pay_offer::set_asset_id(asset_id_t x) {
    asset_id_ = x;
}
} // namespace message
} // namespace keoken
} // namespace bitprim
