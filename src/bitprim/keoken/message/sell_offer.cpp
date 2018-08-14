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
#include <bitprim/keoken/message/sell_offer.hpp>

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

sell_offer::sell_offer(asset_id_t asset_id, amount_t token_amount, amount_t amount_expected, time_window payment_window, time_window offer_validity, amount_t minimum_fee)
    : asset_id_(asset_id)
    , token_amount_(token_amount)
    , amount_expected_(amount_expected)
    , payment_window_(payment_window)
    , offer_validity_(offer_validity)
    , minimum_fee_(minimum_fee)
{}

// Operators.
//-----------------------------------------------------------------------------

// friend
bool operator==(sell_offer const& a, sell_offer const& b) {
    return (a.asset_id_ == b.asset_id_) && (a.token_amount_ == b.token_amount_) 
        && (a.amount_expected_ == b.amount_expected_) && (a.payment_window_ == b.payment_window_)
        && (a.offer_validity_ == b.offer_validity_) && (a.minimum_fee_ == b.minimum_fee_);
}

// friend
bool operator!=(sell_offer const& a, sell_offer const& b) {
    return !(a == b);
}

// Deserialization.
//-----------------------------------------------------------------------------

// static
sell_offer sell_offer::factory_from_data(data_chunk const& data) {
    sell_offer instance;       //NOLINT
    instance.from_data(data);
    return instance;
}

// static
sell_offer sell_offer::factory_from_data(std::istream& stream) {
    sell_offer instance;       //NOLINT
    instance.from_data(stream);
    return instance;
}

// static
sell_offer sell_offer::factory_from_data(bc::reader& source) {
    sell_offer instance;       //NOLINT
    instance.from_data(source);
    return instance;
}

bool sell_offer::from_data(data_chunk const& data) {
    data_source istream(data);
    return from_data(istream);
}

bool sell_offer::from_data(std::istream& stream) {
    istream_reader source(stream);
    return from_data(source);
}

//Note: from_data and to_data are not longer simetrical.
bool sell_offer::from_data(bc::reader& source) {
    asset_id_ = source.read_4_bytes_big_endian();
    token_amount_ = source.read_8_bytes_big_endian();
    amount_expected_ = source.read_8_bytes_big_endian();
    payment_window_ = source.read_2_bytes_big_endian();
    offer_validity_ = source.read_2_bytes_big_endian();
    minimum_fee_ = source.read_8_bytes_big_endian();

    return source;
}

// Serialization.
//-----------------------------------------------------------------------------

data_chunk sell_offer::to_data() const {
    data_chunk data;
    const auto size = serialized_size();
    data.reserve(size);
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == size);
    return data;
}

void sell_offer::to_data(std::ostream& stream) const {
    ostream_writer sink(stream);
    to_data(sink);
}

//Note: from_data and to_data are not longer simetrical.
void sell_offer::to_data(writer& sink) const {
    base::to_data(sink, version, type);
    sink.write_4_bytes_big_endian(asset_id_);
    sink.write_8_bytes_big_endian(token_amount_);
    sink.write_8_bytes_big_endian(amount_expected_);
    sink.write_2_bytes_big_endian(payment_window_);
    sink.write_2_bytes_big_endian(offer_validity_);
    sink.write_8_bytes_big_endian(minimum_fee_);
}


// Properties (size, accessors, cache).
//-----------------------------------------------------------------------------

size_t sell_offer::serialized_size() const {
    return base::serialized_size() +
           sizeof(asset_id_) + 
           sizeof(token_amount_) + 
           sizeof(amount_expected_) + 
           sizeof(payment_window_) + 
           sizeof(offer_validity_) + 
           sizeof(minimum_fee_);

}

asset_id_t sell_offer::asset_id() const {
    return asset_id_;
}

void sell_offer::set_asset_id(asset_id_t asset_id) {
    asset_id_ = asset_id;
}

amount_t sell_offer::token_amount() const {
    return token_amount_;
}

void sell_offer::set_token_amount(amount_t token_amount) {
    token_amount_ = token_amount;
}

amount_t sell_offer::amount_expected() const {
    return amount_expected_;
}

void sell_offer::set_amount_expected(amount_t amount_expected) {
    amount_expected_ = amount_expected;
}

time_window_t sell_offer::payment_window() const {
    return payment_window_;
}

void sell_offer::set_payment_window(time_window_t payment_window) {
    payment_window_ = payment_window;
}

time_window_t sell_offer::offer_validity() const {
    return offer_validity_;
}

void sell_offer::set_offer_validity(time_window_t offer_validity) {
    offer_validity_ = offer_validity;
}

amount_t sell_offer::minimum_fee() const {
    return minimum_fee_;
}

void sell_offer::set_minimum_fee(amount_t minimum_fee) {
    minimum_fee_ = minimum_fee;
}

} // namespace message
} // namespace keoken
} // namespace bitprim
