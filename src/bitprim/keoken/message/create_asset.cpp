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
#include <bitprim/keoken/message/create_asset.hpp>

#include <bitcoin/bitcoin/utility/container_sink.hpp>
#include <bitcoin/bitcoin/utility/container_source.hpp>
#include <bitcoin/bitcoin/utility/istream_reader.hpp>
#include <bitcoin/bitcoin/utility/ostream_writer.hpp>

#include <bitprim/keoken/constants.hpp>
#include <bitprim/keoken/message/base.hpp>
#include <bitprim/keoken/utility.hpp>

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

create_asset::create_asset(std::string name, amount_t amount)
    : name_(std::move(name))
    , amount_(amount)
{}


// Operators.
//-----------------------------------------------------------------------------

// friend
bool operator==(create_asset const& a, create_asset const& b) {
    return a.name_ == b.name_ && a.amount_ == b.amount_;
}

// friend
bool operator!=(create_asset const& a, create_asset const& b) {
    return !(a == b);
}

// Deserialization.
//-----------------------------------------------------------------------------

// static
create_asset create_asset::factory_from_data(data_chunk const& data) {
    create_asset instance;
    instance.from_data(data);
    return instance;
}

// static
create_asset create_asset::factory_from_data(std::istream& stream) {
    create_asset instance;
    instance.from_data(stream);
    return instance;
}

// static
create_asset create_asset::factory_from_data(bc::reader& source) {
    create_asset instance;
    instance.from_data(source);
    return instance;
}

bool create_asset::from_data(data_chunk const& data) {
    data_source istream(data);
    return from_data(istream);
}

bool create_asset::from_data(std::istream& stream) {
    istream_reader source(stream);
    return from_data(source);
}

//Note: from_data and to_data are not longer simetrical.
bool create_asset::from_data(bc::reader& source) {
    auto name_opt = read_null_terminated_string(source, max_name_size);
    if ( ! name_opt) return false;
    if (name_opt->size() < min_asset_name_size) return false;     //NOLINT
    name_ = *name_opt;
    amount_ = source.read_8_bytes_big_endian();

    // if ( ! source)
    //     reset();

    return source;
}

// Serialization.
//-----------------------------------------------------------------------------

data_chunk create_asset::to_data() const {
    data_chunk data;
    const auto size = serialized_size();
    data.reserve(size);
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == size);
    return data;
}

void create_asset::to_data(std::ostream& stream) const {
    ostream_writer sink(stream);
    to_data(sink);
}

//Note: from_data and to_data are not simetrical.
void create_asset::to_data(writer& sink) const {
    base::to_data(sink, version, type);
    sink.write_bytes(reinterpret_cast<uint8_t const*>(name_.data()), name_.size() + 1);     //NOLINT
    sink.write_8_bytes_big_endian(amount_);
}


// Properties (size, accessors, cache).
//-----------------------------------------------------------------------------

size_t create_asset::serialized_size() const {
    return base::serialized_size() +
           sizeof(amount_) + 
           name_.size() + 
           1;   //null terminated string
}

std::string const& create_asset::name() const {
    return name_;
}

void create_asset::set_name(std::string const& x) {
    name_ = x;
}

void create_asset::set_name(std::string&& x) {
    name_ = std::move(x);
}

amount_t create_asset::amount() const {
    return amount_;
}

void create_asset::set_amount(amount_t x) {
    amount_ = x;
}

} // namespace message
} // namespace keoken
} // namespace bitprim
