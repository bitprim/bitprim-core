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
#ifndef LIBBITCOIN_CHAINV2_INPUT_HPP_
#define LIBBITCOIN_CHAINV2_INPUT_HPP_

#include <cstddef>
#include <cstdint>
#include <istream>
#include <vector>

#include <bitcoin/bitcoin/chain/input.hpp>

#include <bitcoin/bitcoin/chainv2/output_point.hpp>
#include <bitcoin/bitcoin/chainv2/script.hpp>

#include <bitcoin/bitcoin/define.hpp>
#include <bitcoin/bitcoin/math/hash.hpp>
#include <bitcoin/bitcoin/utility/reader.hpp>
#include <bitcoin/bitcoin/utility/thread.hpp>
#include <bitcoin/bitcoin/utility/writer.hpp>
#include <bitcoin/bitcoin/wallet/payment_address.hpp>

namespace libbitcoin { namespace chainv2 {

class BC_API input {
public:
    using list = std::vector<input>;

    // Constructors.
    //-----------------------------------------------------------------------------

    input();

    input(input&& other) noexcept;
    input(input const& other);

    input(output_point&& previous_output, chainv2::script&& script, uint32_t sequence);
    input(output_point const& previous_output, chainv2::script const& script, uint32_t sequence);

    // Operators.
    //-----------------------------------------------------------------------------

    /// This class is move assignable and copy assignable.
    input& operator=(input&& other) noexcept;
    input& operator=(const input& other);

    bool operator==(const input& other) const;
    bool operator!=(const input& other) const;

    // explicit
    // operator chain::input() const;

    // Deserialization.
    //-----------------------------------------------------------------------------

    static input factory_from_data(const data_chunk& data, bool wire=true);
    static input factory_from_data(std::istream& stream, bool wire=true);
    static input factory_from_data(reader& source, bool wire=true);

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

    // Deprecated (unsafe).
    output_point& previous_output();

    const output_point& previous_output() const;
    void set_previous_output(const output_point& value);
    void set_previous_output(output_point&& value);

    // Deprecated (unsafe).
    chainv2::script& script();

    const chainv2::script& script() const;
    void set_script(const chainv2::script& value);
    void set_script(chainv2::script&& value);

    uint32_t sequence() const;
    void set_sequence(uint32_t value);

    /// The payment address extracted from this input as a standard script.
    wallet::payment_address address() const;

    // Validation.
    //-----------------------------------------------------------------------------

    bool is_final() const;
    bool is_locked(size_t block_height, uint32_t median_time_past) const;
    size_t signature_operations(bool bip16_active) const;

protected:
    void reset();
    void invalidate_cache() const;

private:
    mutable upgrade_mutex mutex_;
    mutable wallet::payment_address::ptr address_;

    output_point previous_output_;
    chainv2::script script_;
    uint32_t sequence_;
};

}} // namespace libbitcoin::chainv2

#endif // LIBBITCOIN_CHAINV2_INPUT_HPP_
