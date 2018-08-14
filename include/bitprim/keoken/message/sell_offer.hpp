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
#ifndef BITPRIM_KEOKEN_MESSAGE_SELL_OFFER_HPP_
#define BITPRIM_KEOKEN_MESSAGE_SELL_OFFER_HPP_

#include <bitcoin/bitcoin/define.hpp>
#include <bitcoin/bitcoin/utility/data.hpp>
#include <bitcoin/bitcoin/utility/reader.hpp>
#include <bitcoin/bitcoin/utility/writer.hpp>

#include <bitprim/keoken/message/base.hpp>
#include <bitprim/keoken/primitives.hpp>

namespace bitprim {
namespace keoken {
namespace message {

class BC_API sell_offer {
public:
    static const uint16_t version = 0;
    static const uint16_t type = 16;
    typedef uint16_t time_window;
    // Constructors.
    //-------------------------------------------------------------------------

    sell_offer(asset_id_t asset_id, amount_t token_amount, amount_t amount_expected, time_window payment_window, time_window offer_validity, amount_t minimum_fee);

    // Semiregular.
    //-------------------------------------------------------------------------

    sell_offer() = default;

    // Regular.
    //-------------------------------------------------------------------------

    friend
    bool operator==(sell_offer const& a, sell_offer const& b);
    
    friend
    bool operator!=(sell_offer const& a, sell_offer const& b);

    // Deserialization.
    //-------------------------------------------------------------------------

    static sell_offer factory_from_data(bc::data_chunk const& data);
    static sell_offer factory_from_data(std::istream& stream);
    static sell_offer factory_from_data(bc::reader& source);

    bool from_data(bc::data_chunk const& data);
    bool from_data(std::istream& stream);
    bool from_data(bc::reader& source);

    // Serialization.
    //-------------------------------------------------------------------------

    bc::data_chunk to_data() const;
    void to_data(std::ostream& stream) const;
    void to_data(bc::writer& sink) const;

    // Properties (size, accessors, cache).
    //-------------------------------------------------------------------------

    size_t serialized_size() const;

    asset_id_t asset_id() const;
    void set_asset_id(asset_id_t asset_id);

    amount_t token_amount() const;
    void set_token_amount(amount_t token_amount);

    amount_t amount_expected() const;
    void set_amount_expected(amount_t amount_expected);

    time_window payment_window() const;
    void set_payment_window(time_window payment_window);

    time_window offer_validity() const;
    void set_offer_validity(time_window offer_validity);

    amount_t minimum_fee() const;
    void set_minimum_fee(amount_t minimum_fee);


private:
    asset_id_t asset_id_;
    amount_t token_amount_;
    amount_t amount_expected_;
    time_window payment_window_;
    time_window offer_validity_;
    amount_t minimum_fee_;


};

} // namespace message
} // namespace keoken
} // namespace bitprim

#endif //BITPRIM_KEOKEN_MESSAGE_SELL_OFFER_HPP_
