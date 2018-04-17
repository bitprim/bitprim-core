/**
 * Copyright (c) 2011-2018 Bitprim developers (see AUTHORS)
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
#ifndef LIBBITCOIN_CHAINV2_OUTPUT_HPP_
#define LIBBITCOIN_CHAINV2_OUTPUT_HPP_

#include <cstddef>
#include <cstdint>
#include <istream>
#include <string>
#include <vector>

#include <bitcoin/bitcoin/chain/output.hpp>
#include <bitcoin/bitcoin/chainv2/script.hpp>

#include <bitcoin/bitcoin/define.hpp>
#include <bitcoin/bitcoin/utility/reader.hpp>
#include <bitcoin/bitcoin/utility/thread.hpp>
#include <bitcoin/bitcoin/utility/writer.hpp>
#include <bitcoin/bitcoin/wallet/payment_address.hpp>

namespace libbitcoin { namespace chainv2 {

class BC_API output {
public:
    using list = std::vector<output>;

    /// This is a sentinel used in .value to indicate not found in store.
    /// This is a sentinel used in cache.value to indicate not populated.
    /// This is a consensus value used in script::generate_signature_hash.
    static const uint64_t not_found;

    // THIS IS FOR LIBRARY USE ONLY, DO NOT CREATE A DEPENDENCY ON IT.
    struct validation {
        /// This is a non-consensus sentinel used to indicate an output is unspent.
        static const uint32_t not_spent;
        size_t spender_height = validation::not_spent;
    };

    // Constructors.
    //-----------------------------------------------------------------------------

    output();

    output(output&& other) noexcept;
    output(const output& other);

    output(uint64_t value, chainv2::script&& script);
    output(uint64_t value, chainv2::script const& script);

    // Operators.
    //-----------------------------------------------------------------------------

    /// This class is move assignable and copy assignable.
    output& operator=(output&& other) noexcept;
    output& operator=(const output& other);

    bool operator==(const output& other) const;
    bool operator!=(const output& other) const;

    // explicit
    // operator chain::output() const;


    // Deserialization.
    //-----------------------------------------------------------------------------

    static output factory_from_data(const data_chunk& data, bool wire=true);
    static output factory_from_data(std::istream& stream, bool wire=true);
    static output factory_from_data(reader& source, bool wire=true);

    bool from_data(const data_chunk& data, bool wire=true);
    bool from_data(std::istream& stream, bool wire=true);
    bool from_data(reader& source, bool wire=true);

    bool is_valid() const;

    // Serialization.
    //-----------------------------------------------------------------------------

    data_chunk to_data(bool wire=true) const;
    void to_data(std::ostream& stream, bool wire=true) const;
    void to_data(writer& sink, bool wire=true) const;

    // Properties (size, accessors, cache).
    //-----------------------------------------------------------------------------

    size_t serialized_size(bool wire=true) const;

    uint64_t value() const;
    void set_value(uint64_t value);

    // Deprecated (unsafe).
    chainv2::script& script();

    chainv2::script const& script() const;
    void set_script(chainv2::script const& value);
    void set_script(chainv2::script&& value);

    /// The payment address extracted from this output as a standard script.
    wallet::payment_address address(bool testnet = false) const;
    bool is_dust(uint64_t minimum_output_value) const;

    // Validation.
    //-----------------------------------------------------------------------------

    size_t signature_operations() const;

    // THIS IS FOR LIBRARY USE ONLY, DO NOT CREATE A DEPENDENCY ON IT.
    mutable validation validation;

protected:
    void reset();
    void invalidate_cache() const;

private:
    mutable upgrade_mutex mutex_;
    mutable wallet::payment_address::ptr address_;

    uint64_t value_;
    chainv2::script script_;
};

}} // namespace libbitcoin::chainv2

#endif // LIBBITCOIN_CHAINV2_OUTPUT_HPP_
