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
#ifndef BITPRIM_KEOKEN_DOMAIN_ASSET_HPP_
#define BITPRIM_KEOKEN_DOMAIN_ASSET_HPP_

#include <string>

#include <bitcoin/bitcoin/define.hpp>
#include <bitcoin/bitcoin/wallet/payment_address.hpp>

#include <bitprim/keoken/domain/primitives.hpp>


namespace bitprim {
namespace keoken {
namespace domain {

class BC_API asset {
public:

    asset(asset_id_t id, std::string name, amount_t amount, libbitcoin::wallet::payment_address owner);

    // Semiregular.
    //-------------------------------------------------------------------------

    asset() = default;
    asset(asset const& other) = default;
    asset(asset&& other) = default;
    asset& operator=(asset const& other) = default;
    asset& operator=(asset&& other) = default;

    // Regular.
    //-------------------------------------------------------------------------

    friend
    bool operator==(asset const& a, asset const& b);
    
    friend
    bool operator!=(asset const& a, asset const& b);


    // Properties (size, accessors, cache).
    //-------------------------------------------------------------------------

    asset_id_t id() const;
    // void set_id(asset_id_t x);

    std::string const& name() const;
    // void set_name(std::string x);

    amount_t amount() const;
    // void set_amount(amount_t x);

    libbitcoin::wallet::payment_address const& owner() const;
    // void set_owner(libbitcoin::wallet::payment_address x);


private:
    asset_id_t id_;
    std::string name_;
    amount_t amount_;
    libbitcoin::wallet::payment_address owner_;
};

} // namespace domain
} // namespace keoken
} // namespace bitprim

#endif //BITPRIM_KEOKEN_DOMAIN_ASSET_HPP_
