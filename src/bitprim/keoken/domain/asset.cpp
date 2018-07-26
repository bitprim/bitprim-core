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
#include <bitprim/keoken/domain/asset.hpp>

#include <bitprim/keoken/domain/primitives.hpp>

namespace bitprim {
namespace keoken {
namespace domain {

using namespace bc;
using bc::wallet::payment_address;


// Constructors.
//-----------------------------------------------------------------------------

asset::asset(asset_id_t id, std::string name, amount_t amount, libbitcoin::wallet::payment_address owner) 
    : id_(id)
    , name_(std::move(name))
    , amount_(amount)
    , owner_(std::move(owner))
{}

// Operators.
//-----------------------------------------------------------------------------

// friend
bool operator==(asset const& a, asset const& b) {
    return a.id_ == b.id_;
}

// friend
bool operator!=(asset const& a, asset const& b) {
    return !(a == b);
}


// Properties (size, accessors, cache).
//-----------------------------------------------------------------------------

asset_id_t asset::id() const {
    return id_;
}

// void asset::set_id(asset_id_t x) {
//     id_ = x;
// }

std::string const& asset::name() const {
    return name_;
}

// void asset::set_name(std::string x) {
//     name_ = std::move(x);
// }

amount_t asset::amount() const {
    return amount_;
}

// void asset::set_amount(amount_t x) {
//     amount_ = x;
// }

payment_address const& asset::owner() const {
    return owner_;
}

// void asset::set_owner(libbitcoin::wallet::payment_address x) {
//     owner_ = std::move(x);
// }


} // namespace domain
} // namespace keoken
} // namespace bitprim
