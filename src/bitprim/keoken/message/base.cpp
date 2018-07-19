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
#include <bitprim/keoken/message/base.hpp>

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
bool operator==(base const& a, base const& b) {
    return a.version_ == b.version_ && a.type_ == b.type_;
}

// friend
bool operator!=(base const& a, base const& b) {
    return !(a == b);
}

// Deserialization.
//-----------------------------------------------------------------------------

// static
base base::factory_from_data(data_chunk const& data) {
    base instance;
    instance.from_data(data);
    return instance;
}

// static
base base::factory_from_data(std::istream& stream) {
    base instance;
    instance.from_data(stream);
    return instance;
}

// static
base base::factory_from_data(reader& source) {
    base instance;
    instance.from_data(source);
    return instance;
}

bool base::from_data(data_chunk const& data) {
    data_source istream(data);
    return from_data(istream);
}

bool base::from_data(std::istream& stream) {
    istream_reader source(stream);
    return from_data(source);
}

bool base::from_data(reader& source) {

    version_ = source.read_2_bytes_big_endian();
    type_ = source.read_2_bytes_big_endian();

    // if ( ! source)
    //     reset();

    return source;
}

// Serialization.
//-----------------------------------------------------------------------------

data_chunk base::to_data() const {
    data_chunk data;
    const auto size = serialized_size();
    data.reserve(size);
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == size);
    return data;
}

void base::to_data(std::ostream& stream) const {
    ostream_writer sink(stream);
    to_data(sink);
}

void base::to_data(writer& sink) const {
    sink.write_2_bytes_big_endian(version_);
    sink.write_2_bytes_big_endian(type_);
}


// Properties (size, accessors, cache).
//-----------------------------------------------------------------------------

size_t base::serialized_size() const {
    return sizeof(version_) + sizeof(type_);
}

uint16_t base::version() const {
    return version_;
}

void base::set_version(uint16_t x) {
    version_ = x;
}

uint16_t base::type() const {
    return type_;
}

void base::set_type(uint16_t x) {
    type_ = x;
}

} // namespace message
} // namespace keoken
} // namespace bitprim
