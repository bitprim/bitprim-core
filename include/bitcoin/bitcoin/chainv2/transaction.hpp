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
#ifndef LIBBITCOIN_CHAINV2_TRANSACTION_HPP
#define LIBBITCOIN_CHAINV2_TRANSACTION_HPP

#include <cstddef>
#include <cstdint>
#include <istream>
#include <memory>
#include <string>
#include <vector>
#include <boost/optional.hpp>

#include <bitcoin/bitcoin/chain/chain_state.hpp>
#include <bitcoin/bitcoin/chain/transaction.hpp>

#include <bitcoin/bitcoin/chainv2/input.hpp>
#include <bitcoin/bitcoin/chainv2/output.hpp>
#include <bitcoin/bitcoin/chainv2/point.hpp>

#include <bitcoin/bitcoin/define.hpp>
#include <bitcoin/bitcoin/error.hpp>
#include <bitcoin/bitcoin/math/elliptic_curve.hpp>
#include <bitcoin/bitcoin/math/hash.hpp>
#include <bitcoin/bitcoin/machine/opcode.hpp>
#include <bitcoin/bitcoin/machine/rule_fork.hpp>
#include <bitcoin/bitcoin/utility/reader.hpp>
#include <bitcoin/bitcoin/utility/thread.hpp>
#include <bitcoin/bitcoin/utility/writer.hpp>

namespace libbitcoin { namespace chainv2 {

struct BC_API outputs_info {
    size_t count;
    uint64_t total_output_value;
    size_t signature_operations;
    bool any_is_dusty;

    // bool output::is_dust(uint64_t minimum_output_value) const {
    //     // If provably unspendable it does not expand the unspent output set.
    //     return value_ < minimum_output_value && !script_.is_unspendable();
    // }
    
    friend
    bool operator==(outputs_info const& a, outputs_info const& b) {
        return (a.count == b.count)
            && (a.total_output_value == b.total_output_value)
            && (a.signature_operations == b.signature_operations)
            && (a.any_is_dusty == b.any_is_dusty);
    }    
};

class BC_API transaction {
public:
    using ptr = std::shared_ptr<transaction>;
    using const_ptr = std::shared_ptr<const transaction>;
    using ins = input::list;
    // using outs = output::list;
    using list = std::vector<transaction>;

    // // THIS IS FOR LIBRARY USE ONLY, DO NOT CREATE A DEPENDENCY ON IT.
    // struct validation {
    //     uint64_t originator = 0;
    //     code error = error::not_found;
    //     chain::chain_state::ptr state = nullptr;

    //     // The transaction is an unspent duplicate.
    //     bool duplicate = false;

    //     // The unconfirmed tx exists in the store.
    //     bool pooled = false;

    //     // The unconfirmed tx is validated at the block's current fork state.
    //     bool current = false;

    //     // Similate organization and instead just validate the transaction.
    //     bool simulate = false;
    // };

    // Constructors.
    //-----------------------------------------------------------------------------

    transaction();

    // transaction(uint32_t version, uint32_t locktime, ins&& inputs);         //, outs&& outputs);
    // transaction(uint32_t version, uint32_t locktime, ins const& inputs);    //, outs const& outputs);

    /// This class is move assignable and copy assignable [TODO: remove copy].

    // Operators.
    //-----------------------------------------------------------------------------

    friend
    bool operator==(transaction const& a, transaction const& b);
    
    friend
    bool operator!=(transaction const& a, transaction const& b);


    // Deserialization.
    //-----------------------------------------------------------------------------

    static 
    transaction factory_from_data(data_chunk const& data, uint64_t minimum_output_satoshis);

    static 
    transaction factory_from_data(data_chunk&& data, uint64_t minimum_output_satoshis);
    
    bool from_data(data_chunk&& data, uint64_t minimum_output_satoshis);

    bool from_data(data_chunk const& data, uint64_t minimum_output_satoshis);


    bool is_valid() const;

    data_chunk const& data() const;


    // Serialization.
    //-----------------------------------------------------------------------------



    // data_chunk to_data(bool wire=true) const;
    // void to_data(std::ostream& stream, bool wire=true) const;
    // void to_data(writer& sink, bool wire=true) const;

    // Properties (size, accessors, cache).
    //-----------------------------------------------------------------------------

    // size_t serialized_size(bool wire=true) const;
    size_t serialized_size_wired() const;

    uint32_t version() const;
    void set_version(uint32_t value);

    uint32_t locktime() const;
    void set_locktime(uint32_t value);


    chainv2::outputs_info const& outputs_info() const;

    // Deprecated (unsafe).
    ins& inputs();
    ins const& inputs() const;
    void set_inputs(ins const& value);
    void set_inputs(ins&& value);



    // // Deprecated (unsafe).
    // outs& outputs();
    // outs const& outputs() const;
    // void set_outputs(outs const& value);
    // void set_outputs(outs&& value);

    hash_digest hash() const;
    // hash_digest hash(uint32_t sighash_type) const;

    // void recompute_hash();

    // Validation.
    //-----------------------------------------------------------------------------

    uint64_t fees() const;
    point::list previous_outputs() const;
    point::list missing_previous_outputs() const;
    hash_list missing_previous_transactions() const;
    uint64_t total_input_value() const;
    uint64_t total_output_value() const;
    // size_t signature_operations() const;
    size_t signature_operations(bool bip16_active) const;

    bool is_coinbase() const;
    bool is_null_non_coinbase() const;
    bool is_oversized_coinbase() const;
    bool is_mature(size_t height) const;
    bool is_overspent() const;
    bool is_internal_double_spend() const;
    bool is_double_spend(bool include_unconfirmed) const;
    bool is_dusty(uint64_t minimum_output_value) const;
    bool is_missing_previous_outputs() const;
    bool is_final(size_t block_height, uint32_t block_time) const;
    bool is_locked(size_t block_height, uint32_t median_time_past) const;
    bool is_locktime_conflict() const;

    code check(bool transaction_pool=true) const;
    // code accept(bool transaction_pool=true) const;
    // code accept(chain::chain_state const& state) const;
    code accept(chain::chain_state const& state, bool tx_duplicate, bool transaction_pool = true) const;

    // code connect() const;
    // code connect(chain::chain_state const& state) const;
    // code connect_input(chain::chain_state const& state, size_t input_index) const;

    // // THIS IS FOR LIBRARY USE ONLY, DO NOT CREATE A DEPENDENCY ON IT.
    // mutable validation validation;

    // bool is_standard() const;

protected:
    void reset();
    // void invalidate_cache() const;
    bool all_inputs_final() const;

private:
    static 
    transaction factory_from_data(std::istream& stream, uint64_t minimum_output_satoshis);
    
    static 
    transaction factory_from_data(reader& source, uint64_t minimum_output_satoshis);
    
    // bool read_output_info(reader& source /*, bool wire*/);
    bool read_outputs_info(reader& source, uint64_t minimum_output_satoshis);
    bool from_data(std::istream& stream, uint64_t minimum_output_satoshis);
    bool from_data(reader& source, uint64_t minimum_output_satoshis);


    uint32_t version_;
    uint32_t locktime_;
    input::list inputs_;
    // output::list outputs_;
    chainv2::outputs_info outputs_info_;
    size_t serialized_size_wired_;
    data_chunk data_;

    // // These share a mutex as they are not expected to conflict.
    // mutable boost::optional<uint64_t> total_input_value_;
    // mutable boost::optional<uint64_t> total_output_value_;
    // mutable std::shared_ptr<hash_digest> hash_;
    // mutable upgrade_mutex mutex_;
};

}} // namespace libbitcoin::chainv2

#endif
