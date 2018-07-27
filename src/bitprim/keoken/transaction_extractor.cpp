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

#include <bitprim/keoken/transaction_extractor.hpp>

#include <type_traits>

#include <bitcoin/bitcoin/machine/opcode.hpp>
#include <bitcoin/bitcoin/machine/operation.hpp>

#include <bitprim/keoken/constants.hpp>

namespace bitprim {
namespace keoken {

using bc::data_chunk;
using bc::machine::opcode;
using bc::machine::operation;
 
template <typename I>
data_chunk get_keoken_data(I f, I l) {
    //precondition:  
    //postcondition: if return value is an empty vector, it is not a keoken output.

    if (f == l) return data_chunk{};

    if (f->code() != opcode::return_) return data_chunk{};

    ++f;        //move to the next machine::operation
    if (f == l) return data_chunk{};

    if (f->code() != opcode::push_size_4) return data_chunk{};

    if ( ! std::equal(f->data().begin(), f->data().end(), static_cast<uint8_t const*>(protocol_name))) {
        return data_chunk{};
    }

    ++f;        //move to the next machine::operation
    if (f == l) return data_chunk{};

    if (to_underlying(f->code()) < kp_min_size) return data_chunk{};
    if (to_underlying(f->code()) > kp_max_size) return data_chunk{};

    return f->data();   //TODO(fernando): check if we can std::move this vector
}

data_chunk get_keoken_data(operation::list const& ops) {
    return get_keoken_data(ops.begin(), ops.end());
}

data_chunk first_keoken_output(bc::chain::transaction const& tx) {
    //precondition: tx is a confirmed transaction
    for (auto const& out : tx.outputs()) {
        auto const& keo_data = get_keoken_data(out.script().operations());
        if ( ! keo_data.empty()) {
            return keo_data;
        }
    }
    return data_chunk{};
}

} // namespace keoken
} // namespace bitprim
